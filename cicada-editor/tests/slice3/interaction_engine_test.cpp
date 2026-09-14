// Slice-3 B3a-1 gate: SchInteractionEngine 原子操作验收（DoD 四条模型侧版）。
// 依据：docs/07-子代理过程产物/KiCad抽取施工/B3壳层交互引擎探路.md
// 覆盖：HitTest 真语义 / MoveSelection 字段随动 / 端点拖 2腿不出点-3腿出点 /
//       undo-redo 往返（含放置）/ PlaceSymbol 位号=REFDES_TRACKER。
// 全部断言 IU 域（R11：无 px 换算）。
#include <kicad_bridge/sch_interaction_engine.h>
#include <kicad_bridge/snap_utils.h>

#include <sch_line.h>
#include <sch_junction.h>
#include <sch_symbol.h>
#include <sch_pin.h>
#include <sch_field.h>
#include <sch_item.h>
#include <template_fieldnames.h>
#include <layer_ids.h>

#include <assert.h>
#include <stdio.h>
#include <wx/debug.h>
#include <windows.h>
#include <dbghelp.h>
#pragma comment( lib, "dbghelp.lib" )

using cicada::kicad_geometry::SchInteractionEngine;

// ── 崩溃栈转储（配合 Debug 构建 PDB 出符号；Release 出地址）──
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
        DWORD64 dish = 0;
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

// 无 wxApp 环境的断言捕获：不弹模态对话框（会卡住整个测试管线），
// 打印断言位置后继续（真语义回退路径上的 wxFAIL 属预期；真崩溃由 sehHandler 转储栈）。
static void testAssertHandler( const wxString& aFile, int aLine, const wxString& aFunc,
                               const wxString& aCond, const wxString& aMsg )
{
    fprintf( stderr, "[ASSERT] %s:%d in %s | cond=%s | msg=%s\n",
             aFile.ToUTF8().data(), aLine, aFunc.ToUTF8().data(),
             aCond.ToUTF8().data(), aMsg.ToUTF8().data() );
    fflush( stderr );
}

static int countItems( SCH_SCREEN* aScreen )
{
    int n = 0;
    for( SCH_ITEM* it : aScreen->Items() )
        n++;
    return n;
}

static int countJunctions( SCH_SCREEN* aScreen )
{
    int n = 0;
    for( SCH_ITEM* it : aScreen->Items() )
        if( it->Type() == SCH_JUNCTION_T )
            n++;
    return n;
}

