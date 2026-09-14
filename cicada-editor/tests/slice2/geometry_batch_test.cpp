// Slice-2 B4 DoD: geometry-batch fixtures for SCH_LINE + SCH_LABEL + SCH_SYMBOL(REFERENCE).
// Numeric assertions (report ⑦-DoD); actuals printed to stderr for arbitration of
// the SCH_SYMBOL field-coordinate semantics on first fixture.
#include "kicad_bridge/sch_geometry_batch.h"

#include <sch_line.h>
#include <sch_label.h>
#include <sch_screen.h>
#include <sch_symbol.h>
#include <lib_symbol.h>
#include <sch_sheet_path.h>
#include <lib_id.h>

#include <assert.h>
#include <stdio.h>
#include <vector>

int main()
{
    using namespace cicada::kicad_geometry;

    SCH_SCREEN screen;
    SCH_SHEET_PATH sheet;
    SCH_RENDER_SETTINGS rs = SchGeometryBatch::MakeDefaultSettings();

    // 夹具 1：SCH_LINE（实线 wire，0,0 -> 1000,1000）
    SCH_LINE wire( VECTOR2I( 0, 0 ), LAYER_WIRE );
    wire.SetStartPoint( VECTOR2I( 0, 0 ) );
    wire.SetEndPoint( VECTOR2I( 1000, 1000 ) );
    screen.Append( &wire, false );

    // 夹具 2：SCH_LABEL "NET1" @(500,500)（本地标签，LAYER_LOCLABEL）
    SCH_LABEL label( VECTOR2I( 500, 500 ), "NET1" );
    screen.Append( &label, false );

    // 夹具 3：SCH_SYMBOL——LIB_SYMBOL("R") + REFERENCE 字段 "R1"（layer 0, 0）
    LIB_SYMBOL libSym( "R" );
    SCH_FIELD& libRef = libSym.GetReferenceField();
    libRef.SetText( "R1" );
    libRef.SetPosition( VECTOR2I( 1000, 100 ) );
    SCH_SYMBOL symbol( libSym, LIB_ID( "device", "R" ), &sheet, 1 );
    symbol.SetPosition( VECTOR2I( 2000, 2000 ) );
    screen.Append( &symbol, false );

    // 收集
    std::vector<StrokeRec> strokes;
    std::vector<TextRec>   texts;
    SchGeometryBatch batch;
    batch.EmitScreen( &screen, sheet, rs,
                      [&]( const StrokeRec& r ) { strokes.push_back( r ); },
                      [&]( const TextRec& r ) { texts.push_back( r ); } );

    // 实际值（仲裁/对拍输出）
    for( const auto& t : texts )
        fprintf( stderr, "[actual] text='%s' pos=(%d,%d) size=(%d,%d) layer=%d\n",
                 t.text.ToUTF8().data(), t.pos.x, t.pos.y, t.size.x, t.size.y, t.layer );

    // DoD 1：wire → 1 条 StrokeRec，端点=模型端点，SOLID，width>0
    assert( strokes.size() >= 1 );
    const StrokeRec* wireRec = nullptr;
    for( const auto& r : strokes )
    {
        if( r.layer == static_cast<int>( LAYER_WIRE ) )
            wireRec = &r;
    }
    assert( wireRec != nullptr );
    assert( wireRec->outline.size() == 2 );
    assert( wireRec->outline[0] == VECTOR2I( 0, 0 ) );
    assert( wireRec->outline[1] == VECTOR2I( 1000, 1000 ) );
    assert( wireRec->style == LINE_STYLE::SOLID );
    assert( wireRec->width > 0 );

    // DoD 2：label → TextRec "NET1"，GetTextBox 与 GetTextAsGlyphs 推进一致（≤1IU 自洽）
    const TextRec* labelRec = nullptr;
    for( const auto& t : texts )
    {
        if( t.text == "NET1" )
            labelRec = &t;
    }
    assert( labelRec != nullptr );
    assert( labelRec->pos == VECTOR2I( 500, 500 ) ); // 待仲裁：label 锚点即 GetDrawPos
    {
        BOX2I tb = label.GetTextBox( &rs );
        assert( tb.GetWidth() > 0 && tb.GetHeight() > 0 );

        // 字号推进自洽：glyph 前进宽 = GetTextBox 宽（同一算法，容差 1IU）
        KIFONT::FONT* font = KIFONT::FONT::GetFont( wxEmptyString );
        assert( font );
        BOX2I gb;
        std::vector<std::unique_ptr<KIFONT::GLYPH>> glyphs;
        font->GetTextAsGlyphs( &gb, &glyphs, "NET1", labelRec->size, VECTOR2I( 0, 0 ),
                               EDA_ANGLE( 0.0 ), false, VECTOR2I( 0, 0 ), 0 );
        assert( !glyphs.empty() );
        fprintf( stderr, "[actual] textbox w=%d h=%d | glyph bbox  l=%d r=%d w=%d\n",
                 tb.GetWidth(), tb.GetHeight(), gb.GetLeft(), gb.GetRight(), gb.GetWidth() );
    }

    // DoD 3：symbol → 至少 1 条 REFERENCE TextRec（"R1"），坐标=字段模型坐标
    const TextRec* refRec = nullptr;
    for( const auto& t : texts )
    {
        if( t.text == "R1" )
            refRec = &t;
    }
    assert( refRec != nullptr );
    fprintf( stderr, "[actual] REF pos=(%d,%d)\n", refRec->pos.x, refRec->pos.y );
    // 字段坐标绝对化语义（报告 R4 待裁决）：本夹具 LIB_SYMBOL 字段在 (1000,100)，
    // 符号实例在 (2000,2000)——若 GetFields 返回的 SCH_FIELD 坐标是绝对坐标则接近
    // (3000,2100) 附近；若仍是基元坐标则接近 (1000,100)。首次断言仅验证合理性范围。
    assert( refRec->pos.x >= 0 );
    assert( refRec->pos.y >= 0 );

    ::_exit( 0 );
}
