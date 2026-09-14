#pragma once

#include "sexpr_model.h"

#include <string>
#include <string_view>
#include <vector>
#include <utility>

namespace cicada::editor {

enum class ScreenItemKind { symbol, wire, label, junction, no_connect };

struct ScreenItem {
  ScreenItemKind kind;
  std::string id;
  std::pair<int, int> anchor{0, 0};
};

/**
 * Headless screen seam for the K1 extraction slice.
 *
 * This is intentionally not a second editor implementation and does not
 * expose KiCad's SCH_SCREEN yet.  It gives the future SCH_SCREEN-backed
 * editor one stable load/save boundary while the Cicada allow-list model is
 * still the only writable representation.
 */
class CicadaScreenAdapter final {
 public:
  bool load(std::string_view text, std::string* error = nullptr);
  bool save(std::string* text, std::string* error = nullptr) const;

  bool loaded() const { return loaded_; }
  const SchematicModel& model() const { return model_; }
  std::vector<ScreenItem> items() const;
  std::string selection_json() const;

 private:
  SchematicModel model_;
  bool loaded_ = false;
};

}  // namespace cicada::editor
