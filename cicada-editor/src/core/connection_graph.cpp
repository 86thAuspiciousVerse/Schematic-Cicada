#include "connection_graph.h"

#include <map>
#include <set>
#include <algorithm>

namespace cicada::editor {
namespace {
bool on_segment(std::pair<int, int> p, std::pair<int, int> a, std::pair<int, int> b) {
  const long long cross = static_cast<long long>(p.first - a.first) * (b.second - a.second) - static_cast<long long>(p.second - a.second) * (b.first - a.first);
  if (cross != 0) return false;
  return p.first >= std::min(a.first, b.first) && p.first <= std::max(a.first, b.first) &&
         p.second >= std::min(a.second, b.second) && p.second <= std::max(a.second, b.second);
}
}
struct ConnectionGraph::Impl {
  using Point = std::pair<int, int>;
  std::map<Point, Point> parent;
  std::map<Point, std::string> labels;
  std::map<std::pair<std::string, std::string>, Point> pins;
  Point find(Point point) const {
    auto it = parent.find(point);
    if (it == parent.end() || it->second == point) return point;
    return find(it->second);
  }
};

ConnectionGraph::ConnectionGraph(std::shared_ptr<const Impl> impl) : impl_(std::move(impl)) {
  std::set<Impl::Point> roots;
  for (const auto& [point, ignored] : impl_->parent) { (void)ignored; roots.insert(impl_->find(point)); }
  component_count_ = roots.size();
}

ConnectionGraph ConnectionGraph::build(const SchematicModel& model) {
  auto impl = std::make_shared<Impl>();
  auto add = [&](Impl::Point point) { impl->parent.try_emplace(point, point); };
  auto join = [&](Impl::Point a, Impl::Point b) {
    add(a); add(b);
    const auto ra = impl->find(a); const auto rb = impl->find(b);
    if (ra != rb) impl->parent[ra] = rb;
  };
  std::vector<Impl::Point> anchors;
  for (const auto& wire : model.wires) if (wire.points.size() == 2) { add(wire.points[0]); add(wire.points[1]); anchors.push_back(wire.points[0]); anchors.push_back(wire.points[1]); }
  for (const auto& symbol : model.symbols) {
    const auto lib = std::find_if(model.lib_symbols.begin(), model.lib_symbols.end(), [&](const auto& item) { return item.lib_id == symbol.lib_id; });
    if (lib == model.lib_symbols.end()) continue;
    for (const auto& instance_pin : symbol.pins) {
      const auto pin = std::find_if(lib->pins.begin(), lib->pins.end(), [&](const auto& item) { return item.number == instance_pin.number; });
      if (pin == lib->pins.end()) continue;
      int x = pin->x_g; int y = -pin->y_g;
      switch (symbol.rotation) {
        case 0: break;
        case 90: { const int old_x = x; x = y; y = -old_x; break; }
        case 180: x = -x; y = -y; break;
        case 270: { const int old_x = x; x = -y; y = old_x; break; }
        default: continue;
      }
      const Impl::Point world{symbol.x_g + x, symbol.y_g + y};
      impl->pins[{symbol.refdes, instance_pin.number}] = world;
      add(world);
      anchors.push_back(world);
    }
  }
  for (const auto& junction : model.junctions) { const Impl::Point point{junction.x_g, junction.y_g}; add(point); anchors.push_back(point); }
  for (const auto& label : model.labels) { const Impl::Point point{label.x_g, label.y_g}; add(point); anchors.push_back(point); impl->labels[point] = label.text; }
  for (const auto& wire : model.wires) if (wire.points.size() == 2) {
    for (const auto& point : anchors) if (on_segment(point, wire.points[0], wire.points[1])) join(wire.points[0], point);
  }
  return ConnectionGraph(std::move(impl));
}

bool ConnectionGraph::connected(std::pair<int, int> a, std::pair<int, int> b) const {
  if (!impl_) return false;
  if (impl_->parent.find(a) == impl_->parent.end() || impl_->parent.find(b) == impl_->parent.end()) return a == b;
  return impl_->find(a) == impl_->find(b);
}

std::optional<std::pair<int, int>> ConnectionGraph::pin_at(std::string_view refdes, std::string_view number) const {
  if (!impl_) return std::nullopt;
  const auto it = impl_->pins.find({std::string(refdes), std::string(number)});
  return it == impl_->pins.end() ? std::nullopt : std::optional<std::pair<int, int>>(it->second);
}

std::optional<std::string> ConnectionGraph::label_at(std::pair<int, int> point) const {
  if (!impl_) return std::nullopt;
  const auto it = impl_->labels.find(point);
  return it == impl_->labels.end() ? std::nullopt : std::optional<std::string>(it->second);
}
}
