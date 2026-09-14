#pragma once

#include "sexpr_model.h"
#include <string>

namespace cicada::editor {

/**
 * Canonical writer for the Cicada-owned projection of a schematic.
 *
 * Only model fields represented by SchematicModel are emitted.  Library
 * symbol bodies are intentionally not invented here; the authoritative
 * cicada-symbols package remains responsible for supplying lib_symbols.
 */
class SexprModelWriter {
 public:
  std::string write(const SchematicModel& model) const;
};
}
