// Slice-3 B2 smoke: CONNECTION_GRAPH::Recalculate direct-call verification.
// Assembles a minimal real hierarchy (PROJECT + SCHEMATIC::Reset + top sheet),
// appends two wires + one symbol (pin) into the top sheet's screen, and
// asserts connectivity/dangling truth produced by Recalculate itself.
// All assertions in IU domain (report ⑧/R11: no px conversions).
#include <project.h>
#include <schematic.h>
#include <sch_screen.h>
#include <sch_sheet.h>
#include <sch_sheet_path.h>
#include <sch_line.h>
#include <sch_symbol.h>
#include <lib_symbol.h>
#include <lib_id.h>
#include <connection_graph.h>
#include <sch_connection.h>
#include <kicad_bridge/snap_utils.h>

#include <assert.h>
#include <stdio.h>

int main()
{
    fprintf( stderr, "[probe] main enter\n" );
    PROJECT   prj;
    fprintf( stderr, "[probe] project ok\n" );
    SCHEMATIC sch( &prj );
    fprintf( stderr, "[probe] schematic ctor ok\n" );
    sch.Reset(); // 虚拟根 + 默认顶层页 + 连接图
    fprintf( stderr, "[probe] reset ok\n" );

    SCH_SHEET* top = nullptr;
    if( !sch.GetTopLevelSheets().empty() )
        top = sch.GetTopLevelSheets()[0];
    assert( top != nullptr );

    SCH_SCREEN* screen = top->GetScreen();
    assert( screen != nullptr );

    SCH_SHEET_PATH path;
    path.push_back( top );
    sch.SetCurrentSheet( path );

    // 夹具：wire1 (0,0)-(100000,0)；wire2 (100000,0)-(200000,0) 共享端点；
    // 符号 R1 引脚落在 (100000, 20000)（不与 wire 相接——保持可判定的 dangling 语义）
    auto* wire1 = new SCH_LINE( VECTOR2I( 0, 0 ), LAYER_WIRE );
    wire1->SetStartPoint( VECTOR2I( 0, 0 ) );
    wire1->SetEndPoint( VECTOR2I( 100000, 0 ) );
    screen->Append( wire1, false );

    auto* wire2 = new SCH_LINE( VECTOR2I( 100000, 0 ), LAYER_WIRE );
    wire2->SetStartPoint( VECTOR2I( 100000, 0 ) );
    wire2->SetEndPoint( VECTOR2I( 200000, 0 ) );
    screen->Append( wire2, false );

    LIB_SYMBOL libSym( "R" );
    auto* symbol = new SCH_SYMBOL( libSym, LIB_ID( "device", "R" ), &path, 1 );
    symbol->SetPosition( VECTOR2I( 80000, 20000 ) );
    screen->Append( symbol, false );

    // Recalculate（裁决=可直接调用；内部自带 TestDanglingEnds）
    CONNECTION_GRAPH* g = sch.ConnectionGraph();
    assert( g != nullptr );
    g->Recalculate( sch.Hierarchy(), true, nullptr, nullptr );

    // ① 共享端点连接：两 wire 在 (100000,0) 相接 → 该端不 dangling、SubgraphCode 相等
    fprintf( stderr, "[actual] wire1 sDang=%d eDang=%d | wire2 sDang=%d eDang=%d\n",
             wire1->IsStartDangling(), wire1->IsEndDangling(),
             wire2->IsStartDangling(), wire2->IsEndDangling() );
    assert( !wire1->IsEndDangling() );      // (100000,0) 端 = wire1 终点
    assert( !wire2->IsStartDangling() );    // (100000,0) 端 = wire2 起点
    assert( wire1->IsStartDangling() );     // (0,0) 孤端 = dangling
    assert( wire2->IsEndDangling() );       // (200000,0) 孤端 = dangling

    SCH_CONNECTION* c1 = wire1->Connection( &path );
    SCH_CONNECTION* c2 = wire2->Connection( &path );
    assert( c1 && c2 );
    fprintf( stderr, "[actual] wire1 subgraph=%d wire2 subgraph=%d\n",
             c1->SubgraphCode(), c2->SubgraphCode() );
    assert( c1->SubgraphCode() == c2->SubgraphCode() );

    // ② symbol 引脚不接 wire → symbol 孤立（无连接）
    SCH_CONNECTION* cs = symbol->Connection( &path );
    if( cs )
    {
        fprintf( stderr, "[actual] symbol subgraph=%d net=%s\n", cs->SubgraphCode(),
                 cs->Name().ToUTF8().data() );
        assert( cs->SubgraphCode() != c1->SubgraphCode() );
    }

    // ── B1：网格/端点吸附 ──
    // SnapGrid：50mil=12700IU，11000→12700、9000→12700、100→0
    assert( cicada::kicad_geometry::SnapGrid( VECTOR2I( 11000, 9000 ) ) == VECTOR2I( 12700, 12700 ) );
    assert( cicada::kicad_geometry::SnapGrid( VECTOR2I( 100, 100 ) ) == VECTOR2I( 0, 0 ) );

    // SnapEndpoints：在 (100000+300, 0) 附近 → 吸附回共享端点 (100000,0)=owner wire1/wire2
    {
        auto snap = cicada::kicad_geometry::SnapEndpoints( screen, path, VECTOR2I( 100300, 0 ), 1000 );
        assert( snap.has_value() );
        assert( snap->pt == VECTOR2I( 100000, 0 ) );

        // 引脚吸附：符号原点在 (80000,20000)，LIB_SYMBOL 未加 pin 时 GetPins 为空——
        // 为验证 pin 路径，向 libSym 加一个 pin（本地 (0,12000)，世界 (80000,32000)）
        LIB_SYMBOL* mutableLib = const_cast<LIB_SYMBOL*>( symbol->GetLibSymbolRef().get() );
        SCH_PIN*    pin = new SCH_PIN( mutableLib );
        pin->SetPosition( VECTOR2I( 0, 12000 ) );
        pin->SetNumber( "1" );
        mutableLib->AddDrawItem( pin );
        symbol->UpdatePins();
        fprintf( stderr, "[actual] pins=%zu\n", symbol->GetPins( &path ).size() );
        // B3a-2 修正（R-wx1）：单变换 = LIB pin（GetLibPins）+ GetPinPhysicalPosition——
        // 代理 pin（GetPins(&path)）的 GetPosition() 已含 transform+m_pos（sch_pin.cpp:254-260），
        // 再经 GetPinPhysicalPosition 是双重变换（旧断言 pinWorld=(160000,52000) 即其产物）。
        const SCH_PIN* firstPin = symbol->GetLibPins()[0];
        const VECTOR2I pinWorld = symbol->GetPinPhysicalPosition( firstPin );
        fprintf( stderr, "[actual] pinWorld=(%d,%d)\n", pinWorld.x, pinWorld.y );
        // 语义断言：探针=真实 pinWorld 偏移，SnapEndpoints 须命中该 pin 且标 isPin
        const VECTOR2I probe( pinWorld.x + 300, pinWorld.y - 300 );
        auto pnear = cicada::kicad_geometry::SnapEndpoints( screen, path, probe, 1000 );
        assert( pnear.has_value() );
        fprintf( stderr, "[actual] winner=(%d,%d) isPin=%d owner=%p\n",
                 pnear->pt.x, pnear->pt.y, pnear->isPin, (void*) pnear->owner );
        assert( pnear->isPin );
        assert( pnear->pt == pinWorld );
    }

    ::_exit( 0 );
}
