#pragma once

#include "document_editor.h"
#include <string>
#include <string_view>
#include <vector>

namespace cicada::editor {

/** UI/protocol command gate. No command reaches the model except via this list. */
class ToolRegistry {
 public:
  explicit ToolRegistry(DocumentEditor& editor) : editor_(editor) {}
  static bool is_allowed(std::string_view name);
  bool invoke(std::string_view name, const EditCommand& command, std::string* error = nullptr);
  static std::vector<std::string> names();
 private:
  DocumentEditor& editor_;
};
}
