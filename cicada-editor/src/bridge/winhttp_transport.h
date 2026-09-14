#pragma once

#include "editor_http_client.h"

namespace cicada::editor {
/** Windows WinHTTP implementation of the editor bridge transport. */
class WinHttpTransport final : public IHttpTransport {
 public:
  HttpResponse get(std::string_view url, std::string_view authorization) override;
  HttpResponse post(std::string_view url, std::string_view authorization, std::string_view body) override;
};
}
