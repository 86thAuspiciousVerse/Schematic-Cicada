#pragma once

#include "../core/sexpr_model.h"
#include <string>
#include <optional>
#include <vector>

namespace cicada::editor {
struct CanvasPoint { double x = 0; double y = 0; };
struct CanvasPrimitive {
  enum class Kind { Grid, Wire, SymbolBody, Pin, Label, Junction, NoConnect };
  Kind kind;
  std::string id;
  std::vector<CanvasPoint> points;
  std::string text;
  // B3a-2: 产生本图元的实模型项（选中映射/拖移预览平移）；非 KiCad 路径保持 nullptr
  const void* source = nullptr;
  // 可选样式（KiCad 几何批次接入后填充；缺省时画布回退到内置默认色）
  struct Style {
    int colorR = 255, colorG = 255, colorB = 255, colorA = 255;
    int width = 2;
    bool filled = false;
    double angleDeg = 0.0;
    int textSize = 0;
    std::vector<std::vector<CanvasPoint>> glyphs; // 每字形一条折线（视图坐标）
  };
  std::optional<Style> style;
};

/** Backend-neutral scene extraction for a GAL/wx or web canvas. */
class CanvasScene {
 public:
  static CanvasScene from_model(const SchematicModel& model, double zoom = 1.0, CanvasPoint pan = {});
  const std::vector<CanvasPrimitive>& primitives() const { return primitives_; }
  // 内部装配（KiCad 几何适配器注入逐条图元；坐标为视图坐标）
  void add_primitive(CanvasPrimitive prim) { primitives_.push_back(std::move(prim)); }
  // Hit testing is performed in view coordinates and returns a copy so the
  // caller can safely retain the result while a new scene is assembled.
  std::optional<CanvasPrimitive> hit_test(CanvasPoint view, double tolerance = 6.0) const;
  CanvasPoint world_to_view(std::pair<int, int> point) const;
  CanvasPoint view_to_world(CanvasPoint point) const;
  double zoom() const { return zoom_; }
  CanvasPoint pan() const { return pan_; }
  struct Background { int r = 28, g = 30, b = 34; };
  void set_background(int r, int g, int b) { bg_.r = r; bg_.g = g; bg_.b = b; }
  const Background& background() const { return bg_; }
  // 场景坐标(图元存储系) ↔ 视图坐标(屏幕) 变换；默认 zoom=1/pan=0 时恒等
  CanvasPoint to_view(CanvasPoint aScene) const {
    return CanvasPoint{ aScene.x * zoom_ + pan_.x, aScene.y * zoom_ + pan_.y };
  }
  CanvasPoint to_scene(CanvasPoint aView) const {
    return CanvasPoint{ (aView.x - pan_.x) / zoom_, (aView.y - pan_.y) / zoom_ };
  }
  // 以视图坐标 aView 为锚点缩放（鼠标滚轮语义）：锚点下的世界点保持不动
  void zoom_at(CanvasPoint aView, double aFactor);
  // B3c: 场景重建时保持 viewport（默认 1/{0,0}；不复用则每次交互后缩放/平移被重置）
  void set_viewport(double aZoom, CanvasPoint aPan) { zoom_ = aZoom; pan_ = aPan; }
 private:
  double zoom_ = 1.0; CanvasPoint pan_{};
  Background bg_;
  std::vector<CanvasPrimitive> primitives_;
};
}
