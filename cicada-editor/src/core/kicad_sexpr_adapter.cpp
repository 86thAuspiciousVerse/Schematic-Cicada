#include "kicad_sexpr_adapter.h"

#include <stdexcept>
#include <string>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <initializer_list>
#include <set>

#include <sexpr/sexpr_parser.h>

namespace cicada::editor {
namespace {

bool is_allowed_top_level(std::string_view head) {
  return head == "version" || head == "generator" || head == "generator_version"
      || head == "uuid" || head == "paper" || head == "title_block" || head == "lib_symbols"
      || head == "symbol" || head == "wire" || head == "junction" || head == "label"
      || head == "no_connect" || head == "sheet_instances";
}

bool balanced_single_expression(std::string_view text) {
  int depth = 0;
  bool quoted = false;
  bool escaped = false;
  bool saw_root = false;
  bool root_done = false;
  for (const char c : text) {
    if (root_done) {
      if (!std::isspace(static_cast<unsigned char>(c))) return false;
      continue;
    }
    if (quoted) {
      if (escaped) { escaped = false; continue; }
      if (c == '\\') { escaped = true; continue; }
      if (c == '"') quoted = false;
      continue;
    }
    if (c == '"') { quoted = true; continue; }
    if (c == '(') {
      if (root_done) return false;
      saw_root = true;
      ++depth;
    } else if (c == ')') {
      if (depth == 0) return false;
      --depth;
      if (depth == 0) root_done = true;
    }
  }
  return saw_root && !quoted && !escaped && depth == 0 && root_done;
}

std::string node_head(const SEXPR::SEXPR& node) {
  if (!node.IsList() || node.GetNumberOfChildren() == 0) {
    throw std::runtime_error("cicada sexpr: list is missing a head");
  }
  const auto* head = node.GetChild(0);
  if (head == nullptr || !head->IsSymbol()) {
    throw std::runtime_error("cicada sexpr: list head must be a symbol");
  }
  return head->GetSymbol();
}

const SEXPR::SEXPR* child_node(const SEXPR::SEXPR& node, std::string_view head, std::size_t occurrence = 0) {
  if (!node.IsList()) return nullptr;
  for (std::size_t i = 1; i < node.GetNumberOfChildren(); ++i) {
    const auto* child = node.GetChild(i);
    if (child != nullptr && child->IsList() && node_head(*child) == head) {
      if (occurrence == 0) return child;
      --occurrence;
    }
  }
  return nullptr;
}

std::vector<const SEXPR::SEXPR*> child_nodes(const SEXPR::SEXPR& node, std::string_view head) {
  std::vector<const SEXPR::SEXPR*> result;
  if (!node.IsList()) return result;
  for (std::size_t i = 1; i < node.GetNumberOfChildren(); ++i) {
    const auto* child = node.GetChild(i);
    if (child != nullptr && child->IsList() && node_head(*child) == head) result.push_back(child);
  }
  return result;
}

void validate_child_tokens(const SEXPR::SEXPR& node,
                           std::initializer_list<std::string_view> allowed,
                           std::string_view context,
                           std::initializer_list<std::size_t> allowed_atom_indices = {}) {
  for (std::size_t i = 1; i < node.GetNumberOfChildren(); ++i) {
    const auto* child = node.GetChild(i);
    if (child == nullptr) {
      throw std::runtime_error("cicada sexpr: null " + std::string(context) + " child");
    }
    // Every child after a node head is itself a named S-expression in the
    // Cicada whitelist.  Silently skipping an atom here would accept an
    // unknown token and then lose it on write (for example `(wire junk ...)`).
    if (!child->IsList()) {
      bool allowed_atom = false;
      for (const auto index : allowed_atom_indices) {
        if (index == i) {
          allowed_atom = true;
          break;
        }
      }
      if (allowed_atom) continue;
      throw std::runtime_error("cicada sexpr: unsupported atom in " + std::string(context));
    }
    const std::string head = node_head(*child);
    bool ok = false;
    for (const auto candidate : allowed) {
      if (head == candidate) { ok = true; break; }
    }
    if (!ok) {
      throw std::runtime_error("cicada sexpr: unsupported " + std::string(context) + " token " + head);
    }
  }
}

std::string leaf_value(const SEXPR::SEXPR& node, std::size_t index, const char* context) {
  if (!node.IsList() || index >= node.GetNumberOfChildren()) throw std::runtime_error(std::string("cicada sexpr: missing ") + context);
  const auto* value = node.GetChild(index);
  if (value == nullptr) throw std::runtime_error(std::string("cicada sexpr: missing ") + context);
  if (value->IsString()) return value->GetString();
  if (value->IsSymbol()) return value->GetSymbol();
  if (value->IsInteger()) return std::to_string(value->GetLongInteger());
  if (value->IsDouble()) {
    std::ostringstream out;
    out << std::setprecision(10) << value->GetDouble();
    return out.str();
  }
  throw std::runtime_error(std::string("cicada sexpr: expected leaf for ") + context);
}

std::string optional_leaf_value(const SEXPR::SEXPR& node, std::string_view head) {
  const auto* child = child_node(node, head);
  if (child == nullptr) return {};
  // 空可选字段（如 runtime 生成的 (pin "1" (uuid )) —— 只有 head 一个 child）＝缺省。
  if (child->GetNumberOfChildren() <= 1) return {};
  return leaf_value(*child, 1, "optional field");
}

int coordinate(const SEXPR::SEXPR& node, std::size_t index, const char* context) {
  const std::string value = leaf_value(node, index, context);
  char* end = nullptr;
  const double mm = std::strtod(value.c_str(), &end);
  if (end == value.c_str() || *end != '\0' || !std::isfinite(mm)) throw std::runtime_error(std::string("cicada sexpr: invalid ") + context);
  return static_cast<int>(mm * 100.0 + (mm >= 0.0 ? 0.5 : -0.5));
}

std::pair<int, int> at_xy(const SEXPR::SEXPR& node, const char* context) {
  const auto* at = child_node(node, "at");
  if (at == nullptr || at->GetNumberOfChildren() < 3) throw std::runtime_error(std::string("cicada sexpr: missing ") + context + " position");
  return {coordinate(*at, 1, context), coordinate(*at, 2, context)};
}

LibSymbolRecord parse_lib_symbol(const SEXPR::SEXPR& node) {
  LibSymbolRecord result;
  result.lib_id = leaf_value(node, 1, "library symbol id");
  result.body = node.AsString();
  for (const auto* unit : child_nodes(node, "symbol")) {
    for (const auto* pin : child_nodes(*unit, "pin")) {
      if (pin->GetNumberOfChildren() < 3) throw std::runtime_error("cicada sexpr: malformed library pin");
      LibPinRecord value;
      value.type = leaf_value(*pin, 1, "pin type");
      value.shape = leaf_value(*pin, 2, "pin shape");
      const auto* at = child_node(*pin, "at");
      if (at == nullptr || at->GetNumberOfChildren() < 4) throw std::runtime_error("cicada sexpr: library pin missing at");
      value.x_g = coordinate(*at, 1, "pin x");
      value.y_g = coordinate(*at, 2, "pin y");
      value.angle = std::stoi(leaf_value(*at, 3, "pin angle"));
      const auto* length = child_node(*pin, "length");
      if (length != nullptr && length->GetNumberOfChildren() >= 2) value.length_g = coordinate(*length, 1, "pin length");
      if (const auto* name = child_node(*pin, "name"); name != nullptr && name->GetNumberOfChildren() >= 2) value.name = leaf_value(*name, 1, "pin name");
      if (const auto* number = child_node(*pin, "number"); number != nullptr && number->GetNumberOfChildren() >= 2) value.number = leaf_value(*number, 1, "pin number");
      if (value.number.empty()) throw std::runtime_error("cicada sexpr: library pin missing number");
      result.pins.push_back(std::move(value));
    }
  }
  std::set<std::string> pin_numbers;
  for (const auto& pin : result.pins) {
    if (!pin_numbers.insert(pin.number).second) {
      throw std::runtime_error("cicada sexpr: duplicate library pin number " + pin.number);
    }
  }
  return result;
}

SymbolRecord parse_symbol(const SEXPR::SEXPR& node) {
  const auto* lib = child_node(node, "lib_id");
  if (lib == nullptr) throw std::runtime_error("cicada sexpr: symbol missing lib_id");
  SymbolRecord result;
  result.lib_id = leaf_value(*lib, 1, "lib_id");
  std::set<std::string> property_names;
  for (const auto* property : child_nodes(node, "property")) {
    // The list head occupies child 0, so require property name + value.
    if (property->GetNumberOfChildren() < 3) throw std::runtime_error("cicada sexpr: malformed symbol property");
    const std::string name = leaf_value(*property, 1, "property name");
    const std::string value = leaf_value(*property, 2, "property value");
    if (!property_names.insert(name).second) {
      throw std::runtime_error("cicada sexpr: duplicate symbol property " + name);
    }
    validate_child_tokens(*property, {"at"}, "property", {1, 2});
    result.properties[name] = value;
    if (name == "Reference") result.refdes = value;
    if (name == "Value") result.value = value;
  }
  result.uuid = optional_leaf_value(node, "uuid");
  if (child_node(node, "at") != nullptr) {
    const auto [x, y] = at_xy(node, "symbol"); result.x_g = x; result.y_g = y;
    const auto* at = child_node(node, "at");
    if (at->GetNumberOfChildren() >= 4) result.rotation = std::stoi(leaf_value(*at, 3, "symbol rotation"));
  }
  if (const auto* unit = child_node(node, "unit"); unit != nullptr && unit->GetNumberOfChildren() >= 2) result.unit = std::stoi(leaf_value(*unit, 1, "symbol unit"));
  if (result.unit != 1) throw std::runtime_error("cicada sexpr: unsupported symbol unit");
  if (!(result.rotation == 0 || result.rotation == 90 || result.rotation == 180 || result.rotation == 270)) throw std::runtime_error("cicada sexpr: unsupported symbol rotation");
  validate_child_tokens(node, {"lib_id", "at", "unit", "uuid", "property", "pin"}, "symbol");
  for (const auto* pin : child_nodes(node, "pin")) {
    if (pin->GetNumberOfChildren() < 2) throw std::runtime_error("cicada sexpr: malformed pin");
    result.pins.push_back({leaf_value(*pin, 1, "pin number"), optional_leaf_value(*pin, "uuid")});
  }
  if (result.refdes.empty() || result.value.empty()) throw std::runtime_error("cicada sexpr: symbol missing Reference/Value");
  return result;
}

}  // namespace

std::unique_ptr<SEXPR::SEXPR> KiCadSexprAdapter::parse(std::string_view text) const {
  if (!balanced_single_expression(text)) throw std::runtime_error("cicada sexpr (KiCad parser): malformed expression boundaries");
  try {
    SEXPR::PARSER parser;
    std::string source(text);
    auto root = parser.Parse(source);
    if (!root) throw std::runtime_error("empty expression");
    validate_root(*root);
    return root;
  } catch (const SEXPR::PARSE_EXCEPTION& error) {
    throw std::runtime_error(std::string("cicada sexpr (KiCad parser): ") + error.what());
  } catch (const SEXPR::INVALID_TYPE_EXCEPTION& error) {
    throw std::runtime_error(std::string("cicada sexpr (KiCad parser): ") + error.what());
  }
}

SchematicModel KiCadSexprAdapter::read_model(std::string_view text) const {
  auto root = parse(text);
  SchematicModel model;
  bool saw_version = false;
  std::set<std::string> lib_ids;
  std::set<std::string> refdeses;
  std::set<std::string> uuids;
  std::set<std::string> singleton_heads;
  auto remember_uuid = [&](const std::string& uuid, const char* context) {
    if (!uuid.empty() && !uuids.insert(uuid).second) {
      throw std::runtime_error(std::string("cicada sexpr: duplicate ") + context + " uuid " + uuid);
    }
  };
  for (std::size_t i = 1; i < root->GetNumberOfChildren(); ++i) {
    const auto* node = root->GetChild(i);
    const std::string head = node_head(*node);
    if (head == "version" || head == "generator" || head == "generator_version" ||
        head == "uuid" || head == "paper" || head == "title_block" ||
        head == "lib_symbols" || head == "sheet_instances") {
      if (!singleton_heads.insert(head).second) {
        throw std::runtime_error("cicada sexpr: duplicate top-level token " + head);
      }
    }
    if (head == "version") { model.version = std::stoi(leaf_value(*node, 1, "version")); saw_version = true; }
    else if (head == "generator") model.generator = leaf_value(*node, 1, "generator");
    else if (head == "generator_version") model.generator_version = leaf_value(*node, 1, "generator_version");
    else if (head == "uuid") {
      model.uuid = leaf_value(*node, 1, "uuid");
      remember_uuid(model.uuid, "root");
    }
    else if (head == "paper" || head == "title_block") model.has_unrepresented_sections = true;
    else if (head == "lib_symbols") {
      validate_child_tokens(*node, {"symbol"}, "lib_symbols");
      for (const auto* symbol : child_nodes(*node, "symbol")) {
        auto parsed = parse_lib_symbol(*symbol);
        if (!lib_ids.insert(parsed.lib_id).second) {
          throw std::runtime_error("cicada sexpr: duplicate library symbol " + parsed.lib_id);
        }
        model.lib_symbols.push_back(std::move(parsed));
      }
    }
    else if (head == "symbol") {
      auto parsed = parse_symbol(*node);
      if (!refdeses.insert(parsed.refdes).second) {
        throw std::runtime_error("cicada sexpr: duplicate symbol reference " + parsed.refdes);
      }
      std::set<std::string> pin_numbers;
      for (const auto& pin : parsed.pins) {
        if (!pin_numbers.insert(pin.number).second) {
          throw std::runtime_error("cicada sexpr: duplicate instance pin number " + pin.number);
        }
        remember_uuid(pin.uuid, "pin");
      }
      remember_uuid(parsed.uuid, "symbol");
      model.symbols.push_back(std::move(parsed));
    }
    else if (head == "wire") {
      validate_child_tokens(*node, {"pts", "uuid"}, "wire");
      const auto* pts = child_node(*node, "pts");
      if (pts == nullptr) throw std::runtime_error("cicada sexpr: wire missing pts");
      validate_child_tokens(*pts, {"xy"}, "wire pts");
      WireRecord wire; wire.uuid = optional_leaf_value(*node, "uuid");
      remember_uuid(wire.uuid, "wire");
      for (const auto* xy : child_nodes(*pts, "xy")) {
        if (xy->GetNumberOfChildren() < 3) throw std::runtime_error("cicada sexpr: malformed wire point");
        wire.points.emplace_back(coordinate(*xy, 1, "wire x"), coordinate(*xy, 2, "wire y"));
      }
      if (wire.points.size() != 2) throw std::runtime_error("cicada sexpr: wire must have exactly two points");
      model.wires.push_back(std::move(wire));
    } else if (head == "label") {
      validate_child_tokens(*node, {"at", "uuid"}, "label", {1});
      LabelRecord label; label.text = leaf_value(*node, 1, "label text");
      const auto [x, y] = at_xy(*node, "label"); label.x_g = x; label.y_g = y; label.uuid = optional_leaf_value(*node, "uuid");
      remember_uuid(label.uuid, "label");
      if (const auto* at = child_node(*node, "at"); at != nullptr && at->GetNumberOfChildren() >= 4) label.rotation = std::stoi(leaf_value(*at, 3, "label rotation"));
      model.labels.push_back(std::move(label));
    } else if (head == "junction" || head == "no_connect") {
      validate_child_tokens(*node, {"at", "uuid"}, head);
      const auto [x, y] = at_xy(*node, head.c_str());
      if (head == "junction") { JunctionRecord value; value.x_g = x; value.y_g = y; value.uuid = optional_leaf_value(*node, "uuid"); remember_uuid(value.uuid, "junction"); model.junctions.push_back(std::move(value)); }
      else { NoConnectRecord value; value.x_g = x; value.y_g = y; value.uuid = optional_leaf_value(*node, "uuid"); remember_uuid(value.uuid, "no_connect"); model.no_connects.push_back(std::move(value)); }
    } else if (head == "sheet_instances") {
      model.has_sheet_instances = true;
      model.sheet_instances_body = node->AsString();
    }
  }
  if (!saw_version) throw std::runtime_error("cicada sexpr: missing version");
  return model;
}

void KiCadSexprAdapter::validate_root(const SEXPR::SEXPR& root) {
  const std::string head = node_head(root);
  if (head != "kicad_sch") {
    throw std::runtime_error("cicada sexpr: expected kicad_sch root");
  }

  // KiCad's tree stores the list head as child 0.  Every remaining child must
  // be a list with an allow-listed top-level token.  Unknown tokens fail
  // closed before any Cicada model is constructed.
  for (std::size_t index = 1; index < root.GetNumberOfChildren(); ++index) {
    const auto* child = root.GetChild(index);
    if (child == nullptr || !child->IsList()) {
      throw std::runtime_error("cicada sexpr: top-level leaf is unsupported");
    }
    const std::string child_head = node_head(*child);
    if (!is_allowed_top_level(child_head)) {
      throw std::runtime_error("cicada sexpr: unsupported top-level token " + child_head);
    }
  }
}

}  // namespace cicada::editor
