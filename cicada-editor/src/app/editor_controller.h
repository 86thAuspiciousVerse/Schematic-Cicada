#pragma once

#include "host_spawner.h"
#include "stdout_parser.h"
#include "../bridge/event_dispatcher.h"

namespace cicada::editor {
enum class ControllerState { stopped, starting, ready, failed };

/** Small lifecycle coordinator; process ownership remains with the wx layer. */
class EditorController {
 public:
  explicit EditorController(HostLaunchSpec spec);
  void start_requested();
  void consume_stdout(std::string_view chunk);
  void process_exited(int exit_code);
  bool consume_frame(std::string_view frame_json);
  void clear_reload_request() { dispatcher_.clear_reload_request(); }   // K6：reload 消费后清标记
  ControllerState state() const { return state_; }
  const HostStdoutState& host() const { return parser_.state(); }
  const CicadaHostSpawner& spawner() const { return spawner_; }
  const EditorEventState& events() const { return dispatcher_.state(); }
 private:
  CicadaHostSpawner spawner_;
  StdoutParser parser_;
  EventDispatcher dispatcher_;
  ControllerState state_ = ControllerState::stopped;
};
}
