#pragma once

#include "sexpr_model.h"
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace cicada::editor {

/** Geometric connectivity projected from the currently supported model. */
class ConnectionGraph {
 public:
  static ConnectionGraph build(const SchematicModel& model);
  bool connected(std::pair<int, int> a, std::pair<int, int> b) const;
  std::optional<std::pair<int, int>> pin_at(std::string_view refdes, std::string_view number) const;
  std::optional<std::string> label_at(std::pair<int, int> point) const;
  std::size_t component_count() const { return component_count_; }

 private:
  struct Impl;
  explicit ConnectionGraph(std::shared_ptr<const Impl> impl);
  std::shared_ptr<const Impl> impl_;
  std::size_t component_count_ = 0;
};
}
