// main_service.cpp — M1a：cicada-engine 服务进程
// 生命周期：解析参数 → 引擎装配（装载 .cicada_sch，可空文档）→ 自宣告
//   cicada-engine: 127.0.0.1:<EP> <ET>
//   → HTTP 服务（/scene /ops /hit /shutdown；鉴权 X-Cicada-Token；CORS 见 http_server）
//   → /shutdown 或 launcher kill。
// 懒重载：每次 /scene、/ops 前比对磁盘哈希，不同 → 整体重载（AI 写 = undo 栈刷新点）。
#include <kicad_bridge/cicada_document_bridge.h>
#include <kicad_bridge/sch_interaction_engine.h>
#include <service/http_server.h>
#include <service/json_util.h>
#include <service/lib_loader.h>
#include <service/ops.h>
#include <service/scene_json.h>
#include <service/shape_synth.h>
#include <service/wire_preview.h>
#include <core/sexpr_model.h>

#include <cctype>

#include <sch_item.h>

#include <stdio.h>
#include <windows.h>
#include <dbghelp.h>
#pragma comment( lib, "dbghelp.lib" )
#include <wx/debug.h>
#include <wx/string.h>

#include <filesystem>
#include <fstream>
#include <random>
#include <set>
#include <string>

namespace
{

// .cicada_sch 真相文件方言版本（与 DSH 侧 cicada-format 的 TRUTH_VERSION 一致）。
constexpr int kTruthVersion = 20260803;

std::string g_logPath;

void log_line( const std::string& aLine )
{
    fprintf( stderr, "%s\n", aLine.c_str() );
    fflush( stderr );
    if( !g_logPath.empty() )
    {
        std::ofstream f( g_logPath, std::ios::app );
        f << aLine << "\n";
    }
}

// ── 崩溃日志（exe 同目录 -crash.txt；与旧 wx 壳 SEH 方案一致）────────────────────
static void crash_write( const char* aText )
{
    static bool once = false;
    if( !once )
    {
        wchar_t exePath[ MAX_PATH ] = { 0 };
        GetModuleFileNameW( nullptr, exePath, MAX_PATH );
        std::wstring logPath( exePath );
        const auto dot = logPath.find_last_of( L".\\" );
        if( dot != std::wstring::npos && logPath[ dot ] == L'.' )
            logPath = logPath.substr( 0, dot );
        g_logPath = std::string( logPath.begin(), logPath.end() ) + "-crash.txt";
        once = true;
    }
    FILE* f = nullptr;
    _wfopen_s( &f, wxString( g_logPath ).ToStdWstring().c_str(), L"a" );
    if( f )
    {
        fprintf( f, "%s\n", aText );
        fclose( f );
    }
}

static LONG CALLBACK sehHandler( EXCEPTION_POINTERS* aInfo )
{
    char buf[ 256 ];
    snprintf( buf, sizeof( buf ), "=== CRASH code=0x%lx at %p ===",
              aInfo->ExceptionRecord->ExceptionCode, aInfo->ExceptionRecord->ExceptionAddress );
    crash_write( buf );

    HANDLE proc = GetCurrentProcess();
    SymInitialize( proc, nullptr, TRUE );
    SymSetOptions( SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME );
    void* frames[ 32 ];
    int   n = CaptureStackBackTrace( 0, 32, frames, nullptr );
    for( int i = 0; i < n; ++i )
    {
        DWORD64      dish = 0;
        SYMBOL_INFO* si = (SYMBOL_INFO*) calloc( 1, sizeof( SYMBOL_INFO ) + 512 );
        si->SizeOfStruct = sizeof( SYMBOL_INFO );
        si->MaxNameLen = 512;
        if( SymFromAddr( proc, (DWORD64) frames[ i ], &dish, si ) )
        {
            char line[ 512 ];
            snprintf( line, sizeof( line ), "  #%02d %s+0x%llx", i, si->Name,
                      (unsigned long long) dish );
            crash_write( line );
        }
        free( si );
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

static void serviceAssertHandler( const wxString& aFile, int aLine, const wxString& aFunc,
                                  const wxString& aCond, const wxString& aMsg )
{
    // 已知良性断言（SettingsManager 未注册 eeschema，引擎 ctor 链触发）：
    // 记录一行，不弹窗（弹窗 = 进程阻塞，之前的坑）。
    log_line( std::string( "[assert] " ) + aFile.ToUTF8().data() + ":" + std::to_string( aLine )
              + " in " + aFunc.ToUTF8().data() + " | " + aMsg.ToUTF8().data() );
}

std::string random_token()
{
    std::random_device rd;
    std::string        out;
    out.reserve( 32 );
    for( int i = 0; i < 32; ++i )
        out.push_back( "0123456789abcdef"[ rd() % 16 ] );
    return out;
}

std::string arg_value( int argc, char** argv, const std::string& aName,
                       const std::string& aDefault = {} )
{
    const std::string want = "--" + aName;
    for( int i = 1; i + 1 < argc; ++i )
        if( argv[ i ] == want )
            return argv[ i + 1 ];
    return aDefault;
}

} // namespace

int main( int argc, char** argv )
{
    SetUnhandledExceptionFilter( sehHandler );
    wxSetAssertHandler( serviceAssertHandler );

    // 运行态工作区跟随：/document 可切换当前文件；启动 --file 仅为初始值（可为空=空文档）。
    std::string file = arg_value( argc, argv, "file" );
    const std::string portArg = arg_value( argc, argv, "port", "0" );
    const std::string logPath = arg_value( argc, argv, "log" );
    const std::string libDir = arg_value( argc, argv, "lib-dir" );
    const std::string userLibDir = arg_value( argc, argv, "user-lib-dir" );
    g_logPath = logPath;
    const int wantPort = std::atoi( portArg.c_str() );

    cicada::kicad_geometry::SchInteractionEngine engine;
    cicada::editor::SchematicModel                base;
    std::string                                   lastHash;

    bool loaded = false;
    if( !file.empty() )
    {
        wxString err;
        if( cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                engine, wxString::FromUTF8( file.c_str() ), &err, &base ) )
        {
            loaded = true;
            lastHash = cicada::editor::service::file_sha256( file );
            log_line( "[engine] loaded " + file + " hash=" + lastHash );
        }
        else
        {
            log_line( "[engine] load FAILED: " + std::string( err.ToUTF8().data() )
                      + " (continuing with empty document)" );
        }
    }
    else
    {
        log_line( "[engine] no --file; empty document" );
    }
    (void) loaded;

    // M1b：内置单权威先注册——按键模型 R:R/C:C/LED:LED 由内置模板接管，
    // 精选库重名键跳过（RegisterLibTemplate 拒重）；顺序颠倒会产生重复键 +
    // name-only 回退歧义（launcher 空文档启动路径的关键）。
    engine.EnsureBuiltinLibrary();

    // M1b：精选库/用户库装载（*.kicad_sym → 引擎模板注册；服务侧留引脚数据供 /lib/get）
    std::vector<cicada::editor::service::LibItem> libItems;
    // 注册一个 .kicad_sym 解析产物（key = category:name；重名键跳过；libItems 同步）。
    // /lib/synthesize 同用（文件写盘后经此装载）。
    std::set<std::string> userLibKeys;   // 用户库已注册键（synthesize 覆盖守卫：只读库键禁覆盖）
    auto registerLibFile = [&]( const std::filesystem::path& aFile, const std::string& aCategory,
                                bool aUserLib )
    {
        std::vector<cicada::editor::service::LibItem> items;
        std::string err;
        if( !cicada::editor::service::LoadKicadSymFile( aFile.string(), items, &err ) )
        {
            log_line( "[engine] lib skip " + aFile.string() + ": " + err );
            return;
        }
        for( auto& item : items )
        {
            const std::string name = item.symbol->GetName().ToUTF8().data();
            const std::string key = aCategory + ":" + name;
            const auto vIt = item.properties.find( "Value" );
            if( vIt != item.properties.end() )
                item.symbol->GetValueField().SetText( vIt->second );
            item.libId = wxString::FromUTF8( key.c_str() );
            if( engine.RegisterLibTemplate( std::move( item.symbol ), item.refPrefix,
                                            item.libId ) )
            {
                libItems.push_back( std::move( item ) );
                if( aUserLib )
                    userLibKeys.insert( key );
                log_line( "[engine] lib symbol + " + key );
            }
            else
            {
                log_line( "[engine] lib skip (duplicate) " + key );
            }
        }
    };
    // 两级目录（精选/用户库键模型：<dir>/<category>/<name>.kicad_sym，键=category:name）
    // + 根 flat 兼容（测试库：键=文件名:名称）。
    auto loadLibDir = [&]( const std::string& aDir, const char* aTag, bool aUserLib )
    {
        if( aDir.empty() || !std::filesystem::exists( std::filesystem::path( aDir ) ) )
            return;
        for( const auto& entry : std::filesystem::directory_iterator( aDir ) )
        {
            if( entry.is_regular_file() && entry.path().extension() == ".kicad_sym" )
            {
                registerLibFile( entry.path(), entry.path().stem().string(), aUserLib );
            }
            else if( entry.is_directory() )
            {
                const std::string category = entry.path().filename().string();
                for( const auto& f : std::filesystem::directory_iterator( entry.path() ) )
                    if( f.is_regular_file() && f.path().extension() == ".kicad_sym" )
                        registerLibFile( f.path(), category, aUserLib );
            }
        }
        log_line( std::string( "[engine] " ) + aTag + ": " + aDir );
    };
    loadLibDir( libDir, "lib-dir", false );
    loadLibDir( userLibDir, "user-lib-dir", true );

    const std::string token = random_token();
    std::atomic<bool> serverStop{ false };
    std::function<void()> stopNow;   // RunHttpServer 填充："关闭 acceptor"钩子

    auto handler = [&]( const std::string& aMethod, const std::string& aPath,
                        const std::string& aBody, long& aStatus ) -> std::string
    {
        log_line( "[engine] req " + aMethod + " " + aPath );   // M1c 调试：请求台账（收尾移除）
        const auto ensureLoaded = [&]() -> std::string
        {
            if( file.empty() )
                return {};
            const std::string cur = cicada::editor::service::file_sha256( file );
            if( cur != lastHash )
            {
                wxString err;
                base = cicada::editor::SchematicModel{};
                if( !cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                        engine, wxString::FromUTF8( file.c_str() ), &err, &base ) )
                    return err.ToUTF8().data();
                lastHash = cur;
                log_line( "[engine] lazy reload (external/AI write): " + cur );
            }
            return {};
        };

        // ── 运行态工作区跟随 ─────────────────────────────────────────────────────
        // POST /document {"file":"<abs .cicada_sch>"}：切换/装载当前文件（空串=空文档）。
        // 解析失败 → 500 parse_error 且状态不变（保持旧文件与旧内容）。
        if( aPath == "/document" && aMethod == "POST" )
        {
            const std::string needle = "\"file\"";
            const auto pos = aBody.find( needle );
            const auto colon = pos == std::string::npos ? std::string::npos : aBody.find( ':', pos + needle.size() );
            std::string newFile;
            if( colon != std::string::npos )
            {
                auto it = aBody.begin() + colon + 1;
                while( it != aBody.end() && ( *it == ' ' || *it == '\t' ) ) ++it;
                if( it != aBody.end() && *it == '"' )
                {
                    ++it;
                    while( it != aBody.end() && *it != '"' )
                    {
                        if( *it == '\\' && it + 1 != aBody.end() ) ++it;
                        newFile.push_back( *it );
                        ++it;
                    }
                }
            }
            std::string loadErr;
            if( !newFile.empty() )
            {
                // 首次使用的工作区还没有真相文件（docs/07 未定项 3）：创建父目录 + 写空文档，
                // 让"打开画布即可编辑"成立。空文档由引擎自家 writer 产出（格式唯一权威）。
                const std::filesystem::path docPath(
                    wxString::FromUTF8( newFile.c_str() ).ToStdWstring() );
                std::error_code fsEc;
                if( !std::filesystem::exists( docPath, fsEc ) )
                {
                    std::filesystem::create_directories( docPath.parent_path(), fsEc );
                    engine.ClearDocument();
                    cicada::editor::SchematicModel emptyBase;
                    emptyBase.version = kTruthVersion;
                    wxString createErr;
                    if( !cicada::editor::cicada_doc::SaveCicadaSchIntoFile(
                            engine, emptyBase, wxString::FromUTF8( newFile.c_str() ), &createErr ) )
                    {
                        aStatus = 500;
                        return "{\"error\":{\"code\":\"internal\",\"message\":\"cannot create "
                               + cicada::editor::service::json_escape( newFile ) + ": "
                               + cicada::editor::service::json_escape(
                                     std::string( createErr.ToUTF8().data() ) ) + "\"}}";
                    }
                    log_line( "[engine] document created (empty): " + newFile );
                }
                wxString err;
                cicada::editor::SchematicModel newBase;
                if( !cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                        engine, wxString::FromUTF8( newFile.c_str() ), &err, &newBase ) )
                {
                    aStatus = 500;
                    return "{\"error\":{\"code\":\"parse_error\",\"message\":\""
                           + cicada::editor::service::json_escape( err.ToUTF8().data() ) + "\"}}";
                }
                base = std::move( newBase );
                lastHash = cicada::editor::service::file_sha256( newFile );
                log_line( "[engine] document -> " + newFile + " hash=" + lastHash );
            }
            else
            {
                // 空串 = 空文档：必须清掉内存内容，否则残留文档会被后续 /ops 写到不存在的路径。
                engine.ClearDocument();
                base = cicada::editor::SchematicModel{};
                lastHash.clear();
                log_line( "[engine] document -> (empty)" );
            }
            file = newFile;
            const std::string scene = cicada::editor::service::SceneJson( engine, file );
            return "{\"ok\":true,\"scene\":" + scene + "}";
        }
        if( aPath == "/scene" && aMethod == "GET" )
        {
            if( const std::string reloadErr = ensureLoaded(); !reloadErr.empty() )
            {
                aStatus = 500;
                return "{\"error\":{\"code\":\"parse_error\",\"message\":\"" + reloadErr + "\"}}";
            }
            return cicada::editor::service::SceneJson( engine, file );
        }
        if( aPath == "/ops" && aMethod == "POST" )
        {
            if( const std::string reloadErr = ensureLoaded(); !reloadErr.empty() )
            {
                aStatus = 500;
                return "{\"error\":{\"code\":\"parse_error\",\"message\":\"" + reloadErr + "\"}}";
            }
            // 无文档（宿主尚未 POST /document）= 没有真相文件可写。显式拒绝，绝不"改内存后
            // 写盘失败"——那会给出误导性的 saveback 报错（2026-09-08 画布空白缺陷根因之一）。
            if( file.empty() )
            {
                aStatus = 409;
                return "{\"error\":{\"code\":\"conflict\",\"message\":\"no document loaded: "
                       "workspace schematic is not synced (POST /document first)\"}}";
            }
            const std::string cur = cicada::editor::service::file_sha256( file );
            const std::string out = cicada::editor::service::ApplyOps(
                engine, base, file, aBody, cur );
            if( out.find( "\"ok\":true" ) != std::string::npos )
                lastHash = cicada::editor::service::file_sha256( file );
            if( out.find( "\"conflict\"" ) != std::string::npos )
                aStatus = 409;
            return out;
        }
        if( aPath == "/hit" && aMethod == "POST" )
        {
            if( const std::string reloadErr = ensureLoaded(); !reloadErr.empty() )
            {
                aStatus = 500;
                return "{\"error\":{\"code\":\"parse_error\",\"message\":\"" + reloadErr + "\"}}";
            }
            // 极简字段提取（值均整数；字段名固定）
            auto int_field = [&]( const std::string& aKey, long long aDef ) -> long long
            {
                const std::string needle = "\"" + aKey + "\"";
                const auto pos = aBody.find( needle );
                if( pos == std::string::npos )
                    return aDef;
                const auto colon = aBody.find( ':', pos + needle.size() );
                if( colon == std::string::npos )
                    return aDef;
                return std::atoll( aBody.c_str() + colon + 1 );
            };
            const VECTOR2I pt( static_cast<int>( int_field( "x", 0 ) ),
                               static_cast<int>( int_field( "y", 0 ) ) );
            const int accIU = static_cast<int>( int_field( "tolMils", 5000 ) );

            SCH_SYMBOL*  pinOwner = nullptr;
            SCH_ITEM*    hit = engine.HitTest( pt, accIU, &pinOwner );
            std::string  kind = "blank";
            std::ostringstream extra;
            if( pinOwner )
            {
                kind = "pin";
                const std::string refdes = pinOwner->GetRef( &engine.sheetPath(), false )
                                               .ToUTF8().data();
                // pin 号与物理端点
                for( SCH_PIN* lp : pinOwner->GetLibPins() )
                {
                    if( lp->GetPosition() == pt || pinOwner->GetPinPhysicalPosition( lp ) == pt )
                    {
                        extra << ",\"number\":\"" << lp->GetNumber().ToUTF8().data()
                              << "\",\"refdes\":\"" << refdes << "\"";
                        break;
                    }
                }
                if( extra.str().empty() )
                    extra << ",\"refdes\":\"" << refdes << "\"";
            }
            else if( hit )
            {
                switch( hit->Type() )
                {
                case SCH_SYMBOL_T:
                    kind = "component";
                    extra << ",\"refdes\":\""
                          << static_cast<SCH_SYMBOL*>( hit )->GetRef( &engine.sheetPath(),
                                                                      false )
                                 .ToUTF8().data()
                          << "\"";
                    break;
                case SCH_LINE_T:
                {
                    kind = "wire";
                    // 线可选中/可删除/可注入：命中返回 uuid（/ops delete 的稳定引用）。
                    extra << ",\"uuid\":\""
                          << static_cast<SCH_LINE*>( hit )->m_Uuid.AsString().ToUTF8().data()
                          << "\"";
                    break;
                }
                case SCH_JUNCTION_T:
                    kind = "junction";
                    break;
                default:
                    kind = "item";
                    break;
                }
            }
            return "{\"kind\":\"" + kind + "\"" + extra.str() + "}";
        }
        // ── M1c 架构修正：画线预览几何回归引擎（折角/吸附/terminal 判定唯一权威）──
        // 输入 {anchor:[x,y], cursor:[x,y], prevDir?:[x,y], posture:bool, snapRad:int}
        // 响应 {"ok":true,"mid":[x,y],"end":[x,y],"terminal":bool}
        if( aPath == "/wire/preview" && aMethod == "POST" )
        {
            if( const std::string reloadErr = ensureLoaded(); !reloadErr.empty() )
            {
                aStatus = 500;
                return "{\"error\":{\"code\":\"parse_error\",\"message\":\"" + reloadErr + "\"}}";
            }
            // 极简字段提取（整型；[x,y] 对）
            auto int_field = [&]( const std::string& aKey, long long aDef ) -> long long
            {
                const std::string needle = "\"" + aKey + "\"";
                const auto pos = aBody.find( needle );
                if( pos == std::string::npos )
                    return aDef;
                const auto colon = aBody.find( ':', pos + needle.size() );
                if( colon == std::string::npos )
                    return aDef;
                return std::atoll( aBody.c_str() + colon + 1 );
            };
            auto pair_field = [&]( const std::string& aKey, int* aX, int* aY ) -> bool
            {
                const std::string needle = "\"" + aKey + "\"";
                const auto pos = aBody.find( needle );
                if( pos == std::string::npos )
                    return false;
                const auto colon = aBody.find( ':', pos + needle.size() );
                const auto lb = aBody.find( '[', colon + 1 );
                if( lb == std::string::npos )
                    return false;
                const auto rb = aBody.find( ']', lb );
                if( rb == std::string::npos )
                    return false;
                const std::string inner = aBody.substr( lb + 1, rb - lb - 1 );
                const auto comma = inner.find( ',' );
                if( comma == std::string::npos )
                    return false;
                *aX = std::atoi( inner.c_str() );
                *aY = std::atoi( inner.c_str() + comma + 1 );
                return true;
            };
            int ax = 0, ay = 0, cx = 0, cy = 0, px = 0, py = 0;
            if( !pair_field( "anchor", &ax, &ay ) || !pair_field( "cursor", &cx, &cy ) )
            {
                aStatus = 400;
                return "{\"error\":{\"code\":\"bad_request\",\"message\":\"anchor and cursor required\"}}";
            }
            cicada::editor::service::WirePreviewIn in;
            in.anchor = { ax, ay };
            in.cursor = { cx, cy };
            if( pair_field( "prevDir", &px, &py ) )
                in.prevDir = std::make_pair( px, py );
            in.posture = aBody.find( "\"posture\":true" ) != std::string::npos;
            in.snapRadIU = static_cast<int>( int_field( "snapRad", 12000 ) );
            const auto out = cicada::editor::service::SnapWireInput( engine, in );
            std::ostringstream o;
            o << "{\"ok\":true,\"mid\":[" << out.mid.first << "," << out.mid.second
              << "],\"end\":[" << out.end.first << "," << out.end.second
              << "],\"terminal\":" << ( out.terminal ? "true" : "false" ) << "}";
            return o.str();
        }
        if( aPath == "/lib/list" && aMethod == "GET" )
        {
            engine.EnsureBuiltinLibrary();
            std::vector<wxString> names;
            engine.ListLibrarySymbols( names );
            std::ostringstream o;
            o << "{\"symbols\":[";
            for( size_t i = 0; i < names.size(); ++i )
            {
                if( i )
                    o << ",";
                const wxString key = names[ i ];
                const wxString cat = key.BeforeFirst( ':' );
                const wxString nm  = key.AfterFirst( ':' );
                o << "{\"libId\":\"" << cicada::editor::service::json_escape( key.ToUTF8().data() )
                  << "\",\"name\":\"" << cicada::editor::service::json_escape( nm.ToUTF8().data() )
                  << "\",\"category\":\"" << cicada::editor::service::json_escape( cat.ToUTF8().data() )
                  << "\",\"pins\":" << engine.LibSymbolPinCount( key ) << "}";
            }
            o << "]}";
            return o.str();
        }
        if( aPath == "/lib/get" && aMethod == "POST" )
        {
            // 键寻址：字面 libId 命中；未命中 → 尾段（name-only）唯一回退（旧引用归一）。
            const std::string needle = "\"libId\"";
            const auto pos = aBody.find( needle );
            std::string libId;
            if( pos != std::string::npos )
            {
                const auto colon = aBody.find( ':', pos + needle.size() );
                const auto q1 = aBody.find( '"', colon );
                const auto q2 = aBody.find( '"', q1 + 1 );
                if( q1 != std::string::npos && q2 != std::string::npos )
                    libId = aBody.substr( q1 + 1, q2 - q1 - 1 );
            }
            if( libId.empty() )
            {
                aStatus = 400;
                return "{\"error\":{\"code\":\"bad_request\",\"message\":\"libId required\"}}";
            }
            // 键寻址（单一权威 CanonicalLibKey）：字面键命中 → 尾段（name-only）唯一
            // 回退。内置件键化（R:R/C:C/LED:LED）也必须能被 name-only 命中，
            // 否则预热/工具按名字取几何会漏掉内置件（M1e-1 实测）。
            std::vector<SCH_PIN*> pinList;
            std::map<wxString, wxString> props;
            engine.EnsureBuiltinLibrary();
            const auto tailOf = []( const std::string& k ) -> std::string
            { return k.find( ':' ) == std::string::npos ? k : k.substr( k.find( ':' ) + 1 ); };
            const wxString resolved =
                engine.CanonicalLibKey( wxString::FromUTF8( libId.c_str() ),
                                        wxString::FromUTF8( tailOf( libId ).c_str() ) );
            const std::string resolvedKey = resolved.ToUTF8().data();
            // 属性来自已注册库条目（用户/精选）；内置模板无 datasheet 知识，fields 为空。
            for( const auto& item : libItems )
                if( !item.libId.empty() && item.libId.ToUTF8().data() == resolvedKey )
                {
                    props = item.properties;
                    break;
                }
            if( const LIB_SYMBOL* t = engine.FindBuiltinLibSymbol( resolved ) )
                pinList = t->GetPins();
            // docs/09 §5：未命中必须显式 not_found（runtime 自动查缺以 404 为 miss 判定）
            if( pinList.empty() )
            {
                aStatus = 404;
                return "{\"error\":{\"code\":\"not_found\",\"message\":\""
                       + cicada::editor::service::json_escape(
                           "symbol " + libId + " is not in the loaded library" )
                       + "\"}}";
            }
            std::ostringstream o;
            o << "{\"name\":\"" << cicada::editor::service::json_escape( tailOf( resolvedKey ) )
              << "\",\"libId\":\"" << cicada::editor::service::json_escape( resolvedKey )
              << "\",\"category\":\""
              << cicada::editor::service::json_escape(
                     resolvedKey.find( ':' ) == std::string::npos
                         ? std::string()
                         : resolvedKey.substr( 0, resolvedKey.find( ':' ) ) )
              << "\",\"fields\":{";
            bool firstField = true;
            for( const auto& [k, v] : props )
            {
                if( !firstField )
                    o << ",";
                firstField = false;
                o << "\"" << cicada::editor::service::json_escape( k.ToUTF8().data() )
                  << "\":\"" << cicada::editor::service::json_escape( v.ToUTF8().data() ) << "\"";
            }
            o << "},\"pins\":[";
            for( size_t i = 0; i < pinList.size(); ++i )
            {
                const SCH_PIN* p = pinList[ i ];
                if( i )
                    o << ",";
                const VECTOR2I pos = p->GetPosition();
                o << "{\"number\":\"" << cicada::editor::service::json_escape( p->GetNumber().ToUTF8().data() )
                  << "\",\"name\":\"" << cicada::editor::service::json_escape( p->GetName().ToUTF8().data() )
                  << "\",\"x\":" << pos.x << ",\"y\":" << pos.y
                  << ",\"angle\":" << ( static_cast<int>( p->GetOrientation() ) * 90 ) << "}";
            }
            o << "]}";
            return o.str();
        }
        // ── M1e-1：形状块 → 用户库符号（几何确定性，docs/02 附录 A）─────────────
        if( aPath == "/lib/synthesize" && aMethod == "POST" )
        {
            auto errMsg = []( const std::string& aCode, const std::string& aMsg ) -> std::string
            {
                return "{\"error\":{\"code\":\"" + aCode + "\",\"message\":\""
                       + cicada::editor::service::json_escape( aMsg ) + "\"}}";
            };
            // 极简 JSON 提取（形状块字段固定；缺口/格式错 → 400 消息含字段名）
            // 值的两种形态都要认：带引号的字符串，以及裸 token（数字/true/null）。
            // 旧实现只找"冒号后第一个引号"，于是 `"number": 10` 会一路读到**下一个
            // 键名**（`"name"`）→ 两个引脚都成了 number="name" → 报
            // `pin number duplicated: name`（2026-09-13 实测：数据手册子代理把
            // number 写成 JSON 数字，producer 因此完全看不懂错误）。
            auto strField = [&]( const std::string& aJson, const std::string& aKey ) -> std::string
            {
                const std::string needle = "\"" + aKey + "\"";
                const auto pos = aJson.find( needle );
                if( pos == std::string::npos )
                    return {};
                const auto colon = aJson.find( ':', pos + needle.size() );
                if( colon == std::string::npos )
                    return {};
                std::size_t i = colon + 1;
                while( i < aJson.size()
                       && std::isspace( static_cast<unsigned char>( aJson[ i ] ) ) )
                    ++i;
                if( i >= aJson.size() )
                    return {};
                if( aJson[ i ] == '"' )
                {
                    const auto q2 = aJson.find( '"', i + 1 );
                    if( q2 == std::string::npos )
                        return {};
                    return aJson.substr( i + 1, q2 - i - 1 );
                }
                std::size_t end = i;
                while( end < aJson.size() && aJson[ end ] != ',' && aJson[ end ] != '}'
                       && aJson[ end ] != ']'
                       && !std::isspace( static_cast<unsigned char>( aJson[ end ] ) ) )
                    ++end;
                return aJson.substr( i, end - i );
            };
            auto intField = [&]( const std::string& aJson, const std::string& aKey,
                                 long long aDef ) -> long long
            {
                const std::string needle = "\"" + aKey + "\"";
                const auto pos = aJson.find( needle );
                if( pos == std::string::npos )
                    return aDef;
                const auto colon = aJson.find( ':', pos + needle.size() );
                if( colon == std::string::npos )
                    return aDef;
                return std::atoll( aJson.c_str() + colon + 1 );
            };

            cicada::editor::service::ShapeBlock block;
            block.name       = strField( aBody, "name" );
            block.refPrefix  = strField( aBody, "refPrefix" );
            block.description = strField( aBody, "description" );

            const std::string needle = "\"pins\"";
            const auto pos = aBody.find( needle );
            const auto lb = pos == std::string::npos ? std::string::npos : aBody.find( '[', pos );
            if( lb == std::string::npos )
            {
                aStatus = 400;
                return errMsg( "bad_request", "pins required" );
            }
            // pins 数组：逐个 {...} 切片（格式固定：逗号分隔的对象字面量）
            for( std::size_t i = lb + 1; i < aBody.size(); ++i )
            {
                if( aBody[ i ] != '{' )
                    continue;
                const auto rb = aBody.find( '}', i );
                if( rb == std::string::npos )
                    break;
                const std::string slice = aBody.substr( i, rb - i + 1 );
                cicada::editor::service::ShapeBlockPin pin;
                pin.number     = strField( slice, "number" );
                pin.name       = strField( slice, "name" );
                pin.electrical = strField( slice, "electrical" );
                pin.side       = strField( slice, "side" );
                if( pin.electrical.empty() )
                    pin.electrical = "passive";   // 容忍缺省（AI 语法包络）
                block.pins.push_back( pin );
                i = rb;
            }
            const auto bodyPos = aBody.find( "\"body\"" );
            if( bodyPos != std::string::npos )
            {
                block.bodyGiven = true;
                block.bodyW = static_cast<int>( intField( aBody.substr( bodyPos ), "w", 0 ) );
                block.bodyH = static_cast<int>( intField( aBody.substr( bodyPos ), "h", 0 ) );
            }

            const std::string fieldErr = cicada::editor::service::ValidateShapeBlock( block );
            if( !fieldErr.empty() )
            {
                aStatus = 400;
                return errMsg( "bad_request", fieldErr );
            }
            if( userLibDir.empty() )
            {
                aStatus = 400;
                return errMsg( "bad_request", "user lib dir not configured (--user-lib-dir)" );
            }

            const std::string key = "IC:" + block.name;
            // 覆盖守卫：同键已存在 → 仅当来自用户库（只读库键禁覆盖）
            std::vector<std::string> warnings;
            const bool inEngine = engine.FindBuiltinLibSymbol(
                                      wxString::FromUTF8( key.c_str() ) )
                                  != nullptr;
            const bool userHolds = userLibKeys.count( key ) > 0;
            if( inEngine && !userHolds )
            {
                aStatus = 409;
                return errMsg( "conflict", key + " exists in read-only library" );
            }
            if( inEngine )
                warnings.push_back( "replaced existing " + key );

            std::string symText;
            std::string synthErr;
            if( !cicada::editor::service::SynthesizeSymbolText( block, symText, &synthErr ) )
            {
                aStatus = 400;
                return errMsg( "bad_request", synthErr );
            }

            // 写盘（用户库两级：<user-lib-dir>/IC/<name>.kicad_sym）→ 装载（注册 + libItems 同步）
            const std::filesystem::path outDir = std::filesystem::path( userLibDir ) / "IC";
            const std::filesystem::path outFile = outDir / ( block.name + ".kicad_sym" );
            try
            {
                std::filesystem::create_directories( outDir );
                std::ofstream output( outFile, std::ios::binary | std::ios::trunc );
                if( !output )
                    throw std::runtime_error( "unable to open " + outFile.string() );
                output.write( symText.data(), static_cast<std::streamsize>( symText.size() ) );
                if( !output )
                    throw std::runtime_error( "unable to write " + outFile.string() );
            }
            catch( const std::exception& ex )
            {
                aStatus = 500;
                return errMsg( "internal", ex.what() );
            }
            // 覆盖：先注销旧模板 + 清旧条目（registerLibFile 会重推新条目）
            if( inEngine )
            {
                engine.UnregisterLibTemplate( wxString::FromUTF8( key.c_str() ) );
                for( auto it = libItems.begin(); it != libItems.end(); )
                {
                    if( !it->libId.empty() && it->libId.ToUTF8().data() == key )
                        it = libItems.erase( it );
                    else
                        ++it;
                }
                userLibKeys.erase( key );
            }
            registerLibFile( outFile, "IC", true );

            std::ostringstream o;
            o << "{\"ok\":true,\"libId\":\""
              << cicada::editor::service::json_escape( key ) << "\",\"category\":\"IC\",\"name\":\""
              << cicada::editor::service::json_escape( block.name ) << "\",\"warnings\":[";
            for( size_t i = 0; i < warnings.size(); ++i )
            {
                if( i )
                    o << ",";
                o << "\"" << cicada::editor::service::json_escape( warnings[ i ] ) << "\"";
            }
            o << "]}";
            return o.str();
        }
        // ── M1 验收：导出标准 KiCad 10 .kicad_sch（不改唯一真相文件）────────────
        if( aPath == "/export" && aMethod == "POST" )
        {
            const std::string needle = "\"path\"";
            const auto pos = aBody.find( needle );
            const auto colon = pos == std::string::npos ? std::string::npos : aBody.find( ':', pos );
            std::string outPath;
            if( colon != std::string::npos )
            {
                const auto q1 = aBody.find( '"', colon );
                const auto q2 = q1 == std::string::npos ? std::string::npos : aBody.find( '"', q1 + 1 );
                if( q1 != std::string::npos && q2 != std::string::npos )
                    outPath = aBody.substr( q1 + 1, q2 - q1 - 1 );
            }
            if( outPath.empty() )
            {
                aStatus = 400;
                return "{\"error\":{\"code\":\"bad_request\",\"message\":\"path required\"}}";
            }
            wxString err;
            if( !cicada::editor::cicada_doc::ExportKicadSchIntoFile(
                    engine, base, wxString::FromUTF8( outPath.c_str() ), &err ) )
            {
                aStatus = 500;
                return "{\"error\":{\"code\":\"internal\",\"message\":\""
                       + cicada::editor::service::json_escape(
                           std::string( err.ToUTF8().data() ) )
                       + "\"}}";
            }
            return "{\"ok\":true,\"path\":\""
                   + cicada::editor::service::json_escape( outPath ) + "\"}";
        }
        if( aPath == "/shutdown" && aMethod == "POST" )
        {
            log_line( "[engine] shutdown requested" );
            aStatus = 200;
            // 应答后停：置停止位 + 关闭 acceptor（使阻塞的 accept 抛出退出），
            // 延迟 50ms 确保响应已刷出。
            std::thread( [&] {
                std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
                serverStop.store( true );
                stopNow();
            } ).detach();
            return "{\"ok\":true}";
        }
        aStatus = 404;
        return "{\"error\":{\"code\":\"not_found\",\"message\":\"" + aPath + "\"}}";
    };

    log_line( "[engine] starting http on 127.0.0.1:" + std::to_string( wantPort ) );
    const int rc = cicada::editor::service::RunHttpServer(
        token, wantPort,
        [&]( int aActualPort )
        {
            // 自宣告：launcher 解析此行使知 EP/ET（stdout 单独一行；同时落 --log 便于脚本/排障）
            printf( "cicada-engine: 127.0.0.1:%d %s\n", aActualPort, token.c_str() );
            fflush( stdout );
            char buf[ 128 ];
            snprintf( buf, sizeof( buf ), "cicada-engine: 127.0.0.1:%d %s", aActualPort,
                      token.c_str() );
            log_line( buf );
        },
        handler, serverStop, stopNow );
    return rc;
}
