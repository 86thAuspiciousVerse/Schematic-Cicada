#pragma once

#include "editor_ws_client.h"
#include "../render/canvas_document.h"
#include <cstdint>
#include <optional>

namespace cicada::editor {
struct EditorEventState {
  bool reload_requested = false;
  bool selection_confirmed = false;
  std::optional<std::string> file;
  std::optional<std::string> baseline_hash;
  std::optional<std::string> session_id;
  std::optional<std::uint64_t> changelog_seq;
  std::optional<std::int64_t> last_ping;
  unsigned datasheet_updates = 0;
};

/** Maps validated bridge frame types to canvas/controller state. */
class EventDispatcher {
 public:
  bool consume(std::string_view frame_json);
  const EditorEventState& state() const { return state_; }
  void clear_reload_request() { state_.reload_requested = false; }
 private:
  static std::optional<std::string> string_field(std::string_view json, std::string_view key);
  static std::optional<std::int64_t> integer_field(std::string_view json, std::string_view key);
  EditorEventState state_;
};
}
