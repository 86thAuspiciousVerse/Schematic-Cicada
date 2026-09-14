// K6 reload crash repro (headless): load a .cicada_sch into a fresh engine.
// Usage: kicad-model-slice3-reload-test <file.cicada_sch>
// SEH handler dumps the faulting stack (Debug PDB → symbols; else addresses).
#include <kicad_bridge/cicada_document_bridge.h>
#include <kicad_bridge/sch_interaction_engine.h>
#include <kicad_bridge/kicad_scene_adapter.h>

#include <sch_item.h>

#include <stdio.h>
#include <windows.h>
#include <dbghelp.h>
#pragma comment( lib, "dbghelp.lib" )
#include <wx/debug.h>

static LONG CALLBACK sehHandler( EXCEPTION_POINTERS* aInfo )
{
    fprintf( stderr, "[CRASH] code=0x%lx at %p\n", aInfo->ExceptionRecord->ExceptionCode,
             aInfo->ExceptionRecord->ExceptionAddress );

    HANDLE   proc = GetCurrentProcess();
    SymInitialize( proc, nullptr, TRUE );
    SymSetOptions( SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME );

    void* frames[ 32 ];
    int   n = CaptureStackBackTrace( 0, 32, frames, nullptr );
    for( int i = 0; i < n; ++i )
    {
        DWORD64    dish = 0;
        SYMBOL_INFO* si = (SYMBOL_INFO*) calloc( 1, sizeof( SYMBOL_INFO ) + 512 );
        si->SizeOfStruct = sizeof( SYMBOL_INFO );
        si->MaxNameLen = 512;
        if( SymFromAddr( proc, (DWORD64) frames[ i ], &dish, si ) )
            fprintf( stderr, "  #%02d  %s+0x%llx\n", i, si->Name, (unsigned long long) dish );
        else
            fprintf( stderr, "  #%02d  0x%llx\n", i, (unsigned long long) frames[ i ] );
        free( si );
    }

    fflush( stderr );
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
        fprintf( stderr, "usage: reload-probe <file.cicada_sch>\n" );
        return 2;
    }

    fprintf( stderr, "[probe] engine ctor\n" );
    cicada::kicad_geometry::SchInteractionEngine eng;
    fprintf( stderr, "[probe] engine ctor ok; loading %s\n", argv[ 1 ] );

    wxString err;
    const bool ok = cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
        eng, wxString::FromUTF8( argv[ 1 ] ), &err );
    if( !ok )
    {
        fprintf( stderr, "load FAILED: %s\n", err.ToUTF8().data() );
        return 1;
    }

    int n = 0;
    for( SCH_ITEM* it : eng.screen()->Items() )
    {
        (void) it;
        ++n;
    }
    fprintf( stderr, "load OK items=%d\n", n );

    // BuildScene: the same scene assembly the wx canvas runs after reload
    fprintf( stderr, "[probe] BuildScene\n" );
    const auto scene = cicada::editor::kicad_adapter::BuildScene( eng, &eng.Selection() );
    fprintf( stderr, "scene OK primitives=%zu\n", scene.primitives().size() );

    // Round-trip: SaveCicadaSchIntoFile → reload into a fresh engine (K6 saveback)
    cicada::editor::SchematicModel base;
    wxString err2;
    if( !cicada::editor::cicada_doc::LoadCicadaSchIntoEngine( eng, wxString::FromUTF8( argv[ 1 ] ), &err2, &base ) )
    {
        fprintf( stderr, "base load FAILED: %s\n", err2.ToUTF8().data() );
        return 1;
    }
    fprintf( stderr, "[probe] base model lib_symbols=%zu symbols=%zu\n", base.lib_symbols.size(), base.symbols.size() );
    const wxString outPath = wxString::FromUTF8(
        ( std::string( argv[ 1 ] ) + ".probe-save" ).c_str() );
    if( !cicada::editor::cicada_doc::SaveCicadaSchIntoFile( eng, base, outPath, &err2 ) )
    {
        fprintf( stderr, "save FAILED: %s\n", err2.ToUTF8().data() );
        return 1;
    }
    fprintf( stderr, "[probe] save OK -> %s\n", outPath.ToUTF8().data() );

    cicada::kicad_geometry::SchInteractionEngine eng2;
    wxString err3;
    if( !cicada::editor::cicada_doc::LoadCicadaSchIntoEngine( eng2, outPath, &err3 ) )
    {
        fprintf( stderr, "RELOAD-of-save FAILED: %s\n", err3.ToUTF8().data() );
        return 1;
    }
    int n2 = 0;
    for( SCH_ITEM* it : eng2.screen()->Items() ) { (void) it; ++n2; }
    fprintf( stderr, "round-trip load OK items=%d (orig=%d)\n", n2, n );

    fprintf( stderr, "load OK (no crash)\n" );
    return 0;
}
