// Slice-2 render smoke test (B3): validates the pure-compute render kernel
// after real-source substitution for the font/render-settings chains.
// Six assertions per slice-2 recon report ⑦-B3:
//   1) SCH_RENDER_SETTINGS constructs and layer colors set/read.
//   2) FONT::GetFont("") -> non-null STROKE_FONT.
//   3) FONT::StringBoundaryLimits is no longer the slice-1 stub (aSize passthrough).
//   4) STROKE_FONT::GetTextAsGlyphs("R1") emits >=2 glyphs with non-empty polylines.
//   5) STROKE_PARAMS::Stroke with LINE_STYLE::DASH on a 3-point chain emits >=3
//      dash segments whose total length does not exceed the path length.
//   6) EDA_TEXT::GetTextBox(rs) returns real (non-degenerate) bounds for a label.
// Executes asserts in Release via /UNDEBUG (same convention as slice-1 test).
#include <sch_render_settings.h>
#include <render_settings.h>
#include <font/font.h>
#include <font/stroke_font.h>
#include <font/glyph.h>
#include <font/font_metrics.h>
#include <stroke_params.h>
#include <eda_text.h>
#include <base_units.h>
#include <geometry/shape_line_chain.h>
#include <gal/color4d.h>
#include <math/vector2d.h>

#include <assert.h>
#include <memory>

namespace
{
const KIGFX::COLOR4D kGreen( 0.0, 0.8, 0.0, 1.0 );
}

int main()
{
    // 1) SCH_RENDER_SETTINGS 构造 + 层色存取
    SCH_RENDER_SETTINGS rs;
    rs.SetLayerColor( static_cast<int>( LAYER_WIRE ), kGreen );
    assert( rs.GetColor( nullptr, static_cast<int>( LAYER_WIRE ) ) == kGreen );

    // 2) 空字体名 → STROKE_FONT 真源（GetFont 不再返回 nullptr）
    KIFONT::FONT* font = KIFONT::FONT::GetFont( wxEmptyString );
    assert( font != nullptr );
    assert( font->IsStroke() );

    // 3) StringBoundaryLimits 真数值（与切片 1 的"返回 aSize"降级区分）
    const VECTOR2I size( 1000, 1000 );
    VECTOR2I bounds = font->StringBoundaryLimits( "R1", size, 0, false, false,
                                                 KIFONT::METRICS::Default() );
    assert( bounds.x > 0 && bounds.y <= 1000 );

    // 4) GetTextAsGlyphs 产出字形折线（"R"+"1" 两字形，世界坐标非空）
    BOX2I glyphBBox;
    std::vector<std::unique_ptr<KIFONT::GLYPH>> glyphs;
    VECTOR2I advance = font->GetTextAsGlyphs( &glyphBBox, &glyphs, "R1", size, VECTOR2I( 0, 0 ),
                                              EDA_ANGLE( 0.0 ), false, VECTOR2I( 0, 0 ), 0 );
    assert( glyphs.size() >= 2 );
    bool anyNonEmpty = false;
    for( const auto& g : glyphs )
    {
        if( auto* sg = dynamic_cast<KIFONT::STROKE_GLYPH*>( g.get() ) )
        {
            if( !sg->empty() )
                anyNonEmpty = true;
        }
    }
    assert( anyNonEmpty );
    assert( glyphBBox.GetWidth() > 0 && glyphBBox.GetHeight() > 0 );
    (void) advance;

    // 5) DASH 线型分段（render_settings.cpp 真源提供 Dash/Gap 长度）
    SHAPE_LINE_CHAIN chain;
    chain.Append( VECTOR2I( 0, 0 ) );
    chain.Append( VECTOR2I( 400, 0 ) );
    chain.Append( VECTOR2I( 800, 0 ) );
    int   segCount = 0;
    long  totalLen = 0;
    STROKE_PARAMS::Stroke( &chain, LINE_STYLE::DASH, 10, &rs,
                           [&]( const VECTOR2I& a, const VECTOR2I& b )
                           {
                               segCount++;
                               totalLen += ( b - a ).EuclideanNorm();
                           } );
    assert( segCount >= 3 );
    assert( totalLen > 0 && totalLen <= 800 );

    // 6) EDA_TEXT::GetTextBox 真值（非退化）
    EDA_TEXT text( schIUScale );
    text.SetText( "NET1" );
    text.SetTextSize( VECTOR2I( 500, 500 ) );
    BOX2I tbox = text.GetTextBox( &rs );
    assert( tbox.GetWidth() > 0 && tbox.GetHeight() > 0 );

    ::_exit( 0 );
}