int main()
{
    SetUnhandledExceptionFilter( sehHandler );
    wxSetAssertHandler( testAssertHandler );

    fprintf( stderr, "[probe] engine ctor\n" );
    SchInteractionEngine eng;
    fprintf( stderr, "[probe] engine ctor ok\n" );
    SCH_SCREEN*      screen = eng.screen();
    SCH_SHEET_PATH&  path   = eng.sheetPath();
    assert( screen != nullptr );

    // ── 夹具 ────────────────────────────────────────────────────────────────
    auto* wireA = new SCH_LINE( VECTOR2I( 0, 0 ), LAYER_WIRE );
    wireA->SetStartPoint( VECTOR2I( 0, 0 ) );
    wireA->SetEndPoint( VECTOR2I( 200000, 0 ) );
    screen->Append( wireA, false );
    fprintf( stderr, "[probe] wireA appended\n" );

    auto* wireB = new SCH_LINE( VECTOR2I( 400000, 40000 ), LAYER_WIRE );
    wireB->SetStartPoint( VECTOR2I( 400000, 40000 ) );
    wireB->SetEndPoint( VECTOR2I( 400000, 0 ) );
    screen->Append( wireB, false );
    fprintf( stderr, "[probe] wireB appended\n" );

    // 放置（引擎真路径：内置库模板 → CreateSymbol → PlaceSymbol 分配位号）
    SCH_SYMBOL* sym = eng.CreateSymbol( "R", VECTOR2I( 80000, 20000 ) );
    fprintf( stderr, "[probe] CreateSymbol ok sym=%p\n", (void*) sym );
    assert( sym != nullptr );
    eng.PlaceSymbol( sym );
    fprintf( stderr, "[probe] PlaceSymbol ok\n" );
    fprintf( stderr, "[actual] ref=%s pins=%zu\n", sym->GetRef( &path, false ).ToUTF8().data(),
             sym->GetPins( &path ).size() );
    assert( sym->GetRef( &path, false ) == "R1" );            // REFDES_TRACKER：首个=R1
    assert( sym->GetPins( &path ).size() == 2 );              // 模板两引脚代理就位

    // ── DoD1 点选真语义（HitTest 窄查序列）───────────────────────────────────
    SCH_ITEM* hit = eng.HitTest( VECTOR2I( 100000, 0 ), 100 );
    assert( hit == wireA );                                   // 线中点→该线
    hit = eng.HitTest( VECTOR2I( 80000, 20000 ), 100 );
    assert( hit == sym );                                     // 本体矩形中点→该符号
    hit = eng.HitTest( VECTOR2I( 400000, 40000 ), 100 );
    assert( hit == wireB );                                   // wireB 起点

    // ── DoD2 拖移随动（字段位移 == 符号位移）─────────────────────────────────
    const VECTOR2I prePos   = sym->GetPosition();
    const VECTOR2I preField = sym->GetField( FIELD_T::REFERENCE )->GetTextPos();
    eng.SetSelection( std::vector<SCH_ITEM*>{ sym } );
    std::vector<SCH_ITEM*> moved = eng.MoveSelection( VECTOR2I( 5000, 3000 ) );
    assert( moved.size() == 1 );
    assert( sym->GetPosition() == prePos + VECTOR2I( 5000, 3000 ) );
    fprintf( stderr, "[actual] field pre=(%d,%d) after=(%d,%d)\n", preField.x, preField.y,
             sym->GetField( FIELD_T::REFERENCE )->GetTextPos().x,
             sym->GetField( FIELD_T::REFERENCE )->GetTextPos().y );
    assert( sym->GetField( FIELD_T::REFERENCE )->GetTextPos() == preField + VECTOR2I( 5000, 3000 ) );

    assert( eng.Undo() );
    assert( sym->GetPosition() == prePos );
    assert( sym->GetField( FIELD_T::REFERENCE )->GetTextPos() == preField );
    assert( eng.Redo() );
    assert( sym->GetPosition() == prePos + VECTOR2I( 5000, 3000 ) );

    // ── B3a-2-1：pin 容差命中（2b 步）与 SnapEndpoints 单变换 ────────────────
    // sym 现位 (85000,23000)；两 LIB pin 世界位 = (85000, 23000±12700)（identity transform）
    {
        SCH_ITEM* hit2 = eng.HitTest( VECTOR2I( 84800, 35000 ), 1000 );   // 偏离 pin 700IU
        assert( hit2 == sym );                                            // 容差命中→父符号

        auto pinSnap = cicada::kicad_geometry::SnapEndpoints( screen, path,
                                                              VECTOR2I( 84900, 35600 ), 1000 );
        assert( pinSnap.has_value() );
        assert( pinSnap->isPin );                       // 单变换后语义不变
        assert( pinSnap->pt == VECTOR2I( 85000, 35700 ) );   // 单变换世界坐标（模板 pin 位 ±12700）
    }

    // ── DoD3 端点拖（C.2 口径：2 腿不出点 / 3 腿 T 出点）──────────────────────
    const int j0 = countJunctions( screen );
    // 2 腿：wireB 端点 (400000,0) → wireA 端点 (200000,0) = 端点并端点
    eng.MoveLineEndpoint( wireB, false, VECTOR2I( 200000, 0 ) );
    assert( countJunctions( screen ) == j0 );                  // 不出点
    assert( !wireB->IsEndDangling() );
    assert( !wireA->IsEndDangling() );
    assert( wireB->GetEndPoint() == VECTOR2I( 200000, 0 ) );

    // 3 腿：wireB 端点 → wireA 中段 (150000,0) = T 字 → 出点
    eng.MoveLineEndpoint( wireB, false, VECTOR2I( 150000, 0 ) );
    assert( countJunctions( screen ) == j0 + 1 );
    bool foundJunction = false;
    for( SCH_ITEM* it : screen->Items() )
        if( it->Type() == SCH_JUNCTION_T && it->GetPosition() == VECTOR2I( 150000, 0 ) )
            foundJunction = true;
    assert( foundJunction );
    assert( !wireB->IsEndDangling() );
    assert( wireB->Connection( &path ) != nullptr );
    assert( wireA->Connection( &path ) != nullptr );
    fprintf( stderr, "[actual] subgraph wireA=%d wireB=%d\n",
             wireA->Connection( &path )->SubgraphCode(),
             wireB->Connection( &path )->SubgraphCode() );
    assert( wireA->Connection( &path )->SubgraphCode() == wireB->Connection( &path )->SubgraphCode() );
    assert( wireA->IsEndDangling() );   // (200000,0) 已无 wireB；两端悬空（中间经 junction 连通）

    // ── DoD4 undo 往返（端点拖整体回滚：junction 先撤、端点逐级还原）─────────
    assert( eng.Undo() );                                                       // 撤 ADD junction
    fprintf( stderr, "[actual] after undo1 junctions=%d wbEnd=(%d,%d)\n",
             countJunctions( screen ), wireB->GetEndPoint().x, wireB->GetEndPoint().y );
    assert( countJunctions( screen ) == j0 );
    assert( eng.Undo() );                                                       // 撤 MODIFY wb#2
    fprintf( stderr, "[actual] after undo2 wbEnd=(%d,%d)\n",
             wireB->GetEndPoint().x, wireB->GetEndPoint().y );
    assert( wireB->GetEndPoint() == VECTOR2I( 200000, 0 ) );
    assert( eng.Undo() );                                                       // 撤 MODIFY wb#1
    assert( wireB->GetEndPoint() == VECTOR2I( 400000, 0 ) );
    assert( eng.Undo() );                                                       // 撤 MODIFY sym
    fprintf( stderr, "[actual] after undo4 symPos=(%d,%d) prePos=(%d,%d)\n",
             sym->GetPosition().x, sym->GetPosition().y, prePos.x, prePos.y );
    assert( sym->GetPosition() == prePos );
    assert( eng.Undo() );                                                       // 撤 ADD sym（放置）
    assert( countItems( screen ) == 2 );                                        // 剩 wireA+wireB
    assert( eng.Redo() );                                                       // 重做放置
    assert( sym->GetRef( &path, false ) == "R1" );                              // 位号不回退（P6）
    assert( eng.Redo() );                                                       // 重做拖移
    assert( sym->GetPosition() == prePos + VECTOR2I( 5000, 3000 ) );

    // ── 删除 + undo/redo（F.3 + E.3b REMOVE）─────────────────────────────────
    const int n0 = countItems( screen );
    eng.SetSelection( std::vector<SCH_ITEM*>{ wireB } );
    eng.DeleteSelection();
    assert( countItems( screen ) == n0 - 1 );
    assert( eng.Undo() );
    assert( countItems( screen ) == n0 );                                       // REMOVE 逆=原指针回树
    assert( eng.Redo() );
    assert( countItems( screen ) == n0 - 1 );
    assert( eng.Undo() );
    assert( countItems( screen ) == n0 );

    // ── B3c 画线链：CommitWireChain（整条一次撤销）─────────────────────────────
    {
        const int before = countItems( screen );
        eng.CommitWireChain( { VECTOR2I( 300000, 0 ), VECTOR2I( 300000, 50000 ),
                               VECTOR2I( 400000, 50000 ) } );   // 折角链 2 段
        assert( countItems( screen ) == before + 2 );
        assert( eng.Undo() );                                                 // 一次撤整条
        assert( countItems( screen ) == before );
        assert( eng.Redo() );                                                 // 一次重做整条
        assert( countItems( screen ) == before + 2 );
        assert( eng.Undo() );
        assert( countItems( screen ) == before );
    }

    // ── B3c bug3：画线穿越自动 junction（AnalyzePoint break=true 语义）────────
    {
        const int jBefore = countJunctions( screen );
        const int nBefore = countItems( screen );
        eng.CommitWireChain( { VECTOR2I( 100000, -50000 ),
                               VECTOR2I( 100000, 50000 ) } );   // 纯穿越 wireA 中段
        assert( countJunctions( screen ) == jBefore + 1 );       // 穿越处出点
        assert( countItems( screen ) == nBefore + 2 );           // 链 1 段 + junction 1 点
        assert( eng.Undo() );                                    // junction
        assert( countJunctions( screen ) == jBefore );
        assert( eng.Undo() );                                    // chain
        assert( countItems( screen ) == nBefore );
    }

    fprintf( stderr, "[probe] B3a-1 engine gate OK\n" );
    ::_exit( 0 );
}
