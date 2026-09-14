#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace cicada::editor {
struct DownlinkFrame { std::string type; std::string json; };
class EditorWsClient {
 public:
  explicit EditorWsClient(std::string base_url = {});
  std::string endpoint() const;
  static bool is_known_frame_type(std::string_view type);
  static std::optional<DownlinkFrame> parse_frame(std::string_view json);
 private:
  std::string base_url_;
};
}
