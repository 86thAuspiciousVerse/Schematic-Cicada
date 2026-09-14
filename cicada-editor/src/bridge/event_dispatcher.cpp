#include "event_dispatcher.h"
#include <charconv>

namespace cicada::editor {
std::optional<std::string> EventDispatcher::string_field(std::string_view json, std::string_view key) {
  const auto marker = std::string("\"") + std::string(key) + "\"";
  const auto key_pos = json.find(marker);
  if (key_pos == std::string_view::npos) return std::nullopt;
  const auto colon = json.find(':', key_pos + marker.size());
  if (colon == std::string_view::npos) return std::nullopt;
  const auto begin = json.find('"', colon + 1);
  if (begin == std::string_view::npos) return std::nullopt;
  const auto end = json.find('"', begin + 1);
  if (end == std::string_view::npos) return std::nullopt;
  return std::string(json.substr(begin + 1, end - begin - 1));
}

std::optional<std::int64_t> EventDispatcher::integer_field(std::string_view json, std::string_view key) {
  const auto marker = std::string("\"") + std::string(key) + "\"";
  const auto key_pos = json.find(marker);
  if (key_pos == std::string_view::npos) return std::nullopt;
  const auto colon = json.find(':', key_pos + marker.size());
  if (colon == std::string_view::npos) return std::nullopt;
  auto begin = colon + 1;
  while (begin < json.size() && (json[begin] == ' ' || json[begin] == '\t')) ++begin;
  auto end = begin;
  while (end < json.size() && (json[end] == '-' || (json[end] >= '0' && json[end] <= '9'))) ++end;
  std::int64_t value = 0;
  const auto result = std::from_chars(json.data() + begin, json.data() + end, value);
  if (result.ec != std::errc{} || result.ptr != json.data() + end) return std::nullopt;
  return value;
}

bool EventDispatcher::consume(std::string_view frame_json) {
  const auto frame = EditorWsClient::parse_frame(frame_json);
  if (!frame.has_value()) return false;
  const auto& type = frame->type;
  if (type == "canvas.refresh") {
    state_.reload_requested = true;
    state_.file = string_field(frame_json, "file");
  } else if (type == "baseline") {
    state_.file = string_field(frame_json, "file");
    state_.baseline_hash = string_field(frame_json, "baselineHash");
  } else if (type == "changelog") {
    const auto seq = integer_field(frame_json, "seq");
    if (!seq.has_value() || *seq < 0) return false;
    state_.changelog_seq = static_cast<std::uint64_t>(*seq);
    state_.baseline_hash = string_field(frame_json, "baselineHash");
  } else if (type == "selection.confirm") {
    state_.selection_confirmed = true;
    state_.session_id = string_field(frame_json, "sessionId");
  } else if (type == "ping") {
    state_.last_ping = integer_field(frame_json, "t");
  } else if (type == "datasheet.update") {
    ++state_.datasheet_updates;
  }
  return true;
}
}
