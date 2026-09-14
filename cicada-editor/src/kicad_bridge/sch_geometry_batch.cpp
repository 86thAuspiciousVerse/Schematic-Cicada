// sch_geometry_batch.cpp — 切片 2 几何批次实现（CICADA 自研）
#include "sch_geometry_batch.h"

#include <geometry/shape_segment.h>
#include <sch_field.h>
#include <default_values.h>
#include <settings/color_settings.h>

namespace cicada::kicad_geometry
{

void SchGeometryBatch::EmitScreen( SCH_SCREEN* aScreen, const SCH_SHEET_PATH& aSheet,
                                   SCH_RENDER_SETTINGS& aSettings,
                                   const std::function<void( const StrokeRec& )>& aStroke,
                                   const std::function<void( const TextRec& )>& aText ) const
{
    (void) aSheet; // v1：sheet path 用于换算已绝对化字段时按需展开

    for( SCH_ITEM* item : aScreen->Items() )
        emitItem( item, aSettings, aStroke, aText );
}

void SchGeometryBatch::emitItem( SCH_ITEM* aItem, SCH_RENDER_SETTINGS& aSettings,
                                 const std::function<void( const StrokeRec& )>& aStroke,
                                 const std::function<void( const TextRec& )>& aText ) const
{
    switch( aItem->Type() )
    {
    case SCH_LINE_T:
        emitLine( *static_cast<SCH_LINE*>( aItem ), aSettings, aStroke );
        break;

    case SCH_LABEL_T:
    case SCH_GLOBAL_LABEL_T:
    case SCH_HIER_LABEL_T:
    case SCH_DIRECTIVE_LABEL_T:
        emitTextBase( *static_cast<SCH_LABEL_BASE*>( aItem ),
                      static_cast<int>( aItem->GetLayer() ), aSettings, aText, aItem );
        break;

    case SCH_TEXT_T:
    case SCH_TEXTBOX_T:
        emitTextBase( *static_cast<SCH_TEXT*>( aItem ),
                      static_cast<int>( aItem->GetLayer() ), aSettings, aText, aItem );
        break;

    case SCH_JUNCTION_T:
    {
        // 结点：按实线纪律直接出 2 点折线（圆点由壳层按 width 画圆）
        SCH_JUNCTION* j = static_cast<SCH_JUNCTION*>( aItem );
        StrokeRec rec;
        rec.layer = static_cast<int>( aItem->GetLayer() );
        rec.color = aSettings.GetColor( nullptr, rec.layer );
        rec.width = std::max( j->GetEffectiveDiameter(), 0 );
        rec.style = LINE_STYLE::SOLID;
        rec.outline = { j->GetPosition(), j->GetPosition() };
        rec.source = j;
        aStroke( rec );
        break;
    }

    case SCH_SYMBOL_T:
        emitSymbol( *static_cast<SCH_SYMBOL*>( aItem ), aSettings, aText );
        break;

    default:
        break; // v1 范围外（SHEET/PIN 等），后置候选
    }
}

void SchGeometryBatch::emitLine( SCH_LINE& aLine, SCH_RENDER_SETTINGS& aSettings,
                                 const std::function<void( const StrokeRec& )>& aStroke ) const
{
    const VECTOR2I s = aLine.GetStartPoint();
    const VECTOR2I e = aLine.GetEndPoint();
    const int layer = static_cast<int>( aLine.GetLayer() );
    const KIGFX::COLOR4D color = aSettings.GetColor( nullptr, layer );
    const int width = std::max( aLine.GetEffectivePenWidth( &aSettings ), 0 );
    const LINE_STYLE style = aLine.GetLineStyle();

    if( style == LINE_STYLE::SOLID || style == LINE_STYLE::DEFAULT )
    {
        StrokeRec rec;
        rec.layer = layer;
        rec.color = color;
        rec.width = width;
        rec.style = LINE_STYLE::SOLID;
        rec.outline = { s, e };
        rec.source = &aLine;
        aStroke( rec );
        return;
    }

    // 非实线：STROKE_PARAMS::Stroke 分段（虚线/点划线协议来自 render_settings 真源）
    SHAPE_SEGMENT seg( s, e );
    STROKE_PARAMS::Stroke( &seg, style, width, &aSettings,
                           [&]( const VECTOR2I& a, const VECTOR2I& b )
                           {
                               StrokeRec rec;
                               rec.layer = layer;
                               rec.color = color;
                               rec.width = width;
                               rec.style = style;
                               rec.outline = { a, b };
                               rec.source = &aLine;
                               aStroke( rec );
                           } );
}

void SchGeometryBatch::emitTextBase( EDA_TEXT& aText, int aLayer, SCH_RENDER_SETTINGS& aSettings,
                                     const std::function<void( const TextRec& )>& aTextOut,
                                     const SCH_ITEM* aSource ) const
{
    TextRec rec;
    rec.layer = aLayer;
    rec.color = aSettings.GetColor( nullptr, aLayer );
    rec.text = aText.GetShownText( false );
    rec.pos = aText.GetDrawPos();
    rec.size = aText.GetTextSize();
    rec.angle = aText.GetDrawRotation();
    rec.mirror = aText.IsMirrored();
    rec.halign = aText.GetHorizJustify();
    rec.valign = aText.GetVertJustify();
    rec.penWidth = std::max( aText.GetEffectiveTextPenWidth(
                                 aSettings.GetDefaultPenWidth() ), 0 );
    rec.italic = aText.IsItalic();
    rec.multiline = aText.IsMultilineAllowed();
    rec.source = aSource;
    aTextOut( rec );
}

void SchGeometryBatch::emitSymbol( SCH_SYMBOL& aSymbol, SCH_RENDER_SETTINGS& aSettings,
                                   const std::function<void( const TextRec& )>& aText ) const
{
    for( SCH_FIELD& field : aSymbol.GetFields() )
    {
        if( !field.IsVisible() )
            continue;

        emitTextBase( field, static_cast<int>( field.GetLayer() ), aSettings, aText, &aSymbol );
    }
}

SCH_RENDER_SETTINGS SchGeometryBatch::MakeDefaultSettings()
{
    // KiCad 10.0.6 官方默认主题精确色值（源：vendor common/settings/builtin_color_themes.h
    // s_defaultTheme 表；运行时 COLOR_SETTINGS 构造未加载主题，故按表硬编码）
    SCH_RENDER_SETTINGS rs;
    rs.SetLayerColor( static_cast<int>( LAYER_SCHEMATIC_BACKGROUND ), KIGFX::COLOR4D( 245 / 255.0, 244 / 255.0, 239 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_WIRE ),                 KIGFX::COLOR4D( 0 / 255.0, 150 / 255.0, 0 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_JUNCTION ),             KIGFX::COLOR4D( 0 / 255.0, 150 / 255.0, 0 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_DEVICE ),               KIGFX::COLOR4D( 132 / 255.0, 0 / 255.0, 0 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_PIN ),                  KIGFX::COLOR4D( 132 / 255.0, 0 / 255.0, 0 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_PINNUM ),               KIGFX::COLOR4D( 169 / 255.0, 0 / 255.0, 0 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_REFERENCEPART ),        KIGFX::COLOR4D( 0 / 255.0, 100 / 255.0, 100 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_VALUEPART ),            KIGFX::COLOR4D( 0 / 255.0, 100 / 255.0, 100 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_LOCLABEL ),             KIGFX::COLOR4D( 15 / 255.0, 15 / 255.0, 15 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_HIERLABEL ),            KIGFX::COLOR4D( 114 / 255.0, 86 / 255.0, 0 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_GLOBLABEL ),          KIGFX::COLOR4D( 132 / 255.0, 0 / 255.0, 0 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_SHEETLABEL ),           KIGFX::COLOR4D( 0 / 255.0, 100 / 255.0, 100 / 255.0, 1.0 ) );
    rs.SetLayerColor( static_cast<int>( LAYER_NOTES ),                KIGFX::COLOR4D( 0 / 255.0, 0 / 255.0, 194 / 255.0, 1.0 ) );
    rs.SetDefaultPenWidth( DEFAULT_LINE_WIDTH_MILS * 1000 / 25 );
    return rs;
}

} // namespace cicada::kicad_geometry
