#include "stdout_parser.h"

#include <charconv>
#include <string_view>
#include <utility>

namespace cicada::editor {
void StdoutParser::feed(std::string_view chunk) {
  buffer_.append(chunk);
  while (true) {
    const auto pos = buffer_.find('\n');
    if (pos == std::string::npos) break;
    std::string line = buffer_.substr(0, pos);
    buffer_.erase(0, pos + 1);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    consume_line(std::move(line));
  }
}

void StdoutParser::consume_line(std::string line) {
  constexpr std::string_view web_prefix = "dsh web: ";
  constexpr std::string_view editor_prefix = "cicada-editor: ";
  if (line.rfind(web_prefix, 0) == 0) {
    const auto value = line.substr(web_prefix.size());
    if (!value.empty()) state_.web_ui_url = value;
    return;
  }
  if (line.rfind(editor_prefix, 0) != 0) return;
  const auto rest = std::string_view(line).substr(editor_prefix.size());
  const auto split = rest.find(' ');
  if (split == std::string_view::npos || split == 0 || split + 1 >= rest.size()) return;
  unsigned port = 0;
  const auto parsed = std::from_chars(rest.data(), rest.data() + split, port);
  if (parsed.ec != std::errc{} || parsed.ptr != rest.data() + split) return;
  const auto token = rest.substr(split + 1);
  if (token.empty()) return;
  state_.editor_port = port;
  state_.editor_token = std::string(token);
}
}
