#pragma once

#include "editor_ws_client.h"
#include <string>

namespace cicada::editor {

/** Windows WinHTTP WebSocket client for editor-bridge downlink frames. */
class WinHttpWebSocket {
 public:
  WinHttpWebSocket() = default;
  ~WinHttpWebSocket();
  WinHttpWebSocket(const WinHttpWebSocket&) = delete;
  WinHttpWebSocket& operator=(const WinHttpWebSocket&) = delete;
  bool connect(std::string_view url, std::string_view token, std::string* error = nullptr);
  bool send_text(std::string_view text, std::string* error = nullptr);
  std::string receive_text(std::string* error = nullptr);
  void close();
  bool connected() const;

 private:
  void* session_ = nullptr;
  void* request_ = nullptr;
  void* socket_ = nullptr;
};
}
