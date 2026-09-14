#pragma once

#include <string_view>

namespace cicada::editor {
inline constexpr std::string_view kStatePath = "/cicada/editor/state";
inline constexpr std::string_view kSelectionPath = "/cicada/editor/selection";
inline constexpr std::string_view kWebSocketPath = "/cicada/editor/ws";
inline constexpr std::string_view kBearerPrefix = "Bearer ";
}
