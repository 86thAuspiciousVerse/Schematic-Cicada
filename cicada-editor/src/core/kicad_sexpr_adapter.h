#pragma once

#include <memory>
#include <string_view>

#include <sexpr/sexpr.h>

#include "sexpr_model.h"

namespace cicada::editor {

/**
 * Cicada-owned boundary around KiCad's upstream S-expression parser.
 *
 * The returned tree is deliberately not exposed as the editor model. The
 * separate read_model() operation converts the AST into the Cicada allow-list
 * projection while preserving the parser/model boundary.
 */
class KiCadSexprAdapter final {
 public:
  std::unique_ptr<SEXPR::SEXPR> parse(std::string_view text) const;
  SchematicModel read_model(std::string_view text) const;

 private:
  static void validate_root(const SEXPR::SEXPR& root);
};

}  // namespace cicada::editor
