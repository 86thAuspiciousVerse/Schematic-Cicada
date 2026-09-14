#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <utility>
#include <map>

namespace cicada::editor {
struct PinRecord { std::string number; std::string uuid; };
struct SymbolRecord {
  std::string refdes; std::string value; std::string lib_id;
  std::string uuid; std::map<std::string, std::string> properties;
  std::vector<PinRecord> pins; int x_g = 0; int y_g = 0;
  int rotation = 0; int unit = 1;
};
struct WireRecord { std::vector<std::pair<int, int>> points; std::string uuid; };
struct LabelRecord { std::string text; std::string uuid; int x_g = 0; int y_g = 0; int rotation = 0; };
struct JunctionRecord { std::string uuid; int x_g = 0; int y_g = 0; };
struct NoConnectRecord { std::string uuid; int x_g = 0; int y_g = 0; };
struct LibPinRecord {
  std::string number; std::string name; std::string type; std::string shape;
  int x_g = 0; int y_g = 0; int angle = 0; int length_g = 0;
};
struct LibSymbolRecord {
  std::string lib_id;
  std::vector<LibPinRecord> pins;
  // Canonical source body retained so writer never invents missing graphics.
  std::string body;
};
struct SchematicModel {
  int version = 20250114; std::string generator = "cicada";
  std::string generator_version = "0.1"; std::string uuid;
  // True when the source contained sections not yet represented by the
  // lightweight C++ model. Writers must reject these rather than drop data.
  bool has_unrepresented_sections = false;
  bool has_sheet_instances = false;
  std::vector<LibSymbolRecord> lib_symbols;
  std::vector<SymbolRecord> symbols;
  std::vector<WireRecord> wires;
  std::vector<LabelRecord> labels;
  std::vector<JunctionRecord> junctions;
  std::vector<NoConnectRecord> no_connects;
  // Sheet instances are not an editable M2 token, but preserving their
  // original node prevents a load/save cycle from inventing hierarchy data.
  std::string sheet_instances_body;
};

/**
 * Read-only M2 boundary model.
 *
 * The reader is a Cicada-owned adapter: it parses the S-expression structure
 * instead of searching raw substrings, rejects unsupported top-level tokens,
 * and never returns a partially populated model. It intentionally does not
 * duplicate the TypeScript format tokenizer or symbol generator; those remain
 * the DSH-side authorities. A future M2 slice may replace the private parser
 * with the selected KiCad sexpr source copied into this project.
 */
class SexprModelReader {
 public:
  SchematicModel read(std::string_view text) const;
};
}
