// shape_synth_test.cpp — M1e-1：形状块 → .kicad_sym 确定性生成 + 校验
// 断言：几何按 docs/02 附录 A（同侧等距对称/引脚在体外侧/轮廓取偶）；
// 生成文本经 lib_loader::ParseKicadSym 回环（引擎装载唯一真值路径）后 pin/矩形 精确一致。
#include <service/shape_synth.h>
#include <service/lib_loader.h>

#include <sch_item.h>
#include <sch_shape.h>
#include <sch_pin.h>

#include <stdio.h>

static int failures = 0;

#define RUN(name, cond)                                                          \
    do {                                                                         \
        if( !( cond ) )                                                          \
        {                                                                        \
            fprintf( stderr, "FAIL: %s (line %d)\n", name, __LINE__ );           \
            ++failures;                                                          \
        }                                                                        \
    } while( 0 )

using cicada::editor::service::ShapeBlock;
using cicada::editor::service::ShapeBlockPin;
using cicada::editor::service::ValidateShapeBlock;
using cicada::editor::service::SynthesizeSymbolText;

static ShapeBlock three_pin_block()
{
    ShapeBlock block;
    block.name      = "AMS1117";
    block.refPrefix = "U";
    block.pins      = { { "1", "GND", "power_in", "left" },
                        { "2", "VOUT", "power_out", "right" },
                        { "3", "VIN", "power_in", "right" } };
    return block;
}

