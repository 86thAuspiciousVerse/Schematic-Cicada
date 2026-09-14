#pragma once

#include <string>
#include <string_view>

namespace cicada::editor {
struct HttpResponse { int status = 0; std::string body; };
class IHttpTransport {
 public:
  virtual ~IHttpTransport() = default;
  virtual HttpResponse get(std::string_view url, std::string_view authorization) = 0;
  virtual HttpResponse post(std::string_view url, std::string_view authorization, std::string_view body) = 0;
};

class EditorHttpClient {
 public:
  explicit EditorHttpClient(std::string base_url = {});
  std::string state_url() const;
  std::string selection_url() const;
  std::string authorization_header(std::string_view token) const;
  std::string selection_body(std::string_view selection_json, std::string_view session_id = {},
                             bool attach_next_turn = false) const;
  HttpResponse get_state(IHttpTransport& transport, std::string_view token) const;
  HttpResponse post_selection(IHttpTransport& transport, std::string_view token,
                              std::string_view selection_json, std::string_view session_id = {},
                              bool attach_next_turn = false) const;
 private:
  std::string base_url_;
};
}
