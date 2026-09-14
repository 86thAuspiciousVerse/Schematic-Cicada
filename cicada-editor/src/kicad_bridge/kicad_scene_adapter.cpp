// kicad_scene_adapter.cpp — 演示场景/持久文档装配实现（见头文件说明）
#include "kicad_scene_adapter.h"

#include "kicad_bridge/sch_geometry_batch.h"
#include "kicad_bridge/sch_interaction_engine.h"
#include "sch_screen.h"
#include "sch_line.h"
#include "sch_label.h"
#include "sch_symbol.h"
#include "sch_pin.h"
#include "sch_shape.h"
#include "sch_junction.h"
#include "lib_symbol.h"
#include "lib_id.h"
#include "sch_sheet_path.h"
#include "font/font.h"
#include "font/glyph.h"
#include "eda_shape.h"
#include "layer_ids.h"

#include <algorithm>

namespace cicada::editor::kicad_adapter
{

namespace
{

int r8( double c )
{
    return static_cast<int>( c * 255.0 + 0.5 );
}

CanvasPrimitive::Style makeStyle( const KIGFX::COLOR4D& color, int width, bool filled = false )
{
    CanvasPrimitive::Style st;
    st.colorR = r8( color.r );
    st.colorG = r8( color.g );
    st.colorB = r8( color.b );
    st.colorA = r8( color.a );
    st.width = width;
    st.filled = filled;
    return st;
}

// 选中覆色（KiCad LAYER_SELECTION_SHADOWS LIGHT 主题的不透明化；builtin_color_themes.h:348）
const KIGFX::COLOR4D kSelColor( 0.45f, 0.68f, 1.0f, 1.0f );

inline bool isInSelection( const void* aSource,
                           const std::vector<SCH_ITEM*>* aSelection )
{
    if( !aSource || !aSelection )
        return false;
    const SCH_ITEM* item = static_cast<const SCH_ITEM*>( aSource );
    return std::find( aSelection->begin(), aSelection->end(), item ) != aSelection->end();
}

SCH_SCREEN* buildDemoScreen( SCH_SHEET_PATH& aSheet )
{
    auto* screen = new SCH_SCREEN();

    // 实线 wire：(0,0) -> (100000,50000)（10cm × 5cm）
    auto* wire = new SCH_LINE( VECTOR2I( 0, 0 ), LAYER_WIRE );
    wire->SetStartPoint( VECTOR2I( 0, 0 ) );
    wire->SetEndPoint( VECTOR2I( 100000, 50000 ) );
    screen->Append( wire, false );

    // 虚线 wire：(0,60000) -> (60000,60000)（演示 ISO128-2 切分）
    auto* dash = new SCH_LINE( VECTOR2I( 0, 60000 ), LAYER_WIRE );
    dash->SetStartPoint( VECTOR2I( 0, 60000 ) );
    dash->SetEndPoint( VECTOR2I( 60000, 60000 ) );
    dash->GetStroke().SetLineStyle( LINE_STYLE::DASH );
    screen->Append( dash, false );

    // 结点（线中 50000,25000 恰在 wire 上）
    auto* junction = new SCH_JUNCTION( VECTOR2I( 50000, 25000 ) );
    screen->Append( junction, false );

    // 标签 NET1 @(22000,66000)
    auto* label = new SCH_LABEL( VECTOR2I( 22000, 66000 ), "NET1" );
    screen->Append( label, false );

    // R 符号：REFERENCE "R1" + VALUE "10k"；符号原点 (80000,15000)
    LIB_SYMBOL libSym( "R" );
    SCH_FIELD& refField = libSym.GetReferenceField();
    refField.SetText( "R1" );
    refField.SetPosition( VECTOR2I( 0, -4000 ) );
    SCH_FIELD& valueField = *libSym.GetField( FIELD_T::VALUE );
    valueField.SetText( "10k" );
    valueField.SetPosition( VECTOR2I( 0, 4000 ) );

    auto* symbol = new SCH_SYMBOL( libSym, LIB_ID( "device", "R" ), &aSheet, 1 );
    symbol->SetPosition( VECTOR2I( 80000, 15000 ) );
    screen->Append( symbol, false );

    return screen;
}

void emitTextGlyphs( const cicada::kicad_geometry::TextRec& rec, CanvasPrimitive& prim )
{
    // 字形折线（世界坐标 IU）→ 视图坐标
    KIFONT::FONT* font = KIFONT::FONT::GetFont( wxEmptyString );
    if( !font )
        return;

    BOX2I bbox;
    std::vector<std::unique_ptr<KIFONT::GLYPH>> glyphs;
    font->GetTextAsGlyphs( &bbox, &glyphs, rec.text, rec.size,
                           VECTOR2I( 0, 0 ), EDA_ANGLE( 0.0 ), rec.mirror, VECTOR2I( 0, 0 ), 0 );

    CanvasPrimitive::Style st = makeStyle( rec.color, rec.penWidth );
    st.width = std::max( 1, static_cast<int>( rec.penWidth * kScale + 0.5 ) );
    st.textSize = rec.size.x;
    for( const auto& g : glyphs )
    {
        auto* sg = dynamic_cast<KIFONT::STROKE_GLYPH*>( g.get() );
        if( !sg )
            continue;
        for( const auto& run : *sg )
        {
            std::vector<CanvasPoint> line;
            for( const auto& p : run )
                line.push_back( IuToView( VECTOR2I( static_cast<int>( p.x ) + rec.pos.x,
                                                    static_cast<int>( p.y ) + rec.pos.y ) ) );
            st.glyphs.push_back( std::move( line ) );
        }
    }
    prim.style = st;
}

// ── 符号真形状 emission（F.5 矩形/线段起步；世界化 = transform(local)+pos ──
void emitSymbolBodies( SCH_SYMBOL& aSymbol, const SCH_RENDER_SETTINGS& aSettings,
                       const std::vector<SCH_ITEM*>* aSelection, CanvasScene& aScene,
                       int& aIdx, bool aGhost = false )
{
    KIGFX::COLOR4D bodyColor = aSettings.GetColor( nullptr, LAYER_DEVICE );
    KIGFX::COLOR4D pinColor  = aSettings.GetColor( nullptr, LAYER_PIN );
    if( aGhost )
    {
        // 幽灵：与背景 50/50 混色（wxDC 无 alpha，半调色等效 KiCad 半透明 ghost）
        const KIGFX::COLOR4D bg = aSettings.GetBackgroundColor();
        bodyColor = KIGFX::COLOR4D( ( bodyColor.r + bg.r ) / 2.0, ( bodyColor.g + bg.g ) / 2.0,
                                    ( bodyColor.b + bg.b ) / 2.0, 1.0 );
        pinColor = KIGFX::COLOR4D( ( pinColor.r + bg.r ) / 2.0, ( pinColor.g + bg.g ) / 2.0,
                                   ( pinColor.b + bg.b ) / 2.0, 1.0 );
    }
    // 宽度 = IU→px 换算（kScale）；此前误将 IU 直当 px（144IU→144px 巨粗描边把矩形吞成"C"形 blob）
    const int penWidth = std::max( 1, static_cast<int>( aSettings.GetDefaultPenWidth() * kScale + 0.5 ) );

    const TRANSFORM& t = aSymbol.GetTransform();
    const VECTOR2I  pos = aSymbol.GetPosition();

    auto world = [&]( const VECTOR2I& p ) { return IuToView( t.TransformCoordinate( p ) + pos ); };
    auto sel = isInSelection( &aSymbol, aSelection );

    for( const SCH_ITEM& it : aSymbol.GetLibSymbolRef()->GetDrawItems() )
    {
        if( it.Type() != SCH_SHAPE_T )
            continue;

        const SCH_SHAPE* sh = static_cast<const SCH_SHAPE*>( &it );
        CanvasPrimitive  prim;
        prim.kind = CanvasPrimitive::Kind::SymbolBody;
        prim.id = "sym" + std::to_string( aIdx++ );
        prim.source = &aSymbol;
        prim.style = makeStyle( sel ? kSelColor : bodyColor, penWidth + ( sel ? 1 : 0 ),
                                false ); // v1 不填充（KiCad 库体=背景填充；填色随底做 B3c 打磨）

        switch( sh->GetShape() )
        {
        case SHAPE_T::RECTANGLE:
            prim.points = { world( sh->GetStart() ), world( VECTOR2I( sh->GetEnd().x, sh->GetStart().y ) ),
                            world( sh->GetEnd() ), world( VECTOR2I( sh->GetStart().x, sh->GetEnd().y ) ) };
            break;
        case SHAPE_T::SEGMENT:
            prim.points = { world( sh->GetStart() ), world( sh->GetEnd() ) };
            break;
        default:
            continue; // ARC/POLY/CIRCLE/BEZIER：B4 打磨
        }
        aScene.add_primitive( std::move( prim ) );
    }

    // pin 桩（真实世界线段：连通点 GetPinPhysicalPosition ↔ 体端 GetPinRoot+transform）
    for( SCH_PIN* lp : aSymbol.GetLibPins() )
    {
        if( !lp->IsVisible() )
            continue;

        CanvasPrimitive prim;
        prim.kind = CanvasPrimitive::Kind::Pin;
        prim.id = "pin" + std::to_string( aIdx++ );
        prim.source = &aSymbol;
        prim.style = makeStyle( sel ? kSelColor : pinColor, penWidth + ( sel ? 1 : 0 ) );
        prim.points = { world( lp->GetPinRoot() ), IuToView( aSymbol.GetPinPhysicalPosition( lp ) ) };
        aScene.add_primitive( std::move( prim ) );
    }
}

// ── 演示场景装配（MakeDemoScene 与 BuildScene 共用）──────────────────────────
CanvasScene assemble( const std::vector<cicada::kicad_geometry::StrokeRec>& aStrokes,
                      const std::vector<cicada::kicad_geometry::TextRec>& aTexts,
                      const KIGFX::COLOR4D& aBackground,
                      const std::vector<SCH_ITEM*>* aSelection )
{
    CanvasScene scene;
    scene.set_background( r8( aBackground.r ), r8( aBackground.g ), r8( aBackground.b ) );

    int idx = 0;
    for( const auto& s : aStrokes )
    {
        CanvasPrimitive prim;
        prim.kind = ( s.outline.size() == 2 && s.outline[0] == s.outline[1] )
                        ? CanvasPrimitive::Kind::Junction
                        : CanvasPrimitive::Kind::Wire;
        prim.id = "ki" + std::to_string( idx++ );
        prim.source = s.source;
        const bool sel = isInSelection( s.source, aSelection );
        prim.style = makeStyle( sel ? kSelColor : s.color, 0 );
        prim.style->width = std::max( 1, static_cast<int>( s.width * kScale + 0.5 ) )
                            + ( sel ? 1 : 0 );
        for( const auto& p : s.outline )
            prim.points.push_back( IuToView( p ) );

        if( prim.kind == CanvasPrimitive::Kind::Junction )
            prim.points.push_back( prim.points.front() );

        scene.add_primitive( std::move( prim ) );
    }

    for( const auto& t : aTexts )
    {
        CanvasPrimitive prim;
        prim.kind = CanvasPrimitive::Kind::Label;
        prim.id = "txt" + std::to_string( idx++ );
        prim.source = t.source;
        prim.text = t.text;
        prim.points.push_back( IuToView( t.pos ) );
        emitTextGlyphs( t, prim );
        if( isInSelection( t.source, aSelection ) )
        {
            prim.style->colorR = r8( kSelColor.r );
            prim.style->colorG = r8( kSelColor.g );
            prim.style->colorB = r8( kSelColor.b );
            prim.style->colorA = r8( kSelColor.a );
        }
        scene.add_primitive( std::move( prim ) );
    }

    return scene;
}

} // namespace

// ── B3b 放置幽灵 ────────────────────────────────────────────────────────────

void AddGhostPrimitives( CanvasScene& aScene, SCH_SYMBOL& aSymbol )
{
    SCH_RENDER_SETTINGS rs = cicada::kicad_geometry::SchGeometryBatch::MakeDefaultSettings();
    int idx = 0;
    emitSymbolBodies( aSymbol, rs, nullptr, aScene, idx, /*aGhost=*/true );
}

// ── B3a-2 持久文档路径 ──────────────────────────────────────────────────────

void SeedDemoScreen( cicada::kicad_geometry::SchInteractionEngine& aEngine )
{
    SCH_SCREEN*     screen = aEngine.screen();
    SCH_SHEET_PATH& path   = aEngine.sheetPath();

    // B3c 用户裁定重写：演示网络按"画线系统可画"原则——全部 45°/90° 段 + 全部端点落在
    // 50mil(12700IU) 网格上（旧演示的 26.57° 线/非网格 NET1 端点被用户否决）。
    const int G = 12700;

    // W1：45° 对角线 (0,0)→(88900,88900)
    auto* w1 = new SCH_LINE( VECTOR2I( 0, 0 ), LAYER_WIRE );
    w1->SetStartPoint( VECTOR2I( 0, 0 ) );
    w1->SetEndPoint( VECTOR2I( 7 * G, 7 * G ) );
    screen->Append( w1, false );

    // W2：垂直 (88900,88900)→(88900,177800)（与 W1 端点相接）
    auto* w2 = new SCH_LINE( VECTOR2I( 7 * G, 7 * G ), LAYER_WIRE );
    w2->SetStartPoint( VECTOR2I( 7 * G, 7 * G ) );
    w2->SetEndPoint( VECTOR2I( 7 * G, 14 * G ) );
    screen->Append( w2, false );

    // W3：水平 (88900,177800)→(177800,177800)
    auto* w3 = new SCH_LINE( VECTOR2I( 7 * G, 14 * G ), LAYER_WIRE );
    w3->SetStartPoint( VECTOR2I( 7 * G, 14 * G ) );
    w3->SetEndPoint( VECTOR2I( 14 * G, 14 * G ) );
    screen->Append( w3, false );

    // W4：从 W3 中段 (114300,177800) 向下伸（真实 T 三腿 → 该点 junction）
    auto* w4 = new SCH_LINE( VECTOR2I( 9 * G, 14 * G ), LAYER_WIRE );
    w4->SetStartPoint( VECTOR2I( 9 * G, 14 * G ) );
    w4->SetEndPoint( VECTOR2I( 9 * G, 7 * G ) );
    screen->Append( w4, false );

    // T 结：显式 junction（AnalyzePoint 3 腿：W3 左/右 + W4）
    auto* junction = new SCH_JUNCTION( VECTOR2I( 9 * G, 14 * G ) );
    screen->Append( junction, false );

    // 标签 NET1 @ W1 起点 (0,0)（dangling 线端上的标签）
    auto* label = new SCH_LABEL( VECTOR2I( 0, 0 ), "NET1" );
    screen->Append( label, false );

    // R 符号（引擎内置库模板；pin 位网格对齐 ±12700）：本体 (152400,88900) → pins 世界
    // (152400,101600)/(152400,76200)
    SCH_SYMBOL* symbol = aEngine.CreateSymbol( "R", VECTOR2I( 12 * G, 7 * G ) );
    if( symbol )
    {
        if( SCH_FIELD* v = symbol->GetField( FIELD_T::VALUE ) )
            v->SetText( "10k" );
        aEngine.PlaceSymbol( symbol );
    }

    aEngine.RefreshConnections();   // 连通/悬空真值（Seed 后文档即活）
}

CanvasScene BuildScene( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                        const std::vector<SCH_ITEM*>* aSelection )
{
    cicada::kicad_geometry::SchGeometryBatch batch;
    SCH_RENDER_SETTINGS rs = cicada::kicad_geometry::SchGeometryBatch::MakeDefaultSettings();

    std::vector<cicada::kicad_geometry::StrokeRec> strokes;
    std::vector<cicada::kicad_geometry::TextRec>   texts;
    batch.EmitScreen( aEngine.screen(), aEngine.sheetPath(), rs,
                      [&]( const cicada::kicad_geometry::StrokeRec& r ) { strokes.push_back( r ); },
                      [&]( const cicada::kicad_geometry::TextRec& r ) { texts.push_back( r ); } );

    CanvasScene scene = assemble( strokes, texts, rs.GetBackgroundColor(), aSelection );

    // 符号本体/pin（真形状；字段已由 batch 的 TextRec 覆盖）
    int idx = 0;
    for( SCH_ITEM* it : aEngine.screen()->Items() )
        if( SCH_SYMBOL* sym = dynamic_cast<SCH_SYMBOL*>( it ) )
            emitSymbolBodies( *sym, rs, aSelection, scene, idx );

    return scene;
}

// ── 一次性演示场景（占位兼容路径）───────────────────────────────────────────

CanvasScene MakeDemoScene()
{
    SCH_SHEET_PATH sheet;
    SCH_SCREEN* screen = buildDemoScreen( sheet );
    SCH_RENDER_SETTINGS rs = cicada::kicad_geometry::SchGeometryBatch::MakeDefaultSettings();

    std::vector<cicada::kicad_geometry::StrokeRec> strokes;
    std::vector<cicada::kicad_geometry::TextRec> texts;
    cicada::kicad_geometry::SchGeometryBatch batch;
    batch.EmitScreen( screen, sheet, rs,
                      [&]( const cicada::kicad_geometry::StrokeRec& r ) { strokes.push_back( r ); },
                      [&]( const cicada::kicad_geometry::TextRec& r ) { texts.push_back( r ); } );

    CanvasScene scene = assemble( strokes, texts, rs.GetBackgroundColor(), nullptr );

    // 符号本体（简版占位）：矩形 + 上下引脚桩 + 左侧字段已由 GetFields 覆盖
    {
        const VECTOR2I origin( 80000, 15000 );
        CanvasPrimitive body;
        body.kind = CanvasPrimitive::Kind::SymbolBody;
        body.id = "R1";
        body.text = "R1";
        const int w = 24000, h = 8000; // 2.4mm × 0.8mm 矩形电阻
        body.points = { IuToView( origin + VECTOR2I( -w / 2, h / 2 ) ),
                        IuToView( origin + VECTOR2I( w / 2, h / 2 ) ),
                        IuToView( origin + VECTOR2I( w / 2, -h / 2 ) ),
                        IuToView( origin + VECTOR2I( -w / 2, -h / 2 ) ) };
        body.style = makeStyle( KIGFX::COLOR4D( 132 / 255.0, 0 / 255.0, 0 / 255.0, 1.0 ), 6 );
        scene.add_primitive( std::move( body ) );

        for( int side : { -1, 1 } )
        {
            CanvasPrimitive pin;
            pin.kind = CanvasPrimitive::Kind::Pin;
            pin.id = "R1.pin" + std::to_string( side );
            pin.text = side < 0 ? "1" : "2";
            pin.points = { IuToView( origin + VECTOR2I( 0, side * h / 2 ) ),
                           IuToView( origin + VECTOR2I( 0, side * ( h / 2 + 6000 ) ) ) };
            pin.style = makeStyle( KIGFX::COLOR4D( 132 / 255.0, 0 / 255.0, 0 / 255.0, 1.0 ), 6 );
            scene.add_primitive( std::move( pin ) );
        }
    }

    return scene;
}

} // namespace cicada::editor::kicad_adapter
