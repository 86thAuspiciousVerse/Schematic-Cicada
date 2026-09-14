#include "editor_controller.h"
#include <utility>

namespace cicada::editor {
EditorController::EditorController(HostLaunchSpec spec) : spawner_(std::move(spec)) {}
void EditorController::start_requested() { state_ = ControllerState::starting; }
void EditorController::consume_stdout(std::string_view chunk) {
  parser_.feed(chunk);
  if (parser_.state().web_ui_url.has_value() && parser_.state().editor_token.has_value()) state_ = ControllerState::ready;
}
void EditorController::process_exited(int exit_code) { state_ = exit_code == 0 ? ControllerState::stopped : ControllerState::failed; }
bool EditorController::consume_frame(std::string_view frame_json) { return dispatcher_.consume(frame_json); }
}
