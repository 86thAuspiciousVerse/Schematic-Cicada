// http_server.h — M1a：极简单线程 HTTP 服务（Boost.Beast，127.0.0.1 绑定 + token 鉴权 + CORS）
#pragma once

#include <atomic>
#include <functional>
#include <string>

namespace cicada::editor::service
{

// 路由回调：method + path + body(JSON 文本) → 响应 JSON 文本（HTTP 200）；返回空串 = 401。
// aOutStatus 可覆盖状态码（如 409）。
using HttpHandler = std::function<std::string( const std::string& aMethod,
                                                const std::string& aPath,
                                                const std::string& aBody,
                                                long& aOutStatus )>;

// 阻塞服务（单线程 accept 循环）；aOnReady(actualPort) 在绑定成功后调用（自宣告时机）。
// aStop：置位后服务循环退出（/shutdown 或外部）；aOutStopNow：出参——由本函数填入"立即使
// 阻塞的 accept 抛出"的钩子（关闭 acceptor），与 aStop 配合实现被阻塞 accept 也能退出。
// 返回 0=正常运行至停止（后续由调用方退出），非 0=致命失败。
int RunHttpServer( const std::string& aToken, int aPort,
                   const std::function<void( int )>& aOnReady,
                   const HttpHandler& aHandler, std::atomic<bool>& aStop,
                   std::function<void()>& aOutStopNow );

} // namespace cicada::editor::service
