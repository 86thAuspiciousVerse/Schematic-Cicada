#include "editor_ws_client.h"
#include "../core/editor_contract.h"
#include <utility>

namespace cicada::editor {
EditorWsClient::EditorWsClient(std::string base_url) : base_url_(std::move(base_url)) {}
std::string EditorWsClient::endpoint() const { return base_url_ + std::string(kWebSocketPath); }
bool EditorWsClient::is_known_frame_type(std::string_view type) {
  return type == "hello" || type == "canvas.refresh" || type == "changelog"
    || type == "baseline" || type == "datasheet.update"
    || type == "selection.confirm" || type == "ping";
}
std::optional<DownlinkFrame> EditorWsClient::parse_frame(std::string_view json) {
  const auto key = json.find("\"type\"");
  if (key == std::string_view::npos) return std::nullopt;
  const auto colon = json.find(':', key + 6);
  const auto begin = colon == std::string_view::npos ? colon : json.find('"', colon + 1);
  const auto end = begin == std::string_view::npos ? begin : json.find('"', begin + 1);
  if (begin == std::string_view::npos || end == std::string_view::npos) return std::nullopt;
  const auto type = json.substr(begin + 1, end - begin - 1);
  if (!is_known_frame_type(type)) return std::nullopt;
  return DownlinkFrame{std::string(type), std::string(json)};
}
}