int main()
{
    // ── 校验拒绝 ─────────────────────────────────────────────────────────────
    {
        ShapeBlock bad = three_pin_block();
        bad.name.clear();
        RUN( "validate name required", !ValidateShapeBlock( bad ).empty() );
        bad = three_pin_block();
        bad.pins.clear();
        RUN( "validate pins empty", !ValidateShapeBlock( bad ).empty() );
        bad = three_pin_block();
        bad.pins[ 1 ].electrical = "mystery";
        RUN( "validate electrical enum", !ValidateShapeBlock( bad ).empty() );
        bad = three_pin_block();
        bad.pins[ 2 ].number = "1";
        RUN( "validate number unique", !ValidateShapeBlock( bad ).empty() );
        bad = three_pin_block();
        bad.pins[ 0 ].side = "diagonal";
        RUN( "validate side enum", !ValidateShapeBlock( bad ).empty() );
        bad = three_pin_block();
        bad.bodyGiven = true;
        bad.bodyW     = 500;
        bad.bodyH     = 25400;
        RUN( "validate body range", !ValidateShapeBlock( bad ).empty() );
    }

    // ── 确定性几何：混合 side（1 左 / 2 右）───────────────────────────────────
    {
        ShapeBlock block = three_pin_block();
        std::string text;
        RUN( "synth ok", SynthesizeSymbolText( block, text ) );
        std::vector<cicada::editor::service::LibItem> items;
        std::string                                  err;
        RUN( "synth parses via lib_loader", cicada::editor::service::ParseKicadSym( text, items, &err ) );
        RUN( "one symbol", items.size() == 1 );
        if( items.size() != 1 )
            return 1;
        const auto& sym = items[ 0 ].symbol;
        RUN( "name AMS1117", std::string( sym->GetName().ToUTF8().data() ) == "AMS1117" );
        RUN( "pins count 3", sym->GetPins().size() == 3 );
        // 水平侧=0 → 宽 2*MARGIN=25400；垂直侧=2 → 高 (2-1)*PITCH+2*MARGIN=38100
        // 引脚 on 右 2 个：±6400（0.5×12700 先量化到 0.01mm 网格：round(0.5×127)=64 → 6400 IU）；
        // pin1 左：x=-(12700+25400)=-38100, y=0。旧断言写的是半格值 ±6350 —— 那正是与 runtime
        // 的 G 取整/导出文本差 0.01mm 的根源（2026-09-13，docs/recon/one-grid-offset）。
        const int halfW = 12700, halfH = 19050;
        // 回环后 pin at = 连接点（体外侧）、length 向体内
        const auto* p1 = sym->GetPins()[ 0 ];
        const auto* p2 = sym->GetPins()[ 1 ];
        const auto* p3 = sym->GetPins()[ 2 ];
        RUN( "pin1 pos", p1->GetPosition() == VECTOR2I( -( halfW + 25400 ), 0 ) );
        RUN( "pin2 pos", p2->GetPosition() == VECTOR2I( halfW + 25400, -6400 ) );
        RUN( "pin3 pos", p3->GetPosition() == VECTOR2I( halfW + 25400, 6400 ) );
        // 不变量：每个引脚坐标都必须落在 0.01mm 网格上（半格值会让引擎与 runtime 的文本差一格）。
        bool onGrid = true;
        for( const auto* pin : sym->GetPins() )
        {
            onGrid = onGrid && ( pin->GetPosition().x % 100 == 0 ) && ( pin->GetPosition().y % 100 == 0 );
        }
        RUN( "pins on the 0.01mm grid", onGrid );
        RUN( "pin length", p1->GetLength() == 25400 );
        RUN( "pin1 number", std::string( p1->GetNumber().ToUTF8().data() ) == "1" );
        RUN( "pin2 number", std::string( p2->GetNumber().ToUTF8().data() ) == "2" );
        RUN( "pin1 name", std::string( p1->GetName().ToUTF8().data() ) == "GND" );
        // 矩形体：中心对称
        bool rectOk = false;
        for( const auto& item : sym->GetDrawItems() )
        {
            if( item.Type() != SCH_SHAPE_T )
                continue;
            if( static_cast<const SCH_SHAPE*>( static_cast<const SCH_ITEM*>( &item ) )->GetShape()
                != SHAPE_T::RECTANGLE )
                continue;
            const auto& shape = *static_cast<const SCH_SHAPE*>( static_cast<const SCH_ITEM*>( &item ) );
            rectOk = shape.GetStart() == VECTOR2I( -halfW, -halfH )
                     && shape.GetEnd() == VECTOR2I( halfW, halfH );
            break;
        }
        RUN( "body rect exact", rectOk );
        // refPrefix（解析器提取进 LibItem.refPrefix；Reference 字段由引擎注册时使用）
        RUN( "ref prefix U", items[ 0 ].refPrefix == wxString( "U" ) );
    }

    // ── 缺省 side：四边均分 top→right→bottom→left，各 1 引脚在中心相位 ──────
    {
        ShapeBlock block;
        block.name = "IC4";
        block.pins = { { "1", "A", "input", "" }, { "2", "B", "input", "" },
                       { "3", "C", "output", "" }, { "4", "D", "passive", "" } };
        std::string text;
        RUN( "synth ok (assign)", SynthesizeSymbolText( block, text ) );
        std::vector<cicada::editor::service::LibItem> items;
        RUN( "assign parses", cicada::editor::service::ParseKicadSym( text, items, nullptr ) );
        RUN( "assign one symbol", items.size() == 1 );
        if( items.size() != 1 )
            return 1;
        const auto& pins = items[ 0 ].symbol->GetPins();
        RUN( "assign 4 pins", pins.size() == 4 );
        if( pins.size() == 4 )
        {
            const int edge = 12700 + 25400;   // halfW(1 边)=12700 + PIN_LEN
            RUN( "top pin", pins[ 0 ]->GetPosition() == VECTOR2I( 0, edge ) );
            RUN( "right pin", pins[ 1 ]->GetPosition() == VECTOR2I( edge, 0 ) );
            RUN( "bottom pin", pins[ 2 ]->GetPosition() == VECTOR2I( 0, -edge ) );
            RUN( "left pin", pins[ 3 ]->GetPosition() == VECTOR2I( -edge, 0 ) );
        }
    }

    // ── body 覆盖：方体 50800×50800 → pin 外沿 = 25400+25400 = 50800 ─────────
    {
        ShapeBlock block = three_pin_block();
        block.bodyGiven  = true;
        block.bodyW      = 50800;
        block.bodyH      = 50800;
        std::string text;
        RUN( "synth ok (body)", SynthesizeSymbolText( block, text ) );
        std::vector<cicada::editor::service::LibItem> items;
        RUN( "body parses", cicada::editor::service::ParseKicadSym( text, items, nullptr ) );
        if( items.size() == 1 )
            RUN( "body pins outside", items[ 0 ].symbol->GetPins()[ 0 ]->GetPosition()
                                          == VECTOR2I( -( 25400 + 25400 ), 0 ) );
    }

    if( failures == 0 )
        fprintf( stderr, "shape_synth ALL OK\n" );
    return failures == 0 ? 0 : 1;
}
