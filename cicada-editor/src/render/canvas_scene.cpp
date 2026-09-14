#include "canvas_scene.h"
#include "../core/connection_graph.h"
#include <algorithm>
#include <cmath>

namespace cicada::editor {
namespace {
CanvasPoint world(double x, double y, double zoom, CanvasPoint pan) { return {x * zoom + pan.x, y * zoom + pan.y}; }
}

CanvasScene CanvasScene::from_model(const SchematicModel& model, double zoom, CanvasPoint pan) {
  CanvasScene scene; scene.zoom_ = zoom > 0 ? zoom : 1.0; scene.pan_ = pan;
  scene.primitives_.push_back({CanvasPrimitive::Kind::Grid, "grid", {}, {}});
  for (const auto& wire : model.wires) {
    if (wire.points.size() != 2) continue;
    scene.primitives_.push_back({CanvasPrimitive::Kind::Wire, wire.uuid,
      {world(wire.points[0].first, wire.points[0].second, scene.zoom_, scene.pan_), world(wire.points[1].first, wire.points[1].second, scene.zoom_, scene.pan_)}, {}});
  }
  for (const auto& symbol : model.symbols) {
    const std::string id = symbol.uuid.empty() ? symbol.refdes : symbol.uuid;
    scene.primitives_.push_back({CanvasPrimitive::Kind::SymbolBody, id,
      {world(symbol.x_g - 254, symbol.y_g - 381, scene.zoom_, scene.pan_), world(symbol.x_g + 254, symbol.y_g + 381, scene.zoom_, scene.pan_)}, symbol.refdes});
    const auto lib = std::find_if(model.lib_symbols.begin(), model.lib_symbols.end(), [&](const auto& item) { return item.lib_id == symbol.lib_id; });
    if (lib == model.lib_symbols.end()) continue;
    for (const auto& instance_pin : symbol.pins) {
      const auto pin = std::find_if(lib->pins.begin(), lib->pins.end(), [&](const auto& item) { return item.number == instance_pin.number; });
      if (pin == lib->pins.end()) continue;
      const auto graph = ConnectionGraph::build(model);
      const auto pos = graph.pin_at(symbol.refdes, instance_pin.number);
      if (pos) scene.primitives_.push_back({CanvasPrimitive::Kind::Pin, id + "." + instance_pin.number,
        {world(pos->first, pos->second, scene.zoom_, scene.pan_)}, instance_pin.number});
    }
  }
  for (const auto& label : model.labels) scene.primitives_.push_back({CanvasPrimitive::Kind::Label, label.uuid, {world(label.x_g, label.y_g, scene.zoom_, scene.pan_)}, label.text});
  for (const auto& marker : model.junctions) scene.primitives_.push_back({CanvasPrimitive::Kind::Junction, marker.uuid, {world(marker.x_g, marker.y_g, scene.zoom_, scene.pan_)}, {}});
  for (const auto& marker : model.no_connects) scene.primitives_.push_back({CanvasPrimitive::Kind::NoConnect, marker.uuid, {world(marker.x_g, marker.y_g, scene.zoom_, scene.pan_)}, {}});
  return scene;
}

CanvasPoint CanvasScene::world_to_view(std::pair<int, int> point) const { return world(point.first, point.second, zoom_, pan_); }
CanvasPoint CanvasScene::view_to_world(CanvasPoint point) const { return {(point.x - pan_.x) / zoom_, (point.y - pan_.y) / zoom_}; }

std::optional<CanvasPrimitive> CanvasScene::hit_test(CanvasPoint view, double tolerance) const {
  view = to_scene(view);
  const auto distance_to_segment = [](CanvasPoint p, CanvasPoint a, CanvasPoint b) {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    if (dx == 0.0 && dy == 0.0) {
      const double px = p.x - a.x;
      const double py = p.y - a.y;
      return std::sqrt(px * px + py * py);
    }
    const double t = std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / (dx * dx + dy * dy), 0.0, 1.0);
    const double qx = a.x + t * dx - p.x;
    const double qy = a.y + t * dy - p.y;
    return std::sqrt(qx * qx + qy * qy);
  };
  // Later primitives are visually on top of earlier ones.
  for (auto it = primitives_.rbegin(); it != primitives_.rend(); ++it) {
    if (it->points.empty() || it->kind == CanvasPrimitive::Kind::Grid) continue;
    if (it->kind == CanvasPrimitive::Kind::SymbolBody && it->points.size() >= 2) {
      const auto& a = it->points[0];
      const auto& b = it->points[1];
      if (view.x >= std::min(a.x, b.x) - tolerance && view.x <= std::max(a.x, b.x) + tolerance &&
          view.y >= std::min(a.y, b.y) - tolerance && view.y <= std::max(a.y, b.y) + tolerance) return *it;
    } else if (it->kind == CanvasPrimitive::Kind::Wire && it->points.size() >= 2) {
      if (distance_to_segment(view, it->points[0], it->points[1]) <= tolerance) return *it;
    } else if (distance_to_segment(view, it->points.front(), it->points.front()) <= tolerance) {
      return *it;
    }
  }
  return std::nullopt;
}


void CanvasScene::zoom_at(CanvasPoint aView, double aFactor) {
  const double new_zoom = std::clamp(zoom_ * aFactor, 0.02, 50.0);
  const double world_x = (aView.x - pan_.x) / zoom_;
  const double world_y = (aView.y - pan_.y) / zoom_;
  zoom_ = new_zoom;
  pan_.x = aView.x - world_x * new_zoom;
  pan_.y = aView.y - world_y * new_zoom;
}

}
