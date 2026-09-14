#include "document_editor.h"
#include "connection_graph.h"

#include <algorithm>
#include <stdexcept>

namespace cicada::editor {
namespace {
void clear_error(std::string* error) { if (error) error->clear(); }
void fail(std::string* error, const char* message) { if (error) *error = message; }
}

DocumentEditor::DocumentEditor(SchematicModel model) : model_(std::move(model)) {}

bool DocumentEditor::apply(const EditCommand& command, std::string* error) {
  SchematicModel next = model_;
  try {
    switch (command.kind) {
      case EditCommandKind::MoveSymbol: {
        auto it = std::find_if(next.symbols.begin(), next.symbols.end(), [&](const auto& s) { return s.refdes == command.refdes; });
        if (it == next.symbols.end()) throw std::runtime_error("symbol not found");
        const auto before_graph = ConnectionGraph::build(model_);
        const int dx = command.x_g - it->x_g; const int dy = command.y_g - it->y_g;
        it->x_g = command.x_g; it->y_g = command.y_g;
        // DRAG semantics: move connected wire endpoints with the symbol pins.
        for (const auto& pin : it->pins) {
          const auto old_pin = before_graph.pin_at(it->refdes, pin.number);
          if (!old_pin) continue;
          for (auto& wire : next.wires) for (auto& point : wire.points) {
            if (point == *old_pin) { point.first += dx; point.second += dy; }
          }
        }
        break;
      }
      case EditCommandKind::AddWire: {
        if (command.uuid.empty()) throw std::runtime_error("wire uuid required");
        if (std::any_of(next.wires.begin(), next.wires.end(), [&](const auto& w) { return w.uuid == command.uuid; })) throw std::runtime_error("wire uuid already exists");
        next.wires.push_back({{{command.x_g, command.y_g}, {command.x2_g, command.y2_g}}, command.uuid}); break;
      }
      case EditCommandKind::AddLabel: {
        if (command.uuid.empty() || command.value.empty()) throw std::runtime_error("label uuid and text required");
        if (std::any_of(next.labels.begin(), next.labels.end(), [&](const auto& l) { return l.uuid == command.uuid; })) throw std::runtime_error("label uuid already exists");
        next.labels.push_back({command.value, command.uuid, command.x_g, command.y_g, 0}); break;
      }
      case EditCommandKind::AddJunction:
        next.junctions.push_back({command.uuid, command.x_g, command.y_g}); break;
      case EditCommandKind::AddNoConnect:
        next.no_connects.push_back({command.uuid, command.x_g, command.y_g}); break;
      case EditCommandKind::SetProperty: {
        auto it = std::find_if(next.symbols.begin(), next.symbols.end(), [&](const auto& s) { return s.refdes == command.refdes; });
        if (it == next.symbols.end() || command.property.empty()) throw std::runtime_error("property target not found");
        if (command.property == "Reference") it->refdes = command.value;
        else if (command.property == "Value") it->value = command.value;
        it->properties[command.property] = command.value; break;
      }
      case EditCommandKind::DeleteSymbol: {
        const auto old = next.symbols.size();
        auto symbol = std::find_if(next.symbols.begin(), next.symbols.end(), [&](const auto& s) { return s.refdes == command.refdes; });
        if (symbol == next.symbols.end()) throw std::runtime_error("symbol not found");
        next.symbols.erase(std::remove_if(next.symbols.begin(), next.symbols.end(), [&](const auto& s) { return s.refdes == command.refdes; }), next.symbols.end());
        if (old == next.symbols.size()) throw std::runtime_error("symbol not found"); break;
      }
      case EditCommandKind::DeleteWire:
        if (std::none_of(next.wires.begin(), next.wires.end(), [&](const auto& w) { return w.uuid == command.uuid; })) throw std::runtime_error("wire not found");
        next.wires.erase(std::remove_if(next.wires.begin(), next.wires.end(), [&](const auto& w) { return w.uuid == command.uuid; }), next.wires.end()); break;
      case EditCommandKind::AddSymbol:
      case EditCommandKind::AddPowerSymbol: {
        if (command.refdes.empty() || command.value.empty() || command.lib_id.empty())
          throw std::runtime_error("symbol refdes, value and lib_id required");
        if (std::any_of(next.symbols.begin(), next.symbols.end(), [&](const auto& s) { return s.refdes == command.refdes; }))
          throw std::runtime_error("symbol reference already exists");
        const auto lib = std::find_if(next.lib_symbols.begin(), next.lib_symbols.end(),
          [&](const auto& item) { return item.lib_id == command.lib_id; });
        if (lib == next.lib_symbols.end()) throw std::runtime_error("symbol library definition not found");
        SymbolRecord symbol;
        symbol.refdes = command.refdes;
        symbol.value = command.value;
        symbol.lib_id = command.lib_id;
        symbol.uuid = command.uuid;
        symbol.x_g = command.x_g;
        symbol.y_g = command.y_g;
        symbol.properties["Reference"] = symbol.refdes;
        symbol.properties["Value"] = symbol.value;
        for (const auto& pin : lib->pins) symbol.pins.push_back({pin.number, {}});
        next.symbols.push_back(std::move(symbol));
        break;
      }
      case EditCommandKind::Disconnect: {
        if (command.refdes.empty() || command.pin_number.empty())
          throw std::runtime_error("disconnect requires refdes and pin number");
        const auto graph = ConnectionGraph::build(model_);
        const auto pin = graph.pin_at(command.refdes, command.pin_number);
        if (!pin) throw std::runtime_error("disconnect pin not found");
        const auto before = next.wires.size();
        next.wires.erase(std::remove_if(next.wires.begin(), next.wires.end(), [&](const auto& wire) {
          return std::any_of(wire.points.begin(), wire.points.end(), [&](const auto& point) { return point == *pin; });
        }), next.wires.end());
        if (before == next.wires.size()) throw std::runtime_error("disconnect pin has no wire");
        break;
      }
      case EditCommandKind::RemoveComponent: {
        if (command.refdes.empty()) throw std::runtime_error("component refdes required");
        const auto symbol = std::find_if(next.symbols.begin(), next.symbols.end(),
          [&](const auto& candidate) { return candidate.refdes == command.refdes; });
        if (symbol == next.symbols.end()) throw std::runtime_error("symbol not found");
        const auto graph = ConnectionGraph::build(model_);
        std::vector<std::pair<int, int>> pin_points;
        for (const auto& pin : symbol->pins) {
          if (const auto point = graph.pin_at(symbol->refdes, pin.number)) pin_points.push_back(*point);
        }
        const auto is_pin = [&](const std::pair<int, int>& point) {
          return std::find(pin_points.begin(), pin_points.end(), point) != pin_points.end();
        };
        next.symbols.erase(symbol);
        next.labels.erase(std::remove_if(next.labels.begin(), next.labels.end(), [&](const auto& label) { return is_pin({label.x_g, label.y_g}); }), next.labels.end());
        next.no_connects.erase(std::remove_if(next.no_connects.begin(), next.no_connects.end(), [&](const auto& marker) { return is_pin({marker.x_g, marker.y_g}); }), next.no_connects.end());
        std::vector<std::pair<int, int>> survivor_anchors;
        for (const auto& survivor : next.symbols) {
          for (const auto& survivor_pin : survivor.pins) {
            if (const auto point = graph.pin_at(survivor.refdes, survivor_pin.number)) survivor_anchors.push_back(*point);
          }
        }
        for (const auto& label : next.labels) survivor_anchors.push_back({label.x_g, label.y_g});
        const auto is_survivor_anchor = [&](const std::pair<int, int>& point) {
          return std::find(survivor_anchors.begin(), survivor_anchors.end(), point) != survivor_anchors.end();
        };
        next.wires.erase(std::remove_if(next.wires.begin(), next.wires.end(), [&](const auto& wire) {
          const bool touches_removed = std::any_of(wire.points.begin(), wire.points.end(), is_pin);
          if (!touches_removed) return false;
          const auto other = is_pin(wire.points.front()) ? wire.points.back() : wire.points.front();
          return !is_survivor_anchor(other);
        }), next.wires.end());
        break;
      }
    }
    (void)SexprModelWriter().write(next);
  } catch (const std::exception& ex) { fail(error, ex.what()); return false; }
  undo_.push_back({model_}); model_ = std::move(next); redo_.clear(); clear_error(error); return true;
}

bool DocumentEditor::restore(SchematicModel model, std::string* error) {
  try { (void)SexprModelWriter().write(model); } catch (const std::exception& ex) { fail(error, ex.what()); return false; }
  model_ = std::move(model); clear_error(error); return true;
}

bool DocumentEditor::undo(std::string* error) {
  if (undo_.empty()) { fail(error, "undo stack empty"); return false; }
  redo_.push_back({model_}); auto snapshot = std::move(undo_.back()); undo_.pop_back(); return restore(std::move(snapshot.model), error);
}
bool DocumentEditor::redo(std::string* error) {
  if (redo_.empty()) { fail(error, "redo stack empty"); return false; }
  undo_.push_back({model_}); auto snapshot = std::move(redo_.back()); redo_.pop_back(); return restore(std::move(snapshot.model), error);
}
std::string DocumentEditor::serialize(std::string* error) const {
  try { auto text = SexprModelWriter().write(model_); clear_error(error); return text; }
  catch (const std::exception& ex) { fail(error, ex.what()); return {}; }
}
}
