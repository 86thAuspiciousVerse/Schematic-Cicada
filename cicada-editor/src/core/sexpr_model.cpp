#include "sexpr_model.h"

#include "kicad_sexpr_adapter.h"

namespace cicada::editor {

SchematicModel SexprModelReader::read(std::string_view text) const {
  // K1: KiCad-origin SEXPR is the sole syntax/model input.  The adapter owns
  // the fail-closed top-level allow-list and converts only the Cicada subset.
  return KiCadSexprAdapter().read_model(text);
}

}  // namespace cicada::editor
