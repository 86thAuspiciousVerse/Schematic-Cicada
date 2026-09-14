// json_util.h — M1a 引擎服务：JSON 转义 / SHA-256（BCrypt）/ 文件读取（无外部依赖）
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#pragma comment( lib, "bcrypt.lib" )

#include <fstream>
#include <sstream>
#include <string>

namespace cicada::editor::service
{

// 转义为 JSON 字符串字面量内容（不含引号）
inline std::string json_escape( const std::string& aText )
{
    std::string out;
    out.reserve( aText.size() + 8 );
    for( unsigned char c : aText )
    {
        switch( c )
        {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if( c < 0x20 )
            {
                char b[ 8 ];
                snprintf( b, sizeof( b ), "\\u%04x", c );
                out += b;
            }
            else
            {
                out += static_cast<char>( c );
            }
        }
    }
    return out;
}

// 文件内容 SHA-256（小写 hex）；读取失败返回空串
inline std::string file_sha256( const std::string& aPath )
{
    std::ifstream input( aPath, std::ios::binary );
    if( !input )
        return {};
    std::ostringstream contents;
    contents << input.rdbuf();
    if( !input.good() && !input.eof() )
        return {};
    const std::string data = contents.str();

    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    if( !BCRYPT_SUCCESS( BCryptOpenAlgorithmProvider( &alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0 ) ) )
        return {};
    if( !BCRYPT_SUCCESS( BCryptCreateHash( alg, &hash, nullptr, 0, nullptr, 0, 0 ) ) )
    {
        BCryptCloseAlgorithmProvider( alg, 0 );
        return {};
    }
    BCryptHashData( hash, reinterpret_cast<PUCHAR>( const_cast<char*>( data.data() ) ),
                    static_cast<ULONG>( data.size() ), 0 );
    BYTE digest[ 32 ] = { 0 };
    BCryptFinishHash( hash, digest, sizeof( digest ), 0 );
    BCryptDestroyHash( hash );
    BCryptCloseAlgorithmProvider( alg, 0 );

    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve( 64 );
    for( BYTE b : digest )
    {
        out.push_back( hex[ b >> 4 ] );
        out.push_back( hex[ b & 0x0f ] );
    }
    return out;
}

} // namespace cicada::editor::service
