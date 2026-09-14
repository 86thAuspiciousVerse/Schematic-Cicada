#include "editor_http_client.h"
#include "../core/editor_contract.h"
#include <utility>

namespace cicada::editor {
EditorHttpClient::EditorHttpClient(std::string base_url) : base_url_(std::move(base_url)) {}
std::string EditorHttpClient::state_url() const { return base_url_ + std::string(kStatePath); }
std::string EditorHttpClient::selection_url() const { return base_url_ + std::string(kSelectionPath); }
std::string EditorHttpClient::authorization_header(std::string_view token) const {
  return std::string(kBearerPrefix) + std::string(token);
}
std::string EditorHttpClient::selection_body(std::string_view selection_json, std::string_view session_id,
                                             bool attach_next_turn) const {
  std::string body = "{\"selection\":" + std::string(selection_json);
  if (!session_id.empty()) body += ",\"sessionId\":\"" + std::string(session_id) + "\"";
  if (attach_next_turn) body += ",\"attachNextTurn\":true";
  body += "}";
  return body;
}
HttpResponse EditorHttpClient::get_state(IHttpTransport& transport, std::string_view token) const {
  return transport.get(state_url(), authorization_header(token));
}
HttpResponse EditorHttpClient::post_selection(IHttpTransport& transport, std::string_view token,
                                               std::string_view selection_json, std::string_view session_id,
                                               bool attach_next_turn) const {
  return transport.post(selection_url(), authorization_header(token),
                        selection_body(selection_json, session_id, attach_next_turn));
}
}
