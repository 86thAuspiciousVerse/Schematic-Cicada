// http_server.cpp — M1a：极简单线程 HTTP 服务（Boost.Beast + Asio，header-only）
#include "http_server.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <atomic>
#include <cstdio>

namespace beasthttp = boost::beast::http;
namespace asio      = boost::asio;
using tcp           = asio::ip::tcp;

namespace cicada::editor::service
{

int RunHttpServer( const std::string& aToken, int aPort,
                   const std::function<void( int )>& aOnReady,
                   const HttpHandler& aHandler, std::atomic<bool>& aStop,
                   std::function<void()>& aOutStopNow )
{
    try
    {
        asio::io_context ioc;
        tcp::acceptor    acceptor( ioc );
        tcp::endpoint    endpoint( asio::ip::address::from_string( "127.0.0.1" ), aPort );
        acceptor.open( endpoint.protocol() );
        acceptor.bind( endpoint );
        acceptor.listen( 16 );

        aOutStopNow = [&acceptor] { acceptor.close(); };
        const int actualPort = acceptor.local_endpoint().port();
        if( aOnReady )
            aOnReady( actualPort );

        while( !aStop.load() )
        {
            tcp::socket socket( ioc );
            try
            {
                acceptor.accept( socket );
            }
            catch( const std::exception& )
            {
                if( aStop.load() )
                    break;   // aStopNow 关闭 acceptor → accept 抛出 → 正常退出
                continue;    // 其它错误（如 accept 超限）重试
            }

            try
            {
                beasthttp::request<beasthttp::string_body> req;
                boost::beast::flat_buffer buffer;
                beasthttp::read( socket, buffer, req );

                const std::string token = req[ "X-Cicada-Token" ];
                std::string body;
                long        status = 200;

                // CORS 预检（OPTIONS）不带凭据头：必须先于 token 检查放行，
                // 否则浏览器跨源调用全部被 401 掐断（M1c 实测 "Failed to fetch"）。
                if( req.method() == beasthttp::verb::options )
                {
                    body = "{}";
                }
                else if( token != aToken )
                {
                    body = "{\"error\":{\"code\":\"unauthorized\",\"message\":\"token mismatch\"}}";
                    status = 401;
                }
                else
                {
                    body = aHandler( std::string( req.method_string() ), req.target(),
                                     req.body(), status );
                }

                beasthttp::response<beasthttp::string_body> res(
                    static_cast<beasthttp::status>( status ), req.version() );
                res.set( beasthttp::field::server, "cicada-engine" );
                res.set( beasthttp::field::content_type, "application/json" );
                res.set( beasthttp::field::access_control_allow_origin, "http://127.0.0.1:3123" );
                res.set( beasthttp::field::access_control_allow_headers, "X-Cicada-Token, Content-Type" );
                res.set( beasthttp::field::access_control_allow_methods, "GET, POST, OPTIONS" );
                res.set( "Access-Control-Max-Age", "3600" );
                res.body() = body;
                res.prepare_payload();
                beasthttp::write( socket, res );
            }
            catch( const std::exception& )
            {
                // 单请求失败不致命
            }
        }
        return 0;
    }
    catch( const std::exception& e )
    {
        fprintf( stderr, "[engine] http server fatal: %s\n", e.what() );
        return 1;
    }
}

} // namespace cicada::editor::service