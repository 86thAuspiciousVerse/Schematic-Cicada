#include "tool_registry.h"

#include <algorithm>

namespace cicada::editor {
namespace {
struct Tool { const char* name; EditCommandKind kind; };
constexpr Tool kTools[] = {
  {"move_symbol", EditCommandKind::MoveSymbol},
  {"add_wire", EditCommandKind::AddWire},
  {"add_label", EditCommandKind::AddLabel},
  {"place_label", EditCommandKind::AddLabel},
  {"add_junction", EditCommandKind::AddJunction},
  {"add_no_connect", EditCommandKind::AddNoConnect},
  {"place_no_connect", EditCommandKind::AddNoConnect},
  {"set_property", EditCommandKind::SetProperty},
  {"delete_symbol", EditCommandKind::DeleteSymbol},
  {"remove_component", EditCommandKind::RemoveComponent},
  {"disconnect", EditCommandKind::Disconnect},
  {"delete_wire", EditCommandKind::DeleteWire},
  {"place_symbol", EditCommandKind::AddSymbol},
  {"place_power_symbol", EditCommandKind::AddPowerSymbol},
};
const Tool* find(std::string_view name) {
  for (const auto& tool : kTools) if (name == tool.name) return &tool;
  return nullptr;
}
}
bool ToolRegistry::is_allowed(std::string_view name) { return find(name) != nullptr; }
std::vector<std::string> ToolRegistry::names() {
  std::vector<std::string> result; for (const auto& tool : kTools) result.emplace_back(tool.name); return result;
}
bool ToolRegistry::invoke(std::string_view name, const EditCommand& command, std::string* error) {
  const Tool* tool = find(name);
  if (!tool) { if (error) *error = "unsupported Cicada tool"; return false; }
  if (tool->kind != command.kind) { if (error) *error = "tool/command kind mismatch"; return false; }
  return editor_.apply(command, error);
}
}
