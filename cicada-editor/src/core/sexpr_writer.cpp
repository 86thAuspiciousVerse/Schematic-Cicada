#include "sexpr_writer.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace cicada::editor {
namespace {
std::string quote(const std::string& value) {
  std::string out = "\"";
  for (const char c : value) {
    if (c == '\\' || c == '\"') out.push_back('\\');
    out.push_back(c);
  }
  out.push_back('\"');
  return out;
}
std::string mm(int g) {
  std::ostringstream out; out << std::fixed << std::setprecision(2) << (static_cast<double>(g) / 100.0);
  return out.str();
}
void uuid_field(std::ostringstream& out, const std::string& uuid, const char* indent) {
  if (!uuid.empty()) out << indent << "(uuid " << uuid << ")\n";
}
}

std::string SexprModelWriter::write(const SchematicModel& model) const {
  if (model.version <= 0) throw std::runtime_error("cicada sexpr: invalid version");
  if (model.has_unrepresented_sections) throw std::runtime_error("cicada sexpr: model contains unrepresented sections");
  std::ostringstream out;
  out << "(kicad_sch (version " << model.version << ") (generator " << quote(model.generator.empty() ? "cicada" : model.generator)
      << ") (generator_version " << quote(model.generator_version.empty() ? "0.1" : model.generator_version) << ")\n";
  uuid_field(out, model.uuid, "  ");
  if (!model.lib_symbols.empty()) {
    out << "  (lib_symbols\n";
    for (const auto& symbol : model.lib_symbols) out << "    " << symbol.body << "\n";
    out << "  )\n";
  }
  for (const auto& s : model.symbols) {
    if (s.unit != 1 || !(s.rotation == 0 || s.rotation == 90 || s.rotation == 180 || s.rotation == 270))
      throw std::runtime_error("cicada sexpr: unsupported symbol transform");
    if (s.lib_id.empty() || s.refdes.empty() || s.value.empty()) throw std::runtime_error("cicada sexpr: incomplete symbol");
    out << "  (symbol (lib_id " << quote(s.lib_id) << ") (at " << mm(s.x_g) << " " << mm(s.y_g) << " " << s.rotation << ") (unit 1)\n";
    uuid_field(out, s.uuid, "    ");
    const char* ordered[] = {"Reference", "Value", "Footprint", "Datasheet"};
    for (const char* key : ordered) {
      const auto it = s.properties.find(key);
      if (it != s.properties.end()) out << "    (property " << quote(it->first) << " " << quote(it->second) << " (at " << mm(s.x_g) << " " << mm(s.y_g) << " 0))\n";
      else if (std::string(key) == "Reference") out << "    (property \"Reference\" " << quote(s.refdes) << " (at " << mm(s.x_g) << " " << mm(s.y_g) << " 0))\n";
      else if (std::string(key) == "Value") out << "    (property \"Value\" " << quote(s.value) << " (at " << mm(s.x_g) << " " << mm(s.y_g) << " 0))\n";
    }
    for (const auto& [key, value] : s.properties) {
      if (key == "Reference" || key == "Value" || key == "Footprint" || key == "Datasheet") continue;
      out << "    (property " << quote(key) << " " << quote(value) << " (at " << mm(s.x_g) << " " << mm(s.y_g) << " 0))\n";
    }
    for (const auto& pin : s.pins) {
      if (pin.number.empty()) throw std::runtime_error("cicada sexpr: incomplete pin");
      out << "    (pin " << quote(pin.number);
      if (!pin.uuid.empty()) out << " (uuid " << pin.uuid << ")";
      out << ")\n";
    }
    out << "  )\n";
  }
  for (const auto& w : model.wires) {
    if (w.points.size() != 2) throw std::runtime_error("cicada sexpr: wire must have exactly two points");
    out << "  (wire (pts (xy " << mm(w.points[0].first) << " " << mm(w.points[0].second) << ") (xy "
        << mm(w.points[1].first) << " " << mm(w.points[1].second) << "))";
    if (!w.uuid.empty()) out << " (uuid " << w.uuid << ")";
    out << ")\n";
  }
  for (const auto& l : model.labels) {
    if (!(l.rotation == 0 || l.rotation == 90 || l.rotation == 180 || l.rotation == 270)) throw std::runtime_error("cicada sexpr: unsupported label rotation");
    out << "  (label " << quote(l.text) << " (at " << mm(l.x_g) << " " << mm(l.y_g) << " " << l.rotation << ")";
    if (!l.uuid.empty()) out << " (uuid " << l.uuid << ")";
    out << ")\n";
  }
  for (const auto& j : model.junctions) { out << "  (junction (at " << mm(j.x_g) << " " << mm(j.y_g) << ")"; if (!j.uuid.empty()) out << " (uuid " << j.uuid << ")"; out << ")\n"; }
  for (const auto& n : model.no_connects) { out << "  (no_connect (at " << mm(n.x_g) << " " << mm(n.y_g) << ")"; if (!n.uuid.empty()) out << " (uuid " << n.uuid << ")"; out << ")\n"; }
  if (model.has_sheet_instances) {
    if (model.sheet_instances_body.empty()) throw std::runtime_error("cicada sexpr: sheet_instances body missing");
    out << "  " << model.sheet_instances_body << "\n";
  }
  out << ")\n";
  return out.str();
}
}
