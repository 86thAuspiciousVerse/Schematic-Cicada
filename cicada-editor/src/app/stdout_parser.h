#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace cicada::editor {
struct HostStdoutState {
  std::optional<std::string> web_ui_url;
  std::optional<unsigned> editor_port;
  std::optional<std::string> editor_token;
};

/** Chunk-safe parser for the two DSH/Cicada launcher stdout lines. */
class StdoutParser {
 public:
  void feed(std::string_view chunk);
  const HostStdoutState& state() const { return state_; }
 private:
  void consume_line(std::string line);
  std::string buffer_;
  HostStdoutState state_;
};
}
