#include "winhttp_transport.h"

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#include <string>
#pragma comment(lib, "winhttp.lib")

namespace cicada::editor {
namespace {
std::wstring wide(std::string_view text) {
  if (text.empty()) return {};
  const int n = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
  std::wstring result(static_cast<std::size_t>(n), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), n); return result;
}
HttpResponse request(std::wstring method, std::string_view url, std::string_view auth, std::string_view body) {
  URL_COMPONENTS parts{}; parts.dwStructSize = sizeof(parts); parts.dwSchemeLength = static_cast<DWORD>(-1); parts.dwHostNameLength = static_cast<DWORD>(-1); parts.dwUrlPathLength = static_cast<DWORD>(-1); parts.dwExtraInfoLength = static_cast<DWORD>(-1);
  std::wstring target = wide(url); if (!WinHttpCrackUrl(target.c_str(), 0, 0, &parts)) return {0, "WinHttpCrackUrl failed"};
  HINTERNET session = WinHttpOpen(L"Schematic-Cicada/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0); if (!session) return {0, "WinHttpOpen failed"};
  HINTERNET connect = WinHttpConnect(session, std::wstring(parts.lpszHostName, parts.dwHostNameLength).c_str(), parts.nPort, 0); if (!connect) { WinHttpCloseHandle(session); return {0, "WinHttpConnect failed"}; }
  std::wstring path(parts.lpszUrlPath, parts.dwUrlPathLength); path += std::wstring(parts.lpszExtraInfo, parts.dwExtraInfoLength);
  HINTERNET req = WinHttpOpenRequest(connect, method.c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, parts.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0); if (!req) { WinHttpCloseHandle(connect); WinHttpCloseHandle(session); return {0, "WinHttpOpenRequest failed"}; }
  std::wstring headers = L"Content-Type: application/json\r\nAuthorization: " + wide(auth) + L"\r\n";
  const std::string body_bytes(body); BOOL ok = WinHttpSendRequest(req, headers.c_str(), static_cast<DWORD>(-1), body.empty() ? WINHTTP_NO_REQUEST_DATA : const_cast<char*>(body_bytes.data()), static_cast<DWORD>(body_bytes.size()), static_cast<DWORD>(body_bytes.size()), 0) && WinHttpReceiveResponse(req, nullptr);
  if (!ok) { WinHttpCloseHandle(req); WinHttpCloseHandle(connect); WinHttpCloseHandle(session); return {0, "WinHTTP request failed"}; }
  DWORD status = 0, size = sizeof(status); WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
  std::string response; DWORD available = 0; while (WinHttpQueryDataAvailable(req, &available) && available) { std::string chunk(available, '\0'); DWORD read = 0; if (!WinHttpReadData(req, chunk.data(), available, &read)) break; chunk.resize(read); response += chunk; }
  WinHttpCloseHandle(req); WinHttpCloseHandle(connect); WinHttpCloseHandle(session); return {static_cast<int>(status), std::move(response)};
}
}
HttpResponse WinHttpTransport::get(std::string_view url, std::string_view authorization) { return request(L"GET", url, authorization, {}); }
HttpResponse WinHttpTransport::post(std::string_view url, std::string_view authorization, std::string_view body) { return request(L"POST", url, authorization, body); }
}
#else
namespace cicada::editor {
HttpResponse WinHttpTransport::get(std::string_view, std::string_view) { return {0, "WinHTTP is only available on Windows"}; }
HttpResponse WinHttpTransport::post(std::string_view, std::string_view, std::string_view) { return {0, "WinHTTP is only available on Windows"}; }
}
#endif
