// scene_json_test.cpp — M1a headless：装载 fixture → SceneJson → 断言字段
// usage: cicada-engine-scene-test <file.cicada_sch>
#include <kicad_bridge/cicada_document_bridge.h>
#include <kicad_bridge/sch_interaction_engine.h>
#include <service/scene_json.h>

#include <sch_item.h>

#include <stdio.h>
#include <string>
#include <windows.h>
#include <dbghelp.h>
#pragma comment( lib, "dbghelp.lib" )
#include <wx/debug.h>
#include <wx/string.h>

static int failures = 0;

#define CHECK(cond, msg)                                                              \
    do {                                                                              \
        if( !( cond ) )                                                               \
        {                                                                             \
            fprintf( stderr, "FAIL: %s (line %d)\n", msg, __LINE__ );                 \
            ++failures;                                                               \
        }                                                                             \
    } while( 0 )

// 崩溃日志：写到 exe 同目录 crash-scene-test.txt（无硬编码路径；进程自启自清）
static void crash_log( const char* aText )
{
    wchar_t exePath[ MAX_PATH ] = { 0 };
    GetModuleFileNameW( nullptr, exePath, MAX_PATH );
    std::wstring logPath( exePath );
    const auto dot = logPath.find_last_of( L".\\" );
    if( dot != std::wstring::npos && logPath[ dot ] == L'.' )
        logPath = logPath.substr( 0, dot );
    logPath += L"-crash.txt";
    FILE* f = nullptr;
    _wfopen_s( &f, logPath.c_str(), L"a" );
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
    crash_log( buf );

    HANDLE proc = GetCurrentProcess();
    SymInitialize( proc, nullptr, TRUE );
    SymSetOptions( SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME );
    void* frames[ 32 ];
    int   n = CaptureStackBackTrace( 0, 32, frames, nullptr );
    for( int i = 0; i < n; ++i )
    {
        DWORD64     dish = 0;
        SYMBOL_INFO* si = (SYMBOL_INFO*) calloc( 1, sizeof( SYMBOL_INFO ) + 512 );
        si->SizeOfStruct = sizeof( SYMBOL_INFO );
        si->MaxNameLen = 512;
        if( SymFromAddr( proc, (DWORD64) frames[ i ], &dish, si ) )
        {
            char line[ 512 ];
            snprintf( line, sizeof( line ), "  #%02d %s+0x%llx", i, si->Name,
                      (unsigned long long) dish );
            crash_log( line );
        }
        free( si );
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

static void testAssertHandler( const wxString& aFile, int aLine, const wxString& aFunc,
                               const wxString& aCond, const wxString& aMsg )
{
    fprintf( stderr, "[ASSERT] %s:%d in %s | cond=%s | msg=%s\n",
             aFile.ToUTF8().data(), aLine, aFunc.ToUTF8().data(),
             aCond.ToUTF8().data(), aMsg.ToUTF8().data() );
    fflush( stderr );
}

int main( int argc, char** argv )
{
    SetUnhandledExceptionFilter( sehHandler );
    wxSetAssertHandler( testAssertHandler );

    if( argc < 2 )
    {
        fprintf( stderr, "usage: scene-test <file.cicada_sch>\n" );
        return 2;
    }

    cicada::kicad_geometry::SchInteractionEngine eng;
    wxString err;
    if( !cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
            eng, wxString::FromUTF8( argv[ 1 ] ), &err ) )
    {
        fprintf( stderr, "load FAILED: %s\n", err.ToUTF8().data() );
        return 1;
    }

    fprintf( stderr, "[test] loading done; SceneJson...\n" );
    const std::string json = cicada::editor::service::SceneJson( eng, argv[ 1 ] );
    fprintf( stderr, "[test] SceneJson done (%zu bytes)\n", json.size() );

    // 结构完整性：以最小 fixture 为前提（minimal.cicada_sch 有 R1 + 线/结点/label）
    CHECK( json.find( "\"components\":[" ) != std::string::npos, "components array" );
    CHECK( json.find( "\"wires\":[" ) != std::string::npos, "wires array" );
    CHECK( json.find( "\"junctions\":[" ) != std::string::npos, "junctions array" );
    CHECK( json.find( "\"labels\":[" ) != std::string::npos, "labels array" );
    CHECK( json.find( "\"no_connects\":[" ) != std::string::npos, "no_connects array" );
    CHECK( json.find( "\"hash\":\"" ) != std::string::npos, "hash present" );
    CHECK( json.find( "\"version\":\"" ) != std::string::npos, "version present" );

    // 极小健全性：括号配平 + 无 NaN/非法 token
    int braces = 0;
    bool inString = false;
    for( size_t i = 0; i < json.size(); ++i )
    {
        const char c = json[ i ];
        if( inString )
        {
            if( c == '\\' )
                ++i;
            else if( c == '"' )
                inString = false;
            continue;
        }
        if( c == '"' )
            inString = true;
        else if( c == '{' )
            ++braces;
        else if( c == '}' )
            --braces;
    }
    CHECK( !inString, "string well-formed" );
    CHECK( braces == 0, "braces balanced" );
    CHECK( json.find( "NaN" ) == std::string::npos, "no NaN" );

    if( json.find( "\"refdes\":\"R1\"" ) != std::string::npos )
    {
        CHECK( json.find( "\"pins\":[" ) != std::string::npos, "R1 pins array" );
        CHECK( json.find( "\"body\":{\"rect\":{\"w\":" ) != std::string::npos, "R1 body rect" );
    }

    if( failures == 0 )
        fprintf( stderr, "scene_json OK (%zu bytes)\n", json.size() );
    return failures == 0 ? 0 : 1;
}
