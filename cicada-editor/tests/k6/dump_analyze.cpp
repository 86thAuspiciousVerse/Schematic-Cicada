// K6: minidump symbolizer (raw RVA parse + dbghelp against the exe PDB).
// Usage: kicad-dump-tool.exe <dump.dmp> <pdbDir>
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <string>
#include <vector>

#pragma comment( lib, "dbghelp.lib" )

void wideToUtf8( const wchar_t* in, char* out, int cap )
{
    WideCharToMultiByte( CP_UTF8, 0, in, -1, out, cap, nullptr, nullptr );
}

int main()
{
    int ac = 0;
    wchar_t** aw = CommandLineToArgvW( GetCommandLineW(), &ac );
    if( ac < 3 )
    {
        fprintf( stderr, "usage: kicad-dump-tool <dump.dmp> <pdbDir>\n" );
        return 2;
    }

    HANDLE hFile = CreateFileW( aw[ 1 ], GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, nullptr );
    if( hFile == INVALID_HANDLE_VALUE ) { fprintf( stderr, "cannot open dump\n" ); return 3; }
    DWORD sz = GetFileSize( hFile, nullptr );
    std::vector<BYTE> buf( sz );
    DWORD rd = 0;
    ReadFile( hFile, buf.data(), sz, &rd, nullptr );
    CloseHandle( hFile );
    fprintf( stderr, "dump bytes=%u\n", sz );

    const BYTE* p = buf.data();
    if( memcmp( p, "MDMP", 4 ) != 0 ) { fprintf( stderr, "not a minidump\n" ); return 3; }
    unsigned nstreams = *(unsigned*)( p + 8 );
    unsigned dirRva   = *(unsigned*)( p + 12 );
    std::vector<unsigned> stype( nstreams ), srva( nstreams );
    for( unsigned i = 0; i < nstreams; ++i )
    {
        stype[ i ] = *(unsigned*)( p + dirRva + 12 * i );
        srva[ i ]  = *(unsigned*)( p + dirRva + 12 * i + 8 );
    }

    unsigned exTid = 0;
    DWORD code = 0;
    ULONG_PTR addr = 0;
    for( unsigned i = 0; i < nstreams; ++i )
        if( stype[ i ] == 6 )
        {
            const BYTE* e = p + srva[ i ];
            exTid = *(unsigned*)( e + 0 );
            memcpy( &code, e + 8, 4 );
            memcpy( &addr, e + 24, 8 );
            fprintf( stderr, "exception code=0x%08lx thread=%x addr=%p\n", code, exTid,
                     (void*) addr );
        }

    struct Mod { DWORD64 base; DWORD64 size; std::wstring name; };
    std::vector<Mod> mods;
    for( unsigned i = 0; i < nstreams; ++i )
        if( stype[ i ] == 4 )
        {
            const BYTE* m = p + srva[ i ];
            unsigned cnt = *(unsigned*) m;
            for( unsigned k = 0; k < cnt; ++k )
            {
                const BYTE* e = m + 4 + 108 * k;   // MINIDUMP_MODULE = 108 bytes, follows the u32 count
                DWORD64 base = *(DWORD64*)( e + 0 );
                DWORD64 size = *(DWORD64*)( e + 8 );
                unsigned nameRva = *(unsigned*)( e + 20 );
                if( nameRva + 4 > sz ) continue;
                const BYTE* nm = p + nameRva;
                unsigned len = *(unsigned*) nm;
                if( nameRva + 4 + len > sz || len % 2 != 0 ) continue;
                std::wstring n((wchar_t*)( nm + 4 ), len / 2);
                fprintf( stderr, "  module %ls base=%p size=%llx\n", n.c_str(), (void*) base, size );
                mods.push_back( { base, size, n } );
            }
        }

    DWORD64 rip = 0, rsp = 0;
    for( unsigned i = 0; i < nstreams; ++i )
        if( stype[ i ] == 3 )
        {
            const BYTE* m = p + srva[ i ];
            unsigned cnt = *(unsigned*) m;
            for( unsigned k = 0; k < cnt; ++k )
            {
                const BYTE* e = m + 4 + 48 * k;   // MINIDUMP_THREAD = 48 bytes
                unsigned tid = *(unsigned*)( e + 0 );
                unsigned ctxRva = *(unsigned*)( e + 36 );   // ThreadContext.Location.Rva
                unsigned ctxSize = *(unsigned*)( e + 32 );
                if( tid != exTid ) continue;
                fprintf( stderr, "  ctx size=%x rva=%x\n", ctxSize, ctxRva );
                if( ctxRva + ctxSize >= sz ) { fprintf( stderr, "  ctx rva out of dump\n" ); continue; }
                const BYTE* c = p + ctxRva;
                for( unsigned q = 0; q * 8 + 8 <= ctxSize; ++q )
                {
                    DWORD64 v = *(DWORD64*)( c + q * 8 );
                    // pointers into the crashed exe's image = code frames
                    for( auto& mm : mods )
                        if( v >= mm.base && v < mm.base + 0x1212000 )
                            fprintf( stderr, "  ctx[%x]=%llx\n", q * 8, v );
                }
                rip = *(DWORD64*)( c + 0xE8 );
                rsp = *(DWORD64*)( c + 0x88 );
                fprintf( stderr, "  thread=%x rip=%llx rsp=%llx (ctx rva=%x)\n", tid, rip, rsp, ctxRva );
            }
        }

    char pdbDir[ 1024 ] = "";
    wideToUtf8( aw[ 2 ], pdbDir, sizeof( pdbDir ) );
    HANDLE proc = GetCurrentProcess();
    SymSetOptions( SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES );
    if( !SymInitialize( proc, pdbDir, TRUE ) ) fprintf( stderr, "syminit failed\n" );

    // SymLoadModuleEx for the module containing the address (image + pdb sit in pdbDir)
    {
        DWORD64 containing = 0;
        for( auto& m : mods )
            if( addr >= m.base && addr < m.base + m.size )
            {
                containing = m.base;
                char img[ 1024 ] = "";
                char nm8[ 256 ] = "";
                wideToUtf8( m.name.c_str(), nm8, sizeof( nm8 ) );
                snprintf( img, sizeof( img ), "%s\\%s", pdbDir, nm8 );
                DWORD64 h = SymLoadModuleEx( proc, nullptr, img, nullptr, m.base, m.size, nullptr, 0 );
                fprintf( stderr, "  loadimg %s -> %llx\n", img, h );
                break;
            }
    }

    auto sym = [&]( DWORD64 a )
    {
        BYTE sb[ sizeof( SYMBOL_INFO ) + 1024 ];
        SYMBOL_INFO* si = (SYMBOL_INFO*) sb;
        si->SizeOfStruct = sizeof( SYMBOL_INFO );
        si->MaxNameLen = 1024;
        DWORD64 dish = 0;
        if( SymFromAddr( proc, a, &dish, si ) )
        {
            IMAGEHLP_LINE64 line = { sizeof( IMAGEHLP_LINE64 ) };
            DWORD d = 0;
            const char* file = "";
            if( SymGetLineFromAddr64( proc, a, &d, &line ) ) file = line.FileName;
            fprintf( stderr, "  %s+0x%llx (%s:%u)\n", si->Name, dish, file, line.LineNumber );
        }
        else
        {
            for( auto& m : mods )
                if( a >= m.base && a < m.base + m.size )
                {
                    char nb[ 256 ] = "";
                    wideToUtf8( m.name.c_str(), nb, sizeof( nb ) );
                    fprintf( stderr, "  %s+0x%llx (no symbols)\n", nb, a - m.base );
                    return;
                }
            fprintf( stderr, "  <unknown 0x%llx>\n", a );
        }
    };

    // Memory64 walk: find the range holding rsp, read return addresses, resolve them
    if( rsp != 0 )
    {
        for( unsigned i = 0; i < nstreams; ++i )
            if( stype[ i ] == 9 )
            {
                const BYTE* m = p + srva[ i ];
                unsigned long long nRanges = *(unsigned long long*)( m );
                const BYTE* r = m + 16;
                fprintf( stderr, "stack walk: %llu ranges\n", nRanges );
                for( unsigned long long k = 0; k < nRanges; ++k )
                {
                    unsigned long long start = *(unsigned long long*)( r + 16 * k );
                    unsigned long long dsize = *(unsigned long long*)( r + 16 * k + 8 );
                    if( rsp >= start && rsp < start + dsize )
                    {
                        const BYTE* sp = p + srva[ i ] + 16 + 16 * nRanges + (rsp - start);
                        fprintf( stderr, "stack range %llx+%llx; frames:\n", start, dsize );
                        for( int f = 0; f < 20; ++f )
                        {
                            unsigned long long v = *(unsigned long long*)( sp + 8 * f );
                            if( v < 0x10000 ) continue;
                            fprintf( stderr, "  frame %d: ", f );
                            sym( v );
                        }
                        break;
                    }
                }
                break;
            }
    }

    fprintf( stderr, "exception address:\n" );
    sym( addr );
    if( rip != 0 )
    {
        fprintf( stderr, "thread rip:\n" );
        sym( rip );
    }
    LocalFree( aw );
    return 0;
}
