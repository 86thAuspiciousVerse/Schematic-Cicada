// lib_roundtrip_test.cpp — M1b0 回归：lib_symbols 归一容错 / 不静默丢条目 / 内置件单权威
// usage: cicada-engine-libtest <minimal.cicada_sch> <outdir>
#include <kicad_bridge/cicada_document_bridge.h>
#include <kicad_bridge/sch_interaction_engine.h>
#include <core/sexpr_model.h>
#include <service/lib_loader.h>

#include <sch_item.h>
#include <sch_symbol.h>

#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>

#include <wx/debug.h>

static int failures = 0;

#define RUN(name, cond)                                                          \
    do {                                                                         \
        if( !( cond ) )                                                          \
        {                                                                        \
            fprintf( stderr, "FAIL: %s (line %d)\n", name, __LINE__ );           \
            ++failures;                                                          \
        }                                                                        \
    } while( 0 )

static void assertHandler( const wxString& aFile, int aLine, const wxString& aFunc,
                           const wxString& aCond, const wxString& aMsg )
{
    fprintf( stderr, "[ASSERT] %s:%d in %s | msg=%s\n", aFile.ToUTF8().data(), aLine,
             aFunc.ToUTF8().data(), aMsg.ToUTF8().data() );
}

static std::string read_file( const std::string& aPath )
{
    std::ifstream input( aPath, std::ios::binary );
    std::ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

static void write_file( const std::string& aPath, const std::string& aText )
{
    std::ofstream output( aPath, std::ios::binary | std::ios::trunc );
    output << aText;
}

static std::string replace_all( std::string aText, const std::string& aFrom,
                                const std::string& aTo )
{
    size_t pos = 0;
    while( ( pos = aText.find( aFrom, pos ) ) != std::string::npos )
    {
        aText.replace( pos, aFrom.size(), aTo );
        pos += aTo.size();
    }
    return aText;
}

static int count_items( cicada::kicad_geometry::SchInteractionEngine& aEng )
{
    int n = 0;
    for( SCH_ITEM* it : aEng.screen()->Items() )
        (void) it, ++n;
    return n;
}

// 引擎里某 refdes 的 lib_id 格式串（键保真断言用）；缺省返回空串。
static std::string engine_lib_key( cicada::kicad_geometry::SchInteractionEngine& aEng,
                                   const std::string&                          aRefdes )
{
    for( SCH_ITEM* it : aEng.screen()->Items() )
    {
        if( it->Type() != SCH_SYMBOL_T )
            continue;
        auto* sym = static_cast<SCH_SYMBOL*>( it );
        if( sym->GetRef( &aEng.sheetPath(), false ).ToUTF8().data() == aRefdes )
            return std::string( sym->GetLibId().Format().c_str() );
    }
    return {};
}

static int engine_pin_count( cicada::kicad_geometry::SchInteractionEngine& aEng,
                             const std::string&                          aRefdes )
{
    for( SCH_ITEM* it : aEng.screen()->Items() )
    {
        if( it->Type() != SCH_SYMBOL_T )
            continue;
        auto* sym = static_cast<SCH_SYMBOL*>( it );
        if( sym->GetRef( &aEng.sheetPath(), false ).ToUTF8().data() == aRefdes )
            return static_cast<int>( sym->GetLibPins().size() );
    }
    return -1;
}

// 数据手册道文档：U1 = IC:NE555P（引擎无此模板，体在文件自带 lib_symbols 里）+
// R1 = R:R（引擎已注册键）。这个混合正是 2026-09-12 导出 500 的真实形态。
static std::string datasheet_lane_doc()
{
    return "(kicad_sch (version 20250114) (generator \"cicada\") (generator_version \"0.1\")\n"
           "  (uuid 11111111-2222-3333-4444-555555555555)\n"
           "  (lib_symbols\n"
           "    (symbol \"IC:NE555P\"\n"
           "      (pin_names (offset 0.508))\n"
           "      (property \"Reference\" \"U\" (at 0 0 0))\n"
           "      (property \"Value\" \"NE555P\" (at 0 0 0))\n"
           "      (symbol \"NE555P_0_1\"\n"
           "        (rectangle (start -3.81 -3.81) (end 3.81 3.81)))\n"
           "      (symbol \"NE555P_1_1\"\n"
           "        (pin power_in line (at -1.27 8.89 270) (length 2.54) (name \"GND\") (number \"1\"))\n"
           "        (pin input line (at 0 8.89 270) (length 2.54) (name \"TRIG\") (number \"2\"))\n"
           "        (pin output line (at 1.27 8.89 270) (length 2.54) (name \"OUT\") (number \"3\"))))\n"
           "    (symbol \"R:R\"\n"
           "      (pin_numbers (hide yes)) (pin_names (offset 0))\n"
           "      (property \"Reference\" \"R\" (at 0 0 0))\n"
           "      (property \"Value\" \"R\" (at 0 0 0))\n"
           "      (symbol \"R_0_1\" (rectangle (start -1.016 -2.54) (end 1.016 2.54)))\n"
           "      (symbol \"R_1_1\"\n"
           "        (pin passive line (at 0 3.81 270) (length 1.27) (name \"\") (number \"1\"))\n"
           "        (pin passive line (at 0 -3.81 90) (length 1.27) (name \"\") (number \"2\"))))\n"
           "  )\n"
           "  (symbol (lib_id \"IC:NE555P\") (at 25.40 25.40 0) (unit 1)\n"
           "    (uuid bbbbbbbb-1111-0000-0000-000000000001)\n"
           "    (property \"Reference\" \"U1\" (at 25.40 25.40 0))\n"
           "    (property \"Value\" \"NE555P\" (at 25.40 25.40 0))\n"
           "    (property \"Footprint\" \"DIP-8\" (at 25.40 25.40 0))\n"
           "    (property \"Datasheet\" \"NE555P\" (at 25.40 25.40 0))\n"
           "    (pin \"1\" (uuid bbbbbbbb-1111-0000-0000-000000000002))\n"
           "    (pin \"2\" (uuid bbbbbbbb-1111-0000-0000-000000000003))\n"
           "    (pin \"3\" (uuid bbbbbbbb-1111-0000-0000-000000000004))\n"
           "  )\n"
           "  (symbol (lib_id \"R:R\") (at 39.38 25.40 0) (unit 1)\n"
           "    (uuid bbbbbbbb-1111-0000-0000-000000000005)\n"
           "    (property \"Reference\" \"R1\" (at 39.38 25.40 0))\n"
           "    (property \"Value\" \"10k\" (at 39.38 25.40 0))\n"
           "    (property \"Footprint\" \"\" (at 39.38 25.40 0))\n"
           "    (property \"Datasheet\" \"\" (at 39.38 25.40 0))\n"
           "    (pin \"1\" (uuid bbbbbbbb-1111-0000-0000-000000000006))\n"
           "    (pin \"2\" (uuid bbbbbbbb-1111-0000-0000-000000000007))\n"
           "  )\n"
           "  (wire (pts (xy 25.40 25.40) (xy 39.38 25.40)) (uuid aaaaaaaa-1111-0000-0000-000000000001))\n"
           "  (sheet_instances (path \"/\" (page \"1\")))\n"
           ")\n";
}

// M1b 属性五元提取回归：官方 K10 单符号格式 → properties 非空（Value/Reference…）。
static bool check_properties_extraction()
{
    const std::string sample =
        "(kicad_symbol_lib\n  (version 20251024)\n  (generator \"kicad_symbol_editor\")\n"
        "  (symbol \"R_Shunt\"\n    (pin_names (offset 0))\n"
        "    (property \"Reference\" \"R\" (at 0 0 0))\n"
        "    (property \"Value\" \"R_Shunt\" (at 0 0 0))\n"
        "    (property \"Footprint\" \"\" (at 0 0 0))\n"
        "    (property \"Datasheet\" \"\" (at 0 0 0))\n"
        "    (property \"Description\" \"Shunt\" (at 0 0 0))\n"
        "    (symbol \"R_Shunt_0_1\"\n"
        "      (rectangle (start -1.5 1.5) (end 1.5 -1.5))\n"
        "      (pin passive line (at 0 2 270) (length 1) (name \"A\") (number \"1\"))))\n";
    std::vector<cicada::editor::service::LibItem> items;
    std::string err;
    if( !cicada::editor::service::ParseKicadSym( sample, items, &err ) )
        return false;
    if( items.empty() )
        return false;
    return items[ 0 ].properties.count( "Value" ) > 0
           && items[ 0 ].properties[ "Value" ] == "R_Shunt"
           && items[ 0 ].properties.count( "Reference" ) > 0;
}

int main( int argc, char** argv )
{
    wxSetAssertHandler( assertHandler );
    RUN( "props five-field extraction", check_properties_extraction() );
    if( argc < 3 )
    {
        fprintf( stderr, "usage: libtest <minimal.cicada_sch> <outdir>\n" );
        return 2;
    }
    const std::string fixture = argv[ 1 ];
    const std::string outdir  = argv[ 2 ];

    // ── 基线：minimal 正常往返 ────────────────────────────────────────────────
    {
        cicada::kicad_geometry::SchInteractionEngine eng;
        cicada::editor::SchematicModel                base;
        wxString err;
        RUN( "A load", cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                           eng, wxString::FromUTF8( fixture.c_str() ), &err, &base ) );
        const int n1 = count_items( eng );
        const std::string outA = outdir + "/a.cicada_sch";
        RUN( "A save", cicada::editor::cicada_doc::SaveCicadaSchIntoFile(
                           eng, base, wxString::FromUTF8( outA.c_str() ), &err ) );
        const std::string textA = read_file( outA );
        RUN( "A entry R:R", textA.find( "\"R:R\"" ) != std::string::npos );
        RUN( "A entry C:C", textA.find( "\"C:C\"" ) != std::string::npos );

        cicada::kicad_geometry::SchInteractionEngine eng2;
        wxString err2;
        RUN( "A reload", cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                             eng2, wxString::FromUTF8( outA.c_str() ), &err2 ) );
        RUN( "A item count stable", count_items( eng2 ) == n1 );
        fprintf( stderr, "case A ok (items=%d)\n", n1 );
    }

    // ── 路径 B 复现：实例/条目 lib_id 不对称（Device:R 真机名 vs cicada:R）────
    {
        std::string text = read_file( fixture );
        text = replace_all( text, "\"cicada:R\"", "\"Device:R\"" );   // 实例与条目都改
        const std::string fileB = outdir + "/b.cicada_sch";
        write_file( fileB, text );

        cicada::kicad_geometry::SchInteractionEngine eng;
        cicada::editor::SchematicModel                base;
        wxString err;
        RUN( "B load", cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                           eng, wxString::FromUTF8( fileB.c_str() ), &err, &base ) );
        const std::string outB = outdir + "/b-out.cicada_sch";
        RUN( "B save (no silent drop)", cicada::editor::cicada_doc::SaveCicadaSchIntoFile(
                                           eng, base, wxString::FromUTF8( outB.c_str() ),
                                           &err ) );
        const std::string textB = read_file( outB );
        // docs/09 §1：文件层 lib_id = 实际键——归一后条目必须存在（R:R），
        // 且不再保留 Device:R 旧前缀错配。
        RUN( "B normalized entry", textB.find( "\"R:R\"" ) != std::string::npos );
        RUN( "B no device:R entry", textB.find( "\"Device:R\"" ) == std::string::npos );

        cicada::kicad_geometry::SchInteractionEngine eng2;
        wxString err2;
        RUN( "B reload out", cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                                 eng2, wxString::FromUTF8( outB.c_str() ), &err2 ) );
        fprintf( stderr, "case B ok\n" );
    }

    // ── 非内置缺条目：保存必须失败（绝不静默丢）───────────────────────────────
    {
        std::string text = read_file( fixture );
        // 把实例 lib_id 改成库里没有的非内置名（条目仍只有 cicada:R/C）
        text = replace_all( text, "(lib_id \"cicada:R\")", "(lib_id \"cicada:VR1\")" );
        text = replace_all( text, "\"Reference\" \"R1\"", "\"Reference\" \"VR1\"" );
        const std::string fileC = outdir + "/c.cicada_sch";
        write_file( fileC, text );

        cicada::kicad_geometry::SchInteractionEngine eng;
        cicada::editor::SchematicModel                base;
        wxString err;
        RUN( "C load", cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                           eng, wxString::FromUTF8( fileC.c_str() ), &err, &base ) );
        const std::string outC = outdir + "/c-out.cicada_sch";
        const bool saved = cicada::editor::cicada_doc::SaveCicadaSchIntoFile(
            eng, base, wxString::FromUTF8( outC.c_str() ), &err );
        RUN( "C save must fail loudly", !saved );
        RUN( "C error mentions VR1", std::string( err.ToUTF8().data() ).find( "VR1" )
                                        != std::string::npos );
        fprintf( stderr, "case C ok (save blocked: %s)\n", err.ToUTF8().data() );
    }

    // ── D：.kicad_sym 解析 + 引擎注册 + 放置（多引脚符号）──────────────────────
    {
        // 最小官方形态：LED_RGB（4 引脚）+ 矩形体
        const std::string symText =
            "(kicad_symbol_lib (version 20231120) (generator \"kicad_symbol_editor\") "
            "  (symbol \"LED_RGB\" (pin_numbers (hide yes)) (pin_names (offset 1.016)) "
            "    (property \"Reference\" \"D\" (at 0 -2.54 0)) "
            "    (property \"Value\" \"LED_RGB\" (at 0 2.54 0)) "
            "    (symbol \"LED_RGB_0_1\" "
            "      (rectangle (start -2.54 -1.27) (end 2.54 1.27))) "
            "    (symbol \"LED_RGB_1_1\" "
            "      (pin passive line (at -5.08 0 0) (length 2.54) (name \"R\" (effects (font (size 1.27 1.27)))) (number \"1\")) "
            "      (pin passive line (at 5.08 0 180) (length 2.54) (name \"G\" (effects (font (size 1.27 1.27)))) (number \"2\")) "
            "      (pin passive line (at 0 -5.08 90) (length 2.54) (name \"B\" (effects (font (size 1.27 1.27)))) (number \"3\")) "
            "      (pin passive line (at 0 5.08 270) (length 2.54) (name \"A\" (effects (font (size 1.27 1.27)))) (number \"4\")))))";

        std::vector<cicada::editor::service::LibItem> items;
        std::string err;
        RUN( "D parse kicad_sym", cicada::editor::service::ParseKicadSym( symText, items, &err ) );
        RUN( "D one symbol", items.size() == 1 );
        if( items.size() == 1 )
        {
            cicada::kicad_geometry::SchInteractionEngine eng;
            RUN( "D register", eng.RegisterLibTemplate( std::move( items[ 0 ].symbol ),
                                                        items[ 0 ].refPrefix ) );
            RUN( "D create symbol", eng.CreateSymbol( "LED_RGB", VECTOR2I( 10000, 10000 ) )
                                        != nullptr );
        }
        fprintf( stderr, "case D ok\n" );
    }

    // ── E：数据手册道 IC（引擎无模板；体在文件自带 lib_symbols 里）──────────────
    // 回归（2026-09-12 导出 500）：装载端曾把未注册键改写成 cicada:<name>
    // （LIB_ID("cicada", name)）→ 引擎既无该模板、实例键又与文件条目键 IC:<name>
    // 不同族 → /export 报 "has no lib_symbols body"。
    {
        const std::string fileE = outdir + "/e.cicada_sch";
        write_file( fileE, datasheet_lane_doc() );

        cicada::kicad_geometry::SchInteractionEngine eng;
        cicada::editor::SchematicModel                base;
        wxString                                      err;
        RUN( "E load", cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                           eng, wxString::FromUTF8( fileE.c_str() ), &err, &base ) );
        RUN( "E file key preserved", engine_lib_key( eng, "U1" ) == "IC:NE555P" );
        RUN( "E registered key untouched", engine_lib_key( eng, "R1" ) == "R:R" );
        RUN( "E pins from the file body", engine_pin_count( eng, "U1" ) == 3 );

        const std::string outE = outdir + "/e-out.kicad_sch";
        const bool        exportedE = cicada::editor::cicada_doc::ExportKicadSchIntoFile(
            eng, base, wxString::FromUTF8( outE.c_str() ), &err );
        RUN( "E export succeeds", exportedE );
        const std::string textE = read_file( outE );
        RUN( "E export keeps the instance key",
             textE.find( "(lib_id \"IC:NE555P\")" ) != std::string::npos );
        RUN( "E export embeds the file body",
             textE.find( "(symbol \"IC:NE555P\"" ) != std::string::npos
                 && textE.find( "TRIG" ) != std::string::npos );
        RUN( "E export keeps the template symbol",
             textE.find( "(lib_id \"R:R\")" ) != std::string::npos );

        // 写回同一条 collect_engine 路径：.cicada_sch 也必须保键。
        const std::string outE2 = outdir + "/e-out.cicada_sch";
        RUN( "E save keeps the instance key",
             cicada::editor::cicada_doc::SaveCicadaSchIntoFile(
                 eng, base, wxString::FromUTF8( outE2.c_str() ), &err )
                 && read_file( outE2 ).find( "(lib_id \"IC:NE555P\")" ) != std::string::npos );
        fprintf( stderr, "case E ok (export %d bytes)\n", (int) textE.size() );
    }

    // ── F：跨族错配容错（实例 cicada:X vs 条目 IC:X —— 旧模板道文件/被改写的文件）──
    {
        std::string text = datasheet_lane_doc();
        text = replace_all( text, "(lib_id \"IC:NE555P\")", "(lib_id \"cicada:NE555P\")" );
        const std::string fileF = outdir + "/f.cicada_sch";
        write_file( fileF, text );

        cicada::kicad_geometry::SchInteractionEngine eng;
        cicada::editor::SchematicModel                base;
        wxString                                      err;
        RUN( "F load", cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(
                           eng, wxString::FromUTF8( fileF.c_str() ), &err, &base ) );
        RUN( "F key preserved as written", engine_lib_key( eng, "U1" ) == "cicada:NE555P" );
        RUN( "F pins via tail fallback", engine_pin_count( eng, "U1" ) == 3 );

        const std::string outF = outdir + "/f-out.kicad_sch";
        const bool        exportedF = cicada::editor::cicada_doc::ExportKicadSchIntoFile(
            eng, base, wxString::FromUTF8( outF.c_str() ), &err );
        RUN( "F export succeeds (tail body fallback)", exportedF );
        RUN( "F export embeds the entry body",
             read_file( outF ).find( "(symbol \"IC:NE555P\"" ) != std::string::npos );
        fprintf( stderr, "case F ok\n" );
    }

    if( failures == 0 )
        fprintf( stderr, "lib_roundtrip ALL OK\n" );
    return failures == 0 ? 0 : 1;
}
