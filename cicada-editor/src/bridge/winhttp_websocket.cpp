#include "winhttp_websocket.h"

#if defined(_WIN32) && defined(_MSC_VER)
#include <windows.h>
#include <winhttp.h>
#include <string>
#pragma comment(lib, "winhttp.lib")

namespace cicada::editor {
namespace {
std::wstring wide(std::string_view text) {
  if (text.empty()) return {};
  const int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
  if (size <= 0) return {};
  std::wstring result(static_cast<std::size_t>(size), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), size);
  return result;
}
}

WinHttpWebSocket::~WinHttpWebSocket() { close(); }

bool WinHttpWebSocket::connect(std::string_view url, std::string_view token, std::string* error) {
  close();
  URL_COMPONENTS parts{}; parts.dwStructSize = sizeof(parts); parts.dwSchemeLength = static_cast<DWORD>(-1); parts.dwHostNameLength = static_cast<DWORD>(-1); parts.dwUrlPathLength = static_cast<DWORD>(-1); parts.dwExtraInfoLength = static_cast<DWORD>(-1);
  // WinHttpCrackUrl 只认 http/https：ws:// → http://（wss:// → https://）仅用于解析
  // host/port/path；随后请求仍走 GET + Upgrade 握手（原始 URL 语义不变）。
  std::string crack_url(url);
  if (crack_url.rfind("ws://", 0) == 0) crack_url.replace(0, 5, "http://");
  else if (crack_url.rfind("wss://", 0) == 0) crack_url.replace(0, 6, "https://");
  const std::wstring target = wide(crack_url);
  if (!WinHttpCrackUrl(target.c_str(), 0, 0, &parts)) { if (error) *error = "WinHttpCrackUrl failed"; return false; }
  session_ = WinHttpOpen(L"Schematic-Cicada/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
  if (!session_) { if (error) *error = "WinHttpOpen failed"; return false; }
  const std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
  HINTERNET connect_handle = WinHttpConnect(static_cast<HINTERNET>(session_), host.c_str(), parts.nPort, 0);
  if (!connect_handle) { if (error) *error = "WinHttpConnect failed"; close(); return false; }
  const std::wstring path = std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) + std::wstring(parts.lpszExtraInfo, parts.dwExtraInfoLength);
  const DWORD flags = parts.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0;
  HINTERNET request = WinHttpOpenRequest(connect_handle, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags | WINHTTP_FLAG_ESCAPE_DISABLE);
  if (!request) { if (error) *error = "WinHttpOpenRequest failed"; WinHttpCloseHandle(connect_handle); close(); return false; }
  const std::wstring auth = L"Authorization: Bearer " + wide(token) + L"\r\n";
  WinHttpSetOption(request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0);
  const BOOL sent = WinHttpSendRequest(request, auth.c_str(), static_cast<DWORD>(-1), WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(request, nullptr);
  if (!sent) { if (error) *error = "WebSocket handshake failed"; WinHttpCloseHandle(request); WinHttpCloseHandle(connect_handle); close(); return false; }
  HINTERNET websocket = WinHttpWebSocketCompleteUpgrade(request, 0);
  WinHttpCloseHandle(request); WinHttpCloseHandle(connect_handle);
  if (!websocket) { if (error) *error = "WebSocket upgrade failed"; close(); return false; }
  request_ = nullptr; socket_ = websocket; if (error) error->clear(); return true;
}

bool WinHttpWebSocket::send_text(std::string_view text, std::string* error) {
  if (!socket_) { if (error) *error = "WebSocket is not connected"; return false; }
  const DWORD result = WinHttpWebSocketSend(static_cast<HINTERNET>(socket_), WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, const_cast<char*>(text.data()), static_cast<DWORD>(text.size()));
  if (result != ERROR_SUCCESS) { if (error) *error = "WebSocket send failed"; return false; }
  if (error) error->clear(); return true;
}

std::string WinHttpWebSocket::receive_text(std::string* error) {
  if (!socket_) { if (error) *error = "WebSocket is not connected"; return {}; }
  std::string result; std::string chunk(4096, '\0');
  for (;;) {
    DWORD read = 0; WINHTTP_WEB_SOCKET_BUFFER_TYPE type{};
    const DWORD status = WinHttpWebSocketReceive(static_cast<HINTERNET>(socket_), chunk.data(), static_cast<DWORD>(chunk.size()), &read, &type);
    if (status != ERROR_SUCCESS) { if (error) *error = "WebSocket receive failed"; return {}; }
    if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) { if (error) *error = "WebSocket closed"; return {}; }
    result.append(chunk.data(), read);
    if (type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE || type == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE) break;
  }
  if (error) error->clear(); return result;
}

void WinHttpWebSocket::close() {
  if (socket_) { WinHttpWebSocketClose(static_cast<HINTERNET>(socket_), WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0); WinHttpCloseHandle(static_cast<HINTERNET>(socket_)); socket_ = nullptr; }
  if (request_) { WinHttpCloseHandle(static_cast<HINTERNET>(request_)); request_ = nullptr; }
  if (session_) { WinHttpCloseHandle(static_cast<HINTERNET>(session_)); session_ = nullptr; }
}
bool WinHttpWebSocket::connected() const { return socket_ != nullptr; }
}
#else
namespace cicada::editor {
WinHttpWebSocket::~WinHttpWebSocket() = default;
bool WinHttpWebSocket::connect(std::string_view, std::string_view, std::string* error) { if (error) *error = "WebSocket is only implemented on Windows"; return false; }
bool WinHttpWebSocket::send_text(std::string_view, std::string* error) { if (error) *error = "WebSocket is only implemented on Windows"; return false; }
std::string WinHttpWebSocket::receive_text(std::string* error) { if (error) *error = "WebSocket is only implemented on Windows"; return {}; }
void WinHttpWebSocket::close() {}
bool WinHttpWebSocket::connected() const { return false; }
}
#endif
