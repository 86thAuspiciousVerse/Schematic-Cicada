#include "wx_canvas_panel.h"

#include "kicad_bridge/kicad_scene_adapter.h"
#include "kicad_bridge/sch_interaction_engine.h"
#include "kicad_bridge/sch_geometry_batch.h"
#include "kicad_bridge/snap_utils.h"
#include "sch_line.h"
#include "sch_symbol.h"
#include "sch_item.h"

#include <wx/dcbuffer.h>
#include <algorithm>
#include <cmath>

namespace cicada::editor {

namespace
{
constexpr double kPickTolPx = 6.0;   // 点选容差 px（KiCad HITTEST_THRESHOLD_PIXELS=5 上浮）
constexpr double kSnapPx = 12.0;     // 端点磁吸半径 px（切片空间 55mil 的像素化等价，knob）
constexpr double kDragClickPx = 3.0; // 空白按下→框选的防抖阈值 px
constexpr int    kAccIuCap = 10000;  // acc 上限（zoom 极小时防 RTree 全屏退化）

inline int ssign(int v) { return v < 0 ? -1 : 1; }

// B3c 折角公式 = KiCad computeBreakPoint 的壳层等价（sch_line_wire_bus_tool.cpp:521-654，
// 排除区只读参考；mode 固定 45°）。posture=继承上一冻结段方向（续线=被点旧线方向）。
// 返回 {mid, C}：链 A→mid→C（mid==C 时退化为直线 A→C）。
std::pair<VECTOR2I, VECTOR2I> computeWireBreak(const VECTOR2I& A, const VECTOR2I& C,
                                               const VECTOR2I& prevDir, bool posture) {
  const VECTOR2I d = C - A;
  const int xDir = d.x > 0 ? 1 : -1;
  const int yDir = d.y > 0 ? 1 : -1;

  bool preferV = posture && prevDir.y != 0;
  bool preferH = posture && prevDir.x != 0;

  VECTOR2I mid;
  auto breakVertical = [&]() {
    if (!posture) { mid = VECTOR2I(A.x, C.y - yDir * std::abs(d.x)); }
    else { mid = VECTOR2I(C.x, A.y + yDir * std::abs(d.x)); }
  };
  auto breakHorizontal = [&]() {
    if (!posture) { mid = VECTOR2I(C.x - xDir * std::abs(d.y), A.y); }
    else { mid = VECTOR2I(A.x + xDir * std::abs(d.y), C.y); }
  };

  if (preferV) breakVertical();
  else if (preferH) breakHorizontal();

  // 45° 形状校验：非 posture 看符号错位；posture 看段超长 → 双偏好失效
  const VECTOR2I dm = mid - A;
  if (!posture && (ssign(dm.x) != ssign(d.x) || ssign(dm.y) != ssign(d.y))) {
    preferV = preferH = false;
  } else if (posture && (std::abs(dm.x) > std::abs(d.x) || std::abs(dm.y) > std::abs(d.y))) {
    preferV = preferH = false;
  }

  // 终裁（无偏好时）：|dx| < |dy| → 先垂直，否则先水平
  if (!preferH && !preferV) {
    if (std::abs(d.x) < std::abs(d.y)) breakVertical();
    else breakHorizontal();
  }

  return {mid, C};
}
}

wxBEGIN_EVENT_TABLE(WxCanvasPanel, wxPanel)
  EVT_PAINT(WxCanvasPanel::on_paint)
  EVT_LEFT_DOWN(WxCanvasPanel::on_left_down)
  EVT_LEFT_UP(WxCanvasPanel::on_left_up)
  EVT_LEFT_DCLICK(WxCanvasPanel::on_left_dclick)
  EVT_RIGHT_DOWN(WxCanvasPanel::on_right_down)
  EVT_RIGHT_UP(WxCanvasPanel::on_right_up)
  EVT_MOTION(WxCanvasPanel::on_motion)
  EVT_KEY_DOWN(WxCanvasPanel::on_key_down)
  EVT_CHAR(WxCanvasPanel::on_char)
  EVT_MOUSEWHEEL(WxCanvasPanel::on_wheel)
wxEND_EVENT_TABLE()

WxCanvasPanel::WxCanvasPanel(wxWindow* parent)
    : wxPanel(parent), scene_(CanvasScene::from_model(SchematicModel{})) {
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  SetCursor(wxCursor(wxCURSOR_CROSS));
}

void WxCanvasPanel::set_scene(CanvasScene scene) { scene_ = std::move(scene); Refresh(); }

void WxCanvasPanel::rebuild_scene() {
  if (engine_ && builder_) {
    const double z = scene_.zoom();
    const CanvasPoint p = scene_.pan();
    scene_ = builder_(*engine_);
    scene_.set_viewport(z, p);   // B3c：重建不丢 viewport（此前每次点击缩放弹回自适应）
  }
  if (ghost_) kicad_adapter::AddGhostPrimitives(scene_, *ghost_);   // B3b：放置幽灵附加
  if (on_selection_changed_) on_selection_changed_();               // K6：选中变化上抛
  Refresh();
}

VECTOR2I WxCanvasPanel::event_iu(const wxMouseEvent& event) const {
  const CanvasPoint scene = scene_.to_scene({static_cast<double>(event.GetX()),
                                             static_cast<double>(event.GetY())});
  return kicad_adapter::ViewToIu(scene);
}

int WxCanvasPanel::acc_iu() const {
  const double iu_per_px = kicad_adapter::kScale * scene_.zoom();
  return std::clamp(static_cast<int>(std::lround(kPickTolPx / iu_per_px)), 1, kAccIuCap);
}

bool WxCanvasPanel::in_selection(const void* source) const {
  if (!engine_ || !source) return false;
  const SCH_ITEM* item = static_cast<const SCH_ITEM*>(source);
  const auto& sel = engine_->Selection();
  return std::find(sel.begin(), sel.end(), item) != sel.end();
}

void WxCanvasPanel::reset_gesture() {
  state_ = State::IDLE;
  movable_ = true;
  preview_delta_ = VECTOR2I(0, 0);
  chain_.clear();
  wire_posture_ = true;
  wire_prev_dir_ = VECTOR2I(0, 0);
  wire_has_preview_ = false;
}

// WIRE_DRAW：冻结当前预览折点对（mid, cur）到链里；anchor=cur；posture 继承本段方向
void WxCanvasPanel::freeze_wire_pair(const VECTOR2I& cur) {
  const VECTOR2I anchor = chain_.empty() ? VECTOR2I(0, 0) : chain_.back();
  const auto [mid, end] = computeWireBreak(anchor, cur, wire_prev_dir_, wire_posture_);
  if (mid != anchor) chain_.push_back(mid);
  chain_.push_back(end);
  wire_prev_dir_ = end - mid;      // 上一冻结段方向（posture 继承）
  wire_posture_ = true;
  wire_mid_ = wire_cur_ = VECTOR2I(0, 0);
}

void WxCanvasPanel::finish_wire() {
  if (engine_ && chain_.size() >= 2) engine_->CommitWireChain(chain_);
  const bool had = chain_.size() >= 2;
  chain_.clear();
  reset_gesture();
  if (HasCapture()) ReleaseMouse();
  if (had) { rebuild_scene(); if (on_commit_) on_commit_(); }   // K6：画线提交 → 写回
  Refresh();
}

void WxCanvasPanel::place_enter(const wxString& lib) {
  if (!engine_) return;
  place_exit();
  ghost_lib_ = lib;
  ghost_ = engine_->CreateSymbol(lib, VECTOR2I(12700, 12700));   // 初始网格位，motion 跟随
  state_ = State::PLACE_MODE;
  rebuild_scene();
}

void WxCanvasPanel::place_exit() {
  delete ghost_;
  ghost_ = nullptr;
  if (state_ == State::PLACE_MODE) state_ = State::IDLE;
  rebuild_scene();
}

void WxCanvasPanel::on_paint(wxPaintEvent&) {
  wxAutoBufferedPaintDC dc(this);
  {
    const auto& bg = scene_.background();
    dc.SetBackground(wxBrush(wxColour(bg.r, bg.g, bg.b)));
  }
  dc.Clear();
  {
    const auto& bg = scene_.background();
    const bool light = (bg.r + bg.g + bg.b) > 384;
    // B3c：可见网格 = 真吸附网格（50mil=12700IU）——此前装饰性 20px 线与吸附网格脱节
    //（画线落点在 101.6px 网格上而视觉在 20px 网格上 → 严重不跟手/每次不一样）
    dc.SetPen(wxPen(wxColour(light ? 205 : 52, light ? 205 : 56, light ? 205 : 64), 1));
  }
  const auto size = GetClientSize();
  {
    double step = 12700.0 * kicad_adapter::kScale * scene_.zoom();  // 50mil 网格的屏幕间距
    if (step < 4.0) step *= 2.0;                                    // 过密 → 跳级（保持比例）
    if (step < 4.0) step *= 2.0;
    if (step < 4.0) step *= 2.0;
    const CanvasPoint o = scene_.to_view(kicad_adapter::IuToView(VECTOR2I(0, 0)));
    for (double x = o.x - std::floor(o.x / step) * step; x < size.x; x += step)
      dc.DrawLine(static_cast<int>(x), 0, static_cast<int>(x), size.y);
    for (double y = o.y - std::floor(o.y / step) * step; y < size.y; y += step)
      dc.DrawLine(0, static_cast<int>(y), size.x, static_cast<int>(y));
  }

  // 拖移预览偏移（场景坐标=kIuToView 空间，线性 → dIU*kScale）
  const CanvasPoint dir_off = state_ == State::DRAG_MOVE
    ? CanvasPoint{preview_delta_.x * kicad_adapter::kScale, preview_delta_.y * kicad_adapter::kScale}
    : CanvasPoint{};

  for (const auto& primitive : scene_.primitives()) {
    if (primitive.kind == CanvasPrimitive::Kind::Grid) continue;

    const bool move_offset = state_ == State::DRAG_MOVE &&
                             (dir_off.x != 0 || dir_off.y != 0) && in_selection(primitive.source);

    const auto vp = [&](CanvasPoint p) -> wxPoint {
      if (move_offset) { p.x += dir_off.x; p.y += dir_off.y; }
      const CanvasPoint out = scene_.to_view(p);
      return wxPoint(static_cast<int>(out.x), static_cast<int>(out.y));
    };

    if (primitive.style.has_value()) {
      const auto& st = *primitive.style;
      const wxColour col(st.colorR, st.colorG, st.colorB, st.colorA);
      const double wpx = std::max(1.0, st.width * scene_.zoom());
      if (primitive.kind == CanvasPrimitive::Kind::Junction && !primitive.points.empty()) {
        dc.SetPen(wxPen(col, 1)); dc.SetBrush(wxBrush(col));
        { const wxPoint c = vp(primitive.points[0]); dc.DrawCircle(c, static_cast<int>(std::max(2.0, wpx / 2))); }
      } else if (primitive.kind == CanvasPrimitive::Kind::Label) {
        dc.SetPen(wxPen(col, static_cast<int>(std::max(2.0, wpx)))); dc.SetBrush(*wxTRANSPARENT_BRUSH);
        for (const auto& line : st.glyphs) {
          if (line.size() < 2) continue;
          for (size_t i = 1; i < line.size(); ++i) {
            const wxPoint a = vp(line[i-1]);
            const wxPoint b = vp(line[i]);
            dc.DrawLine(a, b);
          }
        }
      } else {
        dc.SetPen(wxPen(col, static_cast<int>(std::max(2.0, wpx)))); dc.SetBrush(st.filled ? wxBrush(col) : *wxTRANSPARENT_BRUSH);
        for (size_t i = 1; i < primitive.points.size(); ++i) {
          const wxPoint a = vp(primitive.points[i-1]);
          const wxPoint b = vp(primitive.points[i]);
          dc.DrawLine(a, b);
        }
        if (primitive.kind == CanvasPrimitive::Kind::SymbolBody && primitive.points.size() == 4) {
          const wxPoint a = vp(primitive.points[3]);
          const wxPoint b = vp(primitive.points[0]);
          dc.DrawLine(a, b);
        }
        if (primitive.kind == CanvasPrimitive::Kind::Pin && primitive.points.size() == 2) {
          // pin 尖端空心圈（KiCad 未连接 pin 指示的简化；纸底填充保证高倍可见）
          const double r = std::max(2.5, wpx * 0.9);
          const auto& bgcol = scene_.background();
          dc.SetPen(wxPen(col, 1));
          dc.SetBrush(wxBrush(wxColour(bgcol.r, bgcol.g, bgcol.b)));
          const wxPoint tip = vp(primitive.points[1]);
          dc.DrawCircle(tip, static_cast<int>(r));
        }
      }
      continue;
    }

    if (primitive.points.empty()) continue;
    const auto point = [&](std::size_t index) { return vp(primitive.points[index]); };
    if (primitive.kind == CanvasPrimitive::Kind::Wire) {
      dc.SetPen(wxPen(wxColour(102, 190, 220), 2));
      for (std::size_t i = 1; i < primitive.points.size(); ++i) {
        const wxPoint a = point(i-1), b = point(i);
        dc.DrawLine(a, b);
      }
    } else if (primitive.kind == CanvasPrimitive::Kind::SymbolBody) {
      dc.SetPen(wxPen(wxColour(220, 180, 90), 2));
      if (primitive.points.size() >= 2) dc.DrawRectangle(wxRect(point(0), point(1)));
      dc.DrawText(primitive.text, point(0) + wxPoint(4, 4));
    } else if (primitive.kind == CanvasPrimitive::Kind::Pin) {
      dc.SetPen(wxPen(wxColour(120, 220, 130), 2)); dc.DrawCircle(point(0), 4);
      dc.DrawText(primitive.text, point(0) + wxPoint(6, -6));
    } else if (primitive.kind == CanvasPrimitive::Kind::Label) {
      dc.SetTextForeground(wxColour(240, 240, 240)); dc.DrawText(primitive.text, point(0) + wxPoint(5, 5));
    } else if (primitive.kind == CanvasPrimitive::Kind::Junction) {
      dc.SetBrush(wxBrush(wxColour(230, 100, 100))); dc.DrawCircle(point(0), 4);
    } else if (primitive.kind == CanvasPrimitive::Kind::NoConnect) {
      dc.SetPen(wxPen(wxColour(240, 100, 100), 2)); dc.DrawLine(point(0) - wxPoint(5, 5), point(0) + wxPoint(5, 5));
      dc.DrawLine(point(0) - wxPoint(5, -5), point(0) + wxPoint(5, -5));
    }
  }

  // dangling 端点指示器（KiCad 常显空心方框；sch_painter.cpp:1712-1739；
  // 颜色=wire 主题色即默认绿 builtin_color_themes.h:77）。纸底填充 → 任何缩放级别下
  // 与线宽重叠仍可见（此前 6px 透明框在高倍下被线色吞没——B3c 目检反馈）
  if (engine_) {
    const auto& bgcol = scene_.background();
    const wxColour paper(bgcol.r, bgcol.g, bgcol.b);
    dc.SetPen(wxPen(wxColour(0, 150, 0), 1)); dc.SetBrush(wxBrush(paper));
    for (const auto& primitive : scene_.primitives()) {
      if (!primitive.source) continue;
      const SCH_ITEM* src = static_cast<const SCH_ITEM*>(primitive.source);
      if (src->Type() != SCH_LINE_T) continue;
      const SCH_LINE* line = static_cast<const SCH_LINE*>(src);
      auto sq = [&](const VECTOR2I& p) {
        const CanvasPoint v = scene_.to_view(kicad_adapter::IuToView(p));
        dc.DrawRectangle(static_cast<int>(v.x) - 4, static_cast<int>(v.y) - 4, 8, 8);
      };
      if (line->IsStartDangling()) sq(line->GetStartPoint());
      if (line->IsEndDangling()) sq(line->GetEndPoint());
    }
  }

  // WIRE_DRAW 预览：冻结链 + 实时折点对（模型未动；wire_has_preview_ 前不画实时对）
  if (state_ == State::WIRE_DRAW && wire_has_preview_) {
    dc.SetPen(wxPen(wxColour(0, 150, 0), 2));
    if (!chain_.empty()) {
      for (size_t i = 1; i < chain_.size(); ++i) {
        const CanvasPoint a = scene_.to_view(kicad_adapter::IuToView(chain_[i-1]));
        const CanvasPoint b = scene_.to_view(kicad_adapter::IuToView(chain_[i]));
        dc.DrawLine(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(b.x), static_cast<int>(b.y));
      }
      const CanvasPoint anchor = scene_.to_view(kicad_adapter::IuToView(chain_.back()));
      const CanvasPoint mid = scene_.to_view(kicad_adapter::IuToView(wire_mid_));
      const CanvasPoint cur = scene_.to_view(kicad_adapter::IuToView(wire_cur_));
      dc.DrawLine(static_cast<int>(anchor.x), static_cast<int>(anchor.y), static_cast<int>(mid.x), static_cast<int>(mid.y));
      dc.DrawLine(static_cast<int>(mid.x), static_cast<int>(mid.y), static_cast<int>(cur.x), static_cast<int>(cur.y));
    }
  }

  // 框选虚线框（屏幕 px 现画）
  if (state_ == State::MARQUEE) {
    const double dx = marquee_b_.x - marquee_a_.x, dy = marquee_b_.y - marquee_a_.y;
    if (dx * dx + dy * dy > kDragClickPx * kDragClickPx) {
      dc.SetPen(wxPen(wxColour(115, 173, 255), 1, wxPENSTYLE_SHORT_DASH));
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.DrawRectangle(wxRect(static_cast<int>(std::min(marquee_a_.x, marquee_b_.x)),
                              static_cast<int>(std::min(marquee_a_.y, marquee_b_.y)),
                              static_cast<int>(std::abs(dx)), static_cast<int>(std::abs(dy))));
    }
  }
}

void WxCanvasPanel::on_wheel(wxMouseEvent& event) {
  const CanvasPoint cursor{static_cast<double>(event.GetX()), static_cast<double>(event.GetY())};
  const double factor = event.GetWheelRotation() > 0 ? 1.15 : 1.0 / 1.15;
  scene_.zoom_at(cursor, factor);
  Refresh();
}

void WxCanvasPanel::on_left_down(wxMouseEvent& event) {
  SetFocus();
  if (!engine_) { reset_gesture(); Refresh(); return; }

  const VECTOR2I p = event_iu(event);

  // B3b 放置：点击 = 落位 + 连续放置（下一次点击继续；ESC 退出）
  if (state_ == State::PLACE_MODE) {
    if (engine_ && ghost_) {
      const VECTOR2I pos = snaps_iu(p);
      engine_->PlaceSymbol(ghost_);
      if (on_commit_) on_commit_();   // K6：放置提交 → 写回
      ghost_ = engine_->CreateSymbol(ghost_lib_, pos);
      rebuild_scene();
    }
    Refresh();
    return;
  }

  // WIRE_DRAW 中：按下=记录（UP 无位移才算"点击"冻结——按住拖动=仅预览不冻结，
  // 根治"按住左键→幽灵线"——B3c 用户复测仍见，本版改为 UP 判定）
  if (state_ == State::WIRE_DRAW) {
    wire_press_ = {static_cast<double>(event.GetX()), static_cast<double>(event.GetY())};
    return;
  }

  CaptureMouse();
  const int acc = acc_iu();
  SCH_SYMBOL* pin_owner = nullptr;
  SCH_ITEM* hit = engine_->HitTest(p, acc, &pin_owner);

  if (hit) {
    engine_->SetSelection(std::vector<SCH_ITEM*>{hit});
    state_ = State::PRESS_ITEM;
    grab_iu_ = p;
    movable_ = hit->Type() != SCH_JUNCTION_T && hit->Type() != SCH_LINE_T;
    // junction 不可拖（点不得脱离线段）；**线也不可拖**——用户裁定（2026-09-05）：
    // 线段拖动与 KiCad 语义差异过大（段拉抻/自动跨段/吸附端点），宁砍不仿：错→Del 重画。
    // 线的移动方式 = 端点按下→续写（新线网格化），或整段 Del 后重建。

    // B3c 用户勘误裁定：按线端点（dangling）→ 直接续写（绝不进入整线移动；KiCad
    // IsPointClickableAnchor 只认悬空端 + auto_start_wires）；按中心段才能拖移。
    if (auto* line = dynamic_cast<SCH_LINE*>(hit)) {
      const VECTOR2I s = line->GetStartPoint(), e = line->GetEndPoint();
      const int r_end = acc;
      const bool near_s = (s - p).EuclideanNorm() <= r_end;
      const bool near_e = (e - p).EuclideanNorm() <= r_end;
      const bool wire_end = near_s && line->IsStartDangling();
      const bool wire_end2 = near_e && line->IsEndDangling();
      if (wire_end || wire_end2) {
        const VECTOR2I end = wire_end ? s : e;
        chain_ = { end };
        wire_prev_dir_ = e - s;                 // 被点旧线方向（posture 继承→先导共线）
        wire_posture_ = true;
        wire_mid_ = wire_cur_ = end;
        wire_has_preview_ = false;              // 未 motion 前不画实时对（防 (0,0) 幽灵）
        state_ = State::WIRE_DRAW;
        Refresh();
        return;
      }
    }
  } else {
    engine_->ClearSelection();
    state_ = State::PRESS_BLANK;
    marquee_a_ = marquee_b_ = {static_cast<double>(event.GetX()), static_cast<double>(event.GetY())};
  }
  rebuild_scene();
}

VECTOR2I WxCanvasPanel::snaps_iu(const VECTOR2I& p) const {
  const double iu_per_px = kicad_adapter::kScale * scene_.zoom();
  const int snap_rad = std::max(acc_iu(), static_cast<int>(std::lround(kSnapPx / iu_per_px)));
  auto sn = cicada::kicad_geometry::SnapEndpoints(engine_->screen(), engine_->sheetPath(), p, snap_rad);
  return sn ? sn->pt : cicada::kicad_geometry::SnapGrid(p);
}

void WxCanvasPanel::on_motion(wxMouseEvent& event) {
  if (!engine_) return;

  if (state_ == State::PLACE_MODE) {
    if (ghost_) {
      ghost_->SetPosition(snaps_iu(event_iu(event)));   // ghost 吸附跟手
      Refresh();
    }
  } else if (state_ == State::WIRE_DRAW) {
    const VECTOR2I cur = snaps_iu(event_iu(event));
    const VECTOR2I anchor = chain_.empty() ? VECTOR2I(0, 0) : chain_.back();
    const auto [mid, end] = computeWireBreak(anchor, cur, wire_prev_dir_, wire_posture_);
    wire_mid_ = mid;
    wire_cur_ = end;
    wire_has_preview_ = true;
    Refresh();
  } else if (state_ == State::PRESS_ITEM && movable_) {
    // 无阈值：motion 即进入拖移预览（模型冻结）
    state_ = State::DRAG_MOVE;
    const VECTOR2I cur = event_iu(event);
    preview_delta_ = cicada::kicad_geometry::SnapGrid(cur) - cicada::kicad_geometry::SnapGrid(grab_iu_);
    Refresh();
  } else if (state_ == State::DRAG_MOVE) {
    const VECTOR2I cur = event_iu(event);
    preview_delta_ = cicada::kicad_geometry::SnapGrid(cur) - cicada::kicad_geometry::SnapGrid(grab_iu_);
    Refresh();
  } else if (state_ == State::PRESS_BLANK) {
    const CanvasPoint cur{static_cast<double>(event.GetX()), static_cast<double>(event.GetY())};
    const double dx = cur.x - marquee_a_.x, dy = cur.y - marquee_a_.y;
    if (dx * dx + dy * dy > kDragClickPx * kDragClickPx) {
      state_ = State::MARQUEE;
      marquee_b_ = cur;
      Refresh();
    }
  } else if (state_ == State::MARQUEE) {
    marquee_b_ = {static_cast<double>(event.GetX()), static_cast<double>(event.GetY())};
    Refresh();
  }
}

void WxCanvasPanel::on_left_up(wxMouseEvent& event) {
  if (!engine_) { reset_gesture(); return; }

  switch (state_) {
  case State::DRAG_MOVE:
    if (preview_delta_.x != 0 || preview_delta_.y != 0) {
      engine_->MoveSelection(preview_delta_);
      if (on_commit_) on_commit_();   // K6：移动/拖拽提交 → 写回
    }
    rebuild_scene();
    break;
  case State::PRESS_ITEM:
    rebuild_scene();   // 续线已在 down 直接进入；此处=纯选择结束
    break;
  case State::MARQUEE: {
    const VECTOR2I c1 = kicad_adapter::ViewToIu(scene_.to_scene(marquee_a_));
    const VECTOR2I c2 = kicad_adapter::ViewToIu(scene_.to_scene(marquee_b_));
    engine_->SetSelection(engine_->BoxSelect(c1, c2, /*aContained=*/true));
    rebuild_scene();
    break;
  }
  case State::PRESS_BLANK:
    rebuild_scene();   // 选中/清选已发生在 down；此时仅确保高亮一致
    break;
  case State::WIRE_DRAW: {
    // 单击=冻结（按下与抬起 ≤3px 才算点击；按住拖动=仅预览）
    const double dx = event.GetX() - wire_press_.x, dy = event.GetY() - wire_press_.y;
    if (dx * dx + dy * dy <= kDragClickPx * kDragClickPx) {
      const VECTOR2I cur = snaps_iu(event_iu(event));
      freeze_wire_pair(cur);
      if (engine_->screen() && engine_->screen()->IsTerminalPoint(cur, LAYER_WIRE))
        finish_wire();   // 落在 terminal point → 自动结束
    }
    Refresh();
    return;              // 手势持续（双击/右键/ESC 才结束）
  }
  }

  reset_gesture();
  if (HasCapture()) ReleaseMouse();
  Refresh();
}

void WxCanvasPanel::on_left_dclick(wxMouseEvent& event) {
  if (state_ == State::WIRE_DRAW) {
    // wxMSW DBLCLK 代替第二个 DOWN：双击 = 直接结束（最终点已由 UP1 的"点击"冻结）
    finish_wire();
  } else {
    event.Skip();
  }
}

void WxCanvasPanel::on_right_down(wxMouseEvent& event) {
  if (state_ == State::WIRE_DRAW) {
    finish_wire();   // v1 简化：右键 = 结束（KiCad 是菜单，含结束）
  } else {
    event.Skip();
  }
}

// K6：右键抬起 → 命中即选中该图元（无命中保留当前选择）→ 弹宿主上下文菜单
void WxCanvasPanel::on_right_up(wxMouseEvent& event) {
  if (!engine_ || state_ == State::WIRE_DRAW) { event.Skip(); return; }
  const VECTOR2I p = event_iu(event);
  const int acc = acc_iu();
  SCH_SYMBOL* pin_owner = nullptr;
  if (SCH_ITEM* hit = engine_->HitTest(p, acc, &pin_owner)) {
    if (!in_selection(hit)) {
      engine_->SetSelection(std::vector<SCH_ITEM*>{hit});
      rebuild_scene();
    }
  }
  if (on_context_menu_) {
    on_context_menu_(wxPoint(event.GetX(), event.GetY()));
  } else {
    event.Skip();
  }
}

void WxCanvasPanel::on_key_down(wxKeyEvent& event) {
  if (!engine_) { event.Skip(); return; }

  if (event.ControlDown()) {
    if (event.GetKeyCode() == 'Z') {           // Ctrl+Z：CHAR 侧是控制码 → 必须 KEY_DOWN 侧
      engine_->Undo();
      rebuild_scene();
      if (on_commit_) on_commit_();   // K6：undo → 写回
      return;
    }
    if (event.GetKeyCode() == 'Y') {
      engine_->Redo();
      rebuild_scene();
      if (on_commit_) on_commit_();   // K6：redo → 写回
      return;
    }
  }

  if (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_BACK) {
    engine_->DeleteSelection();
    rebuild_scene();
    if (on_commit_) on_commit_();   // K6：删除 → 写回
    return;
  }

  if (event.GetKeyCode() == WXK_ESCAPE) {
    if (state_ == State::PLACE_MODE) { place_exit(); return; }   // B3b：ESC 退出放置
    reset_gesture();   // 取消整条（模型从未动 → 零 undo，KiCad ESC 语义）
    if (HasCapture()) ReleaseMouse();
    Refresh();
    return;
  }

  event.Skip();
}

void WxCanvasPanel::on_char(wxKeyEvent& event) {
  // 纯字母热键单面（wxMSW KEY_DOWN 处理过就不再发 CHAR）
  const int k = event.GetUnicodeKey();

  if (state_ == State::PLACE_MODE) {
    // 放置态：1/2/3 换库条目；R 旋转、X/Y 镜像（B3b-2 姿态；KiCad 放置工具语义）
    switch (k) {
    case '1': place_enter("R"); break;
    case '2': place_enter("C"); break;
    case '3': place_enter("LED"); break;
    case 'R': case 'r':
      if (ghost_) ghost_->Rotate(ghost_->GetPosition(), /*aRotateCCW=*/true);
      rebuild_scene();
      break;
    case 'X': case 'x':
      if (ghost_) ghost_->SetMirrorX(!ghost_->GetMirrorX());
      rebuild_scene();
      break;
    case 'Y': case 'y':
      if (ghost_) ghost_->SetMirrorY(!ghost_->GetMirrorY());
      rebuild_scene();
      break;
    default:
      break;
    }
    return;
  }

  switch (k) {
  case '1': place_enter("R"); break;      // B3b：内置库条目选择（palette 简版=数字键）
  case '2': place_enter("C"); break;
  case '3': place_enter("LED"); break;
  default:
    break;
  }
}

} // namespace cicada::editor
