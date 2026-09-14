#pragma once

#include "canvas_scene.h"

#include <wx/panel.h>
#include <functional>
#include <optional>
#include <vector>

#include <math/vector2d.h>

namespace cicada::kicad_geometry
{
class SchInteractionEngine;
} // namespace cicada::kicad_geometry

class SCH_ITEM; // 全局命名空间（sch_item.h，仅指针）
class SCH_LINE;
class SCH_SYMBOL;
class SCH_SYMBOL;

namespace cicada::editor {

// B3a-2：面板 = 引擎的 wx 驱动（点选/拖移/端点拖/框选/undo 状态机 + 预览绘制）。
// 模型侧原则（B3a2-wx接线探路 §B.3）：运动期模型冻结、面板预览偏移；up 才落引擎原子操作。
class WxCanvasPanel final : public wxPanel {
 public:
  using Engine = cicada::kicad_geometry::SchInteractionEngine;
  using Builder = std::function<CanvasScene(Engine&)>;   // 实现自取 e.Selection() 做高亮

  explicit WxCanvasPanel(wxWindow* parent);
  void set_scene(CanvasScene scene);
  void set_engine(Engine* engine, Builder builder) { engine_ = engine; builder_ = std::move(builder); rebuild_scene(); }
  // K6：文件装载/外部变更后由宿主刷新画布（重建场景、保持 viewport）
  void refresh_from_engine() { rebuild_scene(); }
  // K6：选中集变化回调（down/框选/删除/undo 等重建后触发；宿主用它上抛 selection）
  void set_on_selection_changed(std::function<void()> handler) { on_selection_changed_ = std::move(handler); }
  // K6：右键菜单回调（右键命中→选中该图元→回调；宿主弹"加入到上下文"等菜单）
  void set_on_context_menu(std::function<void(const wxPoint& pos)> handler) { on_context_menu_ = std::move(handler); }
  // K6 写回：引擎原子操作完成回调（画线提交/移动/放置/删除/undo/redo → 宿主写回文件）
  void set_on_commit(std::function<void()> handler) { on_commit_ = std::move(handler); }

 private:
  enum class State {
    IDLE,          // 无手势
    PRESS_ITEM,    // 按下命中（潜在拖移；motion 即预览；up 无位移+端点dangling→WIRE_DRAW）
    PRESS_BLANK,   // 按下空白（潜在框选；>3px 才进 MARQUEE，否则=空白点击清选）
    MARQUEE,       // 框选预览
    DRAG_MOVE,     // 拖移预览（模型冻结）
    WIRE_DRAW,     // B3c 续线：端点点击后继续画（45°折角、单击拐点、双击/右键结束、ESC取消）
    PLACE_MODE     // B3b 放置：ghost 随光标，点击落位+连续放置，R/X 旋转镜像，ESC 退出
  };

  void on_paint(wxPaintEvent& event);
  void on_left_down(wxMouseEvent& event);
  void on_left_up(wxMouseEvent& event);
  void on_left_dclick(wxMouseEvent& event);
  void on_right_down(wxMouseEvent& event);
  void on_right_up(wxMouseEvent& event);
  void on_motion(wxMouseEvent& event);
  void on_key_down(wxKeyEvent& event);
  void on_char(wxKeyEvent& event);
  void on_wheel(wxMouseEvent& event);

  void rebuild_scene();
  // 事件 px → IU（唯一换算链：to_scene → ViewToIu；适配器头）
  VECTOR2I event_iu(const wxMouseEvent& event) const;
  // 点选容差 IU（px 容差÷(kScale·zoom)，上限防退化全屏）
  int acc_iu() const;
  bool in_selection(const void* source) const;
  void reset_gesture();
  VECTOR2I snaps_iu(const VECTOR2I& p) const;               // 吸附优先、miss 落网格
  void freeze_wire_pair(const VECTOR2I& cur);               // WIRE_DRAW：冻结折点对
  void finish_wire();                                       // WIRE_DRAW：提交整条链
  void place_enter(const wxString& lib);                    // B3b：进入放置（建 ghost）
  void place_exit();                                        // B3b：退出放置（删 ghost）

  CanvasScene scene_;
  Engine* engine_ = nullptr;
  Builder builder_;
  std::function<void()> on_selection_changed_;
  std::function<void(const wxPoint&)> on_context_menu_;
  std::function<void()> on_commit_;

  State state_ = State::IDLE;
  bool movable_ = true;        // 按下项是否可拖（junction=false）
  VECTOR2I grab_iu_{};         // 按下点 IU（DRAG_MOVE 的 SnapGrid 基准）
  VECTOR2I preview_delta_{};   // DRAG_MOVE 预览偏移（IU）
  // WIRE_DRAW（B3c：KiCad 无端点拖；续线语义 per computeBreakPoint）
  std::vector<VECTOR2I> chain_;            // 冻结顶点 [起点, 拐点×n, 当前]
  VECTOR2I wire_mid_{};                    // 预览折点
  VECTOR2I wire_cur_{};                    // 预览光标落点
  VECTOR2I wire_prev_dir_{};               // 上一冻结段方向（posture 继承）
  bool wire_posture_ = true;               // 姿态继承（续线=旧线方向；45° 模式）
  bool wire_has_preview_ = false;          // 首次 motion 后才画实时对（防 enter 时 (0,0) 幽灵）
  CanvasPoint wire_press_{};               // WIRE_DRAW 中按下点（UP 判定"点击"=≤3px 才冻结）
  CanvasPoint marquee_a_{};
  CanvasPoint marquee_b_{};
  // B3b 放置
  SCH_SYMBOL* ghost_ = nullptr;    // 幽灵符号（未入屏；点击落位）
  wxString ghost_lib_;             // 当前放置的库条目（连续放置复用模板）

  wxDECLARE_EVENT_TABLE();
};
}
