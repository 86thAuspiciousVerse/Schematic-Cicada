// sch_interaction_engine.cpp — 切片 3 B3 壳层交互引擎实现（CICADA 自研）
// 依据：docs/07-子代理过程产物/KiCad抽取施工/B3壳层交互引擎探路.md
// 全部模型原子调用真实 KiCad 语义（探路 A~G 已逐条给签名+file:line）；
// 壳层只做编排（命中优先序、undo 快照栈、补点时机）。
#include "sch_interaction_engine.h"

#include <sch_pin.h>
#include <sch_junction.h>
#include <sch_label.h>
#include <sch_line.h>
#include <sch_field.h>
#include <sch_shape.h>
#include <lib_symbol.h>
#include <lib_id.h>
#include <connection_graph.h>
#include <sch_connection.h>
#include <junction_helpers.h>
#include <refdes_tracker.h>
#include <template_fieldnames.h>
#include <eda_item_flags.h>
#include <core/typeinfo.h>
#include <eda_shape.h>
#include <layer_ids.h>
#include <pgm_base.h>
#include <math/box2.h>

#include <algorithm>
#include <cctype>
#include <cmath>

namespace cicada::kicad_geometry
{

namespace
{

// Pgm() 单例：KiCad 语义由 wxApp OnInit 设置（SetPgm）；壳层无 wxApp，
// 由引擎 ctor 代管（进程期单例、不析构=KiCad 惯例）。仅覆盖唯一纯虚 MacOpenFile。
class CicadaPgm : public PGM_BASE
{
public:
    CicadaPgm()
    {
        SetPgm( this );
        // 分配 settings/library 管理器（KiCad 语义 = wxApp OnInit → InitPgm；壳层无 wxApp，
        // 采用单元测试模式：分配后即返回 false——不载配置、不建窗口，仅保证
        // Pgm().GetSettingsManager() 等可达不为空）
        InitPgm( /*aHeadless=*/true, /*aSkipPyInit=*/true, /*aIsUnitTest=*/true );
    }

    void MacOpenFile( const wxString& ) override {}
};

CicadaPgm* g_pgmInstance = nullptr;

void EnsurePgm()
{
    // KiCad 语义：wxApp OnInit 时 SetPgm(PGM) + SetInstance(wxApp)。壳层无 wxEntry 生命周期，
    // 代管一个裸 wxApp（进程期单例，与 KiCad QA test_app_main.cpp 的 APP_TEST 同构）。
    if( !wxApp::GetInstance() )
        wxApp::SetInstance( new wxApp() );

    if( !PgmOrNull() )
        g_pgmInstance = new CicadaPgm();
}

} // namespace

// ── InteractionUndo ─────────────────────────────────────────────────────────

InteractionUndo::~InteractionUndo()
{
    delete image;
}

InteractionUndo::InteractionUndo( InteractionUndo&& aOther ) noexcept
    : kind( aOther.kind ), item( aOther.item ),
      image( aOther.image ), screen( aOther.screen ), chain( std::move( aOther.chain ) )
{
    aOther.image = nullptr;
}

InteractionUndo& InteractionUndo::operator=( InteractionUndo&& aOther ) noexcept
{
    if( this != &aOther )
    {
        delete image;
        kind = aOther.kind; item = aOther.item;
        image = aOther.image; screen = aOther.screen;
        chain = std::move( aOther.chain );
        aOther.image = nullptr;
    }
    return *this;
}

// ── 活文档装配（探路 §0 = connection_smoke_test.cpp:22-64 模板）──────────────

SchInteractionEngine::SchInteractionEngine() : sch_( &prj_ )
{
    EnsurePgm(); // Pgm() 断言根治：无 wxApp 时也保证 Pgm() 可返回（KiCad QA MOCK_PGM_BASE 同构）
    sch_.Reset();
    if( sch_.GetTopLevelSheets().empty() )
        return;

    SCH_SHEET* top = sch_.GetTopLevelSheets()[0];
    screen_ = top->GetScreen();
    path_.push_back( top );
    sch_.SetCurrentSheet( path_ );

    tracker_ = std::make_shared<REFDES_TRACKER>();
    ScanRefDes();
}

SchInteractionEngine::~SchInteractionEngine() = default;

// ── 命中（探路 A.3 窄查序列）────────────────────────────────────────────────

SCH_ITEM* SchInteractionEngine::HitTest( const VECTOR2I& aPtIU, int aAccIU,
                                         SCH_SYMBOL** aPinOwner )
{
    if( aPinOwner )
        *aPinOwner = nullptr;

    // 1) 结点（小圆）最上
    if( SCH_ITEM* hit = screen_->GetItem( aPtIU, aAccIU, SCH_JUNCTION_T ) )
        return hit;

    // 2) pin 精确点（端点命中语义）→ 升级为父符号
    SCH_SYMBOL* owner = nullptr;
    if( SCH_PIN* pin = screen_->GetPin( aPtIU, &owner, true ) )
    {
        (void) pin;
        if( owner )
        {
            if( aPinOwner )
                *aPinOwner = owner;
            return owner;
        }
    }

    // 2b) pin 容差命中（B3a-2 交互化；GetPin 两分支皆点精确，容差须自算）：
    //     遍历 RTree 符号 GetLibPins + GetPinPhysicalPosition（单变换；代理 pin 会双变换=R-wx1）。
    //     命中语义 = 选中父符号（KiCad prepareSelection: sch_move_tool.cpp:629）。
    {
        const VECTOR2I lo( aPtIU.x - aAccIU, aPtIU.y - aAccIU );
        const BOX2I    box( lo, VECTOR2I( 2 * aAccIU, 2 * aAccIU ) );
        SCH_SYMBOL*    owner2  = nullptr;
        SCH_PIN*       bestPin = nullptr;
        int            bestD   = aAccIU;

        for( SCH_ITEM* it : screen_->Items().Overlapping( SCH_SYMBOL_T, box ) )
        {
            SCH_SYMBOL* sym = static_cast<SCH_SYMBOL*>( it );
            if( !sym->GetLibSymbolRef() )
                continue;

            for( SCH_PIN* lp : sym->GetLibPins() )
            {
                if( !lp->IsVisible() )
                    continue;
                if( lp->GetUnit() > 0 && sym->GetUnit() > 0 && lp->GetUnit() != sym->GetUnit() )
                    continue;

                const VECTOR2I wp = sym->GetPinPhysicalPosition( lp );
                const VECTOR2I diff = wp - aPtIU;
                const int      d = std::max( std::abs( diff.x ), std::abs( diff.y ) ); // Chebyshev
                if( d <= bestD )
                {
                    bestD   = d;
                    bestPin = lp;
                    owner2  = sym;
                }
            }
        }

        (void) bestPin;
        if( owner2 )
        {
            if( aPinOwner )
                *aPinOwner = owner2;
            return owner2;
        }
    }

    // 3) 标签（含 3 类）
    if( SCH_ITEM* hit = screen_->GetLabel( aPtIU, aAccIU ) )
        return hit;

    // 4) 线（仅 LAYER_WIRE）
    if( SCH_ITEM* hit = screen_->GetWire( aPtIU, aAccIU ) )
        return hit;

    // 5) 其余（symbol/no_connect…）；默认类型 = SCH_LOCATE_ANY_T（勿传 TYPE_NOT_INIT）
    return screen_->GetItem( aPtIU, aAccIU, SCH_LOCATE_ANY_T );
}

// 可选白名单（报告 D.2）：替代不存在的 IsHitTestable/IsSelectable
bool SchInteractionEngine::EngineSelectable( const SCH_ITEM* aItem ) const
{
    if( !aItem )
        return false;

    switch( aItem->Type() )
    {
    case SCH_SYMBOL_T:
    case SCH_LINE_T:
    case SCH_JUNCTION_T:
    case SCH_LABEL_T:
    case SCH_GLOBAL_LABEL_T:
    case SCH_HIER_LABEL_T:
    case SCH_DIRECTIVE_LABEL_T:
    case SCH_NO_CONNECT_T:
        return !( aItem->GetEditFlags() & ( IS_MOVING | IS_DELETED | SKIP_STRUCT ) );
    default:
        return false; // field/pin/sheet/marker/bus-entry v1 不直选
    }
}

// ── 框选（报告 D.2）：v1 固定 contained 语义 ────────────────────────────────

std::vector<SCH_ITEM*> SchInteractionEngine::BoxSelect( const VECTOR2I& aCorner1,
                                                        const VECTOR2I& aCorner2,
                                                        bool aContained ) const
{
    const VECTOR2I lo( std::min( aCorner1.x, aCorner2.x ), std::min( aCorner1.y, aCorner2.y ) );
    const VECTOR2I hi( std::max( aCorner1.x, aCorner2.x ), std::max( aCorner1.y, aCorner2.y ) );
    const BOX2I    box( lo, hi - lo );

    std::vector<SCH_ITEM*> out;
    for( SCH_ITEM* it : screen_->Items().Overlapping( box ) )
    {
        if( EngineSelectable( it ) && it->HitTest( box, aContained, 0 ) )
            out.push_back( it );
    }
    return out;
}

// ── 拖移（报告 B.2 最小等价）：逐项 Move + undo(MODIFY) → CommitEdits ───────

std::vector<SCH_ITEM*> SchInteractionEngine::MoveSelection( const VECTOR2I& aDeltaIU )
{
    std::vector<SCH_ITEM*> changed;

    // B3c：junction 随所载线移动——被撬线的连接点若落在任一选中线段上则一并挪
    //（不可独拖，但跟线走；KiCad 移动 wire 时其上 junction 同步移动）
    std::vector<SCH_ITEM*> moving;
    for( SCH_ITEM* it : selection_ )
    {
        if( !it )
            continue;
        if( it->Type() == SCH_JUNCTION_T )
            continue; // junction 不可拖（KiCad 语义：点不得脱离线段；面板侧同样排除）
        moving.push_back( it );
    }

    for( SCH_ITEM* it : selection_ )
    {
        if( !it || it->Type() != SCH_LINE_T )
            continue;

        SCH_LINE*   line = static_cast<SCH_LINE*>( it );
        const VECTOR2I s = line->GetStartPoint(), e = line->GetEndPoint();
        const VECTOR2I dv = e - s;

        for( SCH_ITEM* jit : screen_->Items() )
        {
            if( jit->Type() != SCH_JUNCTION_T )
                continue;
            if( std::find( moving.begin(), moving.end(), jit ) != moving.end() )
                continue;

            const VECTOR2I jp = jit->GetPosition();
            const VECTOR2I pv = jp - s;
            // 点在线上：叉积=0 且点积 ∈ [0, |dv|²]
            if( (long long) dv.x * pv.y - (long long) dv.y * pv.x != 0 )
                continue;
            const long long dot = (long long) pv.x * dv.x + (long long) pv.y * dv.y;
            if( dot < 0 || dot > (long long) dv.x * dv.x + (long long) dv.y * dv.y )
                continue;
            moving.push_back( jit );
        }
    }

    // B3c 修正（用户勘误）：KiCad 移动线段 = 保持网络连接——邻接段共享端点跟随拉伸
    // （按中心段触发；绝不出现在网络"扯断出孤立段"的行为）。先记录各选中线的旧端点。
    struct StubInfo
    {
        SCH_LINE* line;
        VECTOR2I  s0, e0;
    };
    std::vector<StubInfo> stubs;
    for( SCH_ITEM* it : moving )
        if( it->Type() == SCH_LINE_T )
        {
            SCH_LINE* l = static_cast<SCH_LINE*>( it );
            stubs.push_back( { l, l->GetStartPoint(), l->GetEndPoint() } );
        }

    for( SCH_ITEM* it : moving )
    {
        SCH_ITEM* before = static_cast<SCH_ITEM*>( it->Clone() );
        it->Move( aDeltaIU );                       // SCH_SYMBOL::Move 头内联=字段随动

        StageUndo( InteractionUndo::Kind::MODIFY, it, screen_, before );
        changed.push_back( it );
    }

    // 邻段拉伸：与选中线共享旧端点的其它线（非移动集内）→ 共享端改为新位置
    for( const StubInfo& st : stubs )
    {
        for( SCH_ITEM* nit : screen_->Items() )
        {
            if( nit->Type() != SCH_LINE_T )
                continue;
            if( std::find( moving.begin(), moving.end(), nit ) != moving.end() )
                continue;

            SCH_LINE* n = static_cast<SCH_LINE*>( nit );
            if( n->GetStartPoint() == st.s0 || n->GetEndPoint() == st.s0
                || n->GetStartPoint() == st.e0 || n->GetEndPoint() == st.e0 )
            {
                SCH_ITEM* nb = static_cast<SCH_ITEM*>( n->Clone() );
                if( n->GetStartPoint() == st.s0 )
                    n->SetStartPoint( st.s0 + aDeltaIU );
                else if( n->GetEndPoint() == st.s0 )
                    n->SetEndPoint( st.s0 + aDeltaIU );
                if( n->GetStartPoint() == st.e0 )
                    n->SetStartPoint( st.e0 + aDeltaIU );
                else if( n->GetEndPoint() == st.e0 )
                    n->SetEndPoint( st.e0 + aDeltaIU );

                StageUndo( InteractionUndo::Kind::MODIFY, n, screen_, nb );
                changed.push_back( n );
            }
        }
    }

    if( !changed.empty() )
        CommitEdits( std::deque<SCH_ITEM*>( changed.begin(), changed.end() ) );

    return changed;
}

// ── 端点拖（报告 C.1/C.4）───────────────────────────────────────────────────

void SchInteractionEngine::MoveLineEndpoint( SCH_LINE* aLine, bool aStart, const VECTOR2I& aPtIU )
{
    if( !aLine )
        return;

    SCH_ITEM* before = static_cast<SCH_ITEM*>( aLine->Clone() );
    if( aStart )
        aLine->SetStartPoint( aPtIU );
    else
        aLine->SetEndPoint( aPtIU );
    

    StageUndo( InteractionUndo::Kind::MODIFY, aLine, screen_, before );
    CommitEdits( std::deque<SCH_ITEM*>{ aLine } );
}

// ── 提交单点（铁律 1/3 + 补点 + Recalculate）────────────────────────────────

void SchInteractionEngine::CommitEdits( const std::deque<SCH_ITEM*>& aChanged )
{
    if( !screen_ )
        return;

    // 铁律 1：几何改动后重索引（RTree bbox 一致）
    for( SCH_ITEM* it : aChanged )
        screen_->Update( it, false );

    // C.3 裁决：junction 必须引擎显式补点（Recalculate 不做）；不断线（连接图兜底）
    std::deque<EDA_ITEM*> changedED;
    for( SCH_ITEM* it : aChanged )
        changedED.push_back( it );

    std::vector<VECTOR2I> pts = screen_->GetNeededJunctions( changedED );

    // B3c bug3：画线穿越 junction（KiCad 工具层语义 = AnalyzePoint(aBreakCrossings=**true**)：
    // 穿越线按 2 方向计入 → 纯穿越也出点；GetNeededJunctions 内部是 false 版本，不含穿越）。
    // 对每条改动线 × 屏上每条其它线做段-段交点，交点上 AnalyzePoint(true) 判 junction。
    for( SCH_ITEM* it : aChanged )
    {
        if( it->Type() != SCH_LINE_T )
            continue;

        SCH_LINE*   L   = static_cast<SCH_LINE*>( it );
        const VECTOR2I p1 = L->GetStartPoint(), p2 = L->GetEndPoint();
        const VECTOR2I d1 = p2 - p1;

        for( SCH_ITEM* wit : screen_->Items() )
        {
            if( wit == it || wit->Type() != SCH_LINE_T )
                continue;

            SCH_LINE*   W   = static_cast<SCH_LINE*>( wit );
            const VECTOR2I q1 = W->GetStartPoint(), q2 = W->GetEndPoint();
            const VECTOR2I d2 = q2 - q1;

            // 段-段交点（t,u ∈ (0,1) 纯内部穿越；共享端点由 GetNeededJunctions 处理）
            // t = cross(q1-p1,d2)/cross(d1,d2)；u = cross(q1-p1,d1)/cross(d1,d2)
            const long long den = (long long) d1.x * d2.y - (long long) d1.y * d2.x;
            if( den == 0 )
                continue;
            const long long num = (long long) p1.y * d2.x + (long long) q1.x * d2.y
                                - (long long) q1.y * d2.x - (long long) p1.x * d2.y;
            const double t = double( num ) / double( den );
            if( t <= 0.0 || t >= 1.0 )
                continue;
            const long long numU = (long long) q1.x * d1.y + (long long) p1.y * d1.x
                                 - (long long) q1.y * d1.x - (long long) p1.x * d1.y;
            const double u = double( numU ) / double( den );
            if( u <= 0.0 || u >= 1.0 )
                continue;

            const VECTOR2I pt( static_cast<int>( std::lround( p1.x + t * d1.x ) ),
                               static_cast<int>( std::lround( p1.y + t * d1.y ) ) );

            const JUNCTION_HELPERS::POINT_INFO info =
                JUNCTION_HELPERS::AnalyzePoint( screen_->Items(), pt, /*aBreakCrossings=*/true );
            if( info.isJunction && !info.hasExplicitJunctionDot )
                pts.push_back( pt );
        }
    }

    // 去重（GetNeededJunctions 与穿越扫描可能交叠）+ 补点
    std::sort( pts.begin(), pts.end(),
               []( const VECTOR2I& a, const VECTOR2I& b )
               {
                   return a.x < b.x || ( a.x == b.x && a.y < b.y );
               } );
    pts.erase( std::unique( pts.begin(), pts.end() ), pts.end() );

    for( const VECTOR2I& p : pts )
    {
        auto* j = new SCH_JUNCTION( p );
        StageUndo( InteractionUndo::Kind::ADD, j, screen_ );
        screen_->Append( j, false );
    }

    RefreshConnections();
}

// ── 删除（报告 F.3）─────────────────────────────────────────────────────────

void SchInteractionEngine::DeleteSelection()
{
    if( selection_.empty() )
        return;

    for( SCH_ITEM* it : selection_ )
    {
        if( !it )
            continue;

        SCH_ITEM* image = static_cast<SCH_ITEM*>( it->Clone() );
        StageUndo( InteractionUndo::Kind::REMOVE, it, screen_, image );
        screen_->Remove( it, false );               // 只摘树；对象由记录持有
    }

    selection_.clear();
    RefreshConnections();
}

// ── undo/redo（报告 E.3b 快照栈）────────────────────────────────────────────

void SchInteractionEngine::StageUndo( InteractionUndo::Kind aKind, SCH_ITEM* aItem,
                                      SCH_SCREEN* aScreen, SCH_ITEM* aImage )
{
    undoStack_.resize( undoPos_ );                  // 新编辑丢弃 redo 尾

    InteractionUndo rec;
    rec.kind = aKind;
    rec.item = aItem;
    rec.image = aImage;
    rec.screen = aScreen;
    undoStack_.push_back( std::move( rec ) );
    undoPos_ = undoStack_.size();
}

void SchInteractionEngine::StageUndoChain( const std::vector<SCH_ITEM*>& aItems, SCH_SCREEN* aScreen )
{
    undoStack_.resize( undoPos_ );

    InteractionUndo rec;
    rec.kind = InteractionUndo::Kind::CHAIN_ADD;
    rec.screen = aScreen;
    rec.chain = aItems;
    undoStack_.push_back( std::move( rec ) );
    undoPos_ = undoStack_.size();
}

bool SchInteractionEngine::Undo()
{
    if( !CanUndo() )
        return false;

    InteractionUndo& r = undoStack_[undoPos_ - 1];
    switch( r.kind )
    {
    case InteractionUndo::Kind::MODIFY:
        r.item->SwapItemData( r.image );            // 对调语义：item↔image 单槽切换
        r.screen->Update( r.item, false );
        break;
    case InteractionUndo::Kind::ADD:
        r.screen->Remove( r.item, false );
        break;
    case InteractionUndo::Kind::REMOVE:
        r.screen->Append( r.item, false );          // 原指针回树（RTree 非拥有）
        break;
    case InteractionUndo::Kind::CHAIN_ADD:
        for( SCH_ITEM* it : r.chain )
            r.screen->Remove( it, false );
        break;
    }

    --undoPos_;
    RefreshConnections();
    return true;
}

bool SchInteractionEngine::Redo()
{
    if( !CanRedo() )
        return false;

    InteractionUndo& r = undoStack_[undoPos_];
    switch( r.kind )
    {
    case InteractionUndo::Kind::MODIFY:
        r.item->SwapItemData( r.image );            // 单槽对调回到改后状态
        r.screen->Update( r.item, false );
        break;
    case InteractionUndo::Kind::ADD:
        r.screen->Append( r.item, false );
        break;
    case InteractionUndo::Kind::REMOVE:
        r.screen->Remove( r.item, false );
        break;
    case InteractionUndo::Kind::CHAIN_ADD:
        for( SCH_ITEM* it : r.chain )
            r.screen->Append( it, false );
        break;
    }

    ++undoPos_;
    RefreshConnections();
    return true;
}

void SchInteractionEngine::ClearUndo()
{
    undoStack_.clear();
    undoPos_ = 0;
}

// ── B3c 画线链 ───────────────────────────────────────────────────────────────

void SchInteractionEngine::CommitWireChain( const std::vector<VECTOR2I>& aVertices )
{
    if( aVertices.size() < 2 || !screen_ )
        return;

    std::deque<SCH_ITEM*> changed;
    std::vector<SCH_ITEM*> chainItems;

    for( size_t i = 1; i < aVertices.size(); ++i )
    {
        const VECTOR2I& a = aVertices[i - 1];
        const VECTOR2I& b = aVertices[i];
        if( a == b )
            continue;

        auto* line = new SCH_LINE( a, LAYER_WIRE );
        line->SetStartPoint( a );
        line->SetEndPoint( b );
        screen_->Append( line, false );
        changed.push_back( line );
        chainItems.push_back( line );
    }

    if( chainItems.empty() )
        return;

    // 整条一次撤销（KiCad Push 仅在 finish；B3c 报告 H）
    StageUndoChain( chainItems, screen_ );
    CommitEdits( changed );   // 链末一次：Update×N + GetNeededJunctions 补点 + Recalculate
}

// ── M1a G5：pin-到-pin 语义连接（v1：L 型路由 + 单条 CHAIN_ADD undo）────────────

namespace
{

// "R2.1" → (refdes, pin)
bool parse_endpoint( const wxString& aEndpoint, wxString* aRefdes, wxString* aPin )
{
    const int dot = aEndpoint.Find( '.' );
    if( dot <= 0 || dot + 1 >= (int) aEndpoint.Len() )
        return false;
    *aRefdes = aEndpoint.Left( dot );
    *aPin = aEndpoint.Mid( dot + 1 );
    return true;
}

// 按 refdes 找符号；pin 号 → 库引脚物理坐标；未命中返回 false
bool resolve_pin( SCH_SCREEN* aScreen, SCH_SHEET_PATH* aPath, const wxString& aRefdes,
                  const wxString& aPinNumber, VECTOR2I* aPhys, wxString* aError )
{
    for( SCH_ITEM* it : aScreen->Items() )
    {
        if( it->Type() != SCH_SYMBOL_T )
            continue;
        SCH_SYMBOL* sym = static_cast<SCH_SYMBOL*>( it );
        if( sym->GetRef( aPath, false ) != aRefdes )
            continue;
        for( SCH_PIN* lp : sym->GetLibPins() )
        {
            if( lp->GetNumber() != aPinNumber )
                continue;
            *aPhys = sym->GetPinPhysicalPosition( lp );
            return true;
        }
        if( aError )
            *aError = wxString::Format( "pin %s not found on %s", aPinNumber, aRefdes );
        return false;
    }
    if( aError )
        *aError = wxString::Format( "component %s not found", aRefdes );
    return false;
}

} // namespace

bool SchInteractionEngine::ConnectPins( const std::vector<std::pair<wxString, wxString>>& aEndpoints,
                                        wxString* aError )
{
    if( !screen_ )
        return false;

    for( const auto& [a, b] : aEndpoints )
    {
        wxString r1, p1s, r2, p2s;
        if( !parse_endpoint( a, &r1, &p1s ) || !parse_endpoint( b, &r2, &p2s ) )
        {
            if( aError )
                *aError = "endpoint format: refdes.pin (e.g. R2.1)";
            return false;
        }
        VECTOR2I p1, p2;
        if( !resolve_pin( screen_, &path_, r1, p1s, &p1, aError ) )
            return false;
        if( !resolve_pin( screen_, &path_, r2, p2s, &p2, aError ) )
            return false;

        if( p1 == p2 )
        {
            if( aError )
                *aError = wxString::Format( "%s and %s are the same point", a, b );
            return false;
        }

        // v1 路由：同轴直线；否则水平优先 L 型（垂直优先候选留 M1 打磨）
        std::vector<VECTOR2I> vertices;
        if( p1.x == p2.x || p1.y == p2.y )
        {
            vertices = { p1, p2 };
        }
        else
        {
            vertices = { p1, VECTOR2I( p2.x, p1.y ), p2 };
        }
        CommitWireChain( vertices );
    }
    return true;
}

// ── 放置（B3b）──────────────────────────────────────────────────────────────

void SchInteractionEngine::EnsureBuiltinLibrary()
{
    if( !libBuilt_ )
        BuildBuiltinLibrary();
}

void SchInteractionEngine::BuildBuiltinLibrary()
{
    // 内置库：R（电阻）/C（电容）/LED。每模板建一次；每次 new SCH_SYMBOL(*tpl,…)
    // 经 ctor 内 Flatten() 深拷贝（报告 G.3 裁决：实例互不共享、模板保持纯净）。
    auto add = [&]( const char* aName, const char* aPrefix, const char* aValue )
    {
        const wxString key = wxString::Format( "%s:%s", aName, aName );   // R:R / C:C / LED:LED
        auto tpl = std::make_unique<LIB_SYMBOL>( aName );

        tpl->GetReferenceField().SetText( aPrefix );
        tpl->GetValueField().SetText( aValue );

        // 本体矩形（B3c 演示系统对齐：pin 位/体高按 50mil 网格（12700IU）定型）
        auto* body = new SCH_SHAPE( SHAPE_T::RECTANGLE, LAYER_DEVICE );
        body->SetStart( VECTOR2I( -3810, 8890 ) );
        body->SetEnd( VECTOR2I( 3810, -8890 ) );
        tpl->AddDrawItem( body );

        // 两个竖直引脚（本地坐标，网格对齐：±12700）；库内局部坐标系，放置后世界坐标经 transform+m_pos
        for( int i = 0; i < 2; ++i )
        {
            auto* pin = new SCH_PIN( tpl.get() );
            pin->SetPosition( VECTOR2I( 0, i == 0 ? 12700 : -12700 ) );
            pin->SetNumber( wxString::Format( "%d", i + 1 ) );
            pin->SetName( wxString::Format( "P%d", i + 1 ) );
            tpl->AddDrawItem( pin );
        }

        libPrefix_[ key.ToStdString() ] = aPrefix;
        libTemplates_.emplace_back( key, std::move( tpl ) );
    };

    add( "R",   "R", "10k" );
    add( "C",   "C", "100n" );
    add( "LED", "D", "LED" );
    libBuilt_ = true;
}

void SchInteractionEngine::ListBuiltinSymbols( std::vector<wxString>& aNamesOut ) const
{
    aNamesOut.clear();
    for( const auto& entry : libTemplates_ )
        aNamesOut.emplace_back( entry.first );
}

SCH_SYMBOL* SchInteractionEngine::CreateSymbol( const wxString& aLibName, const VECTOR2I& aPosIU )
{
    EnsureBuiltinLibrary();

    for( const auto& entry : libTemplates_ )
    {
        if( entry.first == aLibName )
        {
            // 键 = category:name → LIB_ID(category, name)，文件 lib_id 原样保留键。
            const wxString cat = aLibName.BeforeFirst( ':' );
            const wxString nm  = aLibName.AfterFirst( ':' );
            auto* sym = new SCH_SYMBOL( *entry.second, LIB_ID( cat, nm ), &path_, 1 );
            sym->SetPosition( aPosIU );
            return sym;
        }
    }
    const wxString tail = aLibName.Find( ':' ) == wxNOT_FOUND ? aLibName : aLibName.AfterLast( ':' );
    const LIB_SYMBOL* fallback = nullptr;
    for( const auto& entry : libTemplates_ )
    {
        if( entry.first.AfterLast( ':' ) == tail )
        {
            if( fallback != nullptr )
                return nullptr;   // 同名多键 → 歧义，不归一
            fallback = entry.second.get();
        }
    }
    if( fallback != nullptr )
    {
        auto* sym = new SCH_SYMBOL( *fallback, LIB_ID( wxEmptyString, tail ), &path_, 1 );
        sym->SetPosition( aPosIU );
        return sym;
    }
    return nullptr;
}

const LIB_SYMBOL* SchInteractionEngine::FindBuiltinLibSymbol( const wxString& aLibName ) const
{
    for( const auto& entry : libTemplates_ )
        if( entry.first == aLibName )
            return entry.second.get();
    return nullptr;
}

void SchInteractionEngine::PlaceSymbol( SCH_SYMBOL* aSymbol )
{
    if( !aSymbol || !screen_ )
        return;

    EnsureBuiltinLibrary();

    // 位号分配（G.6）：REFDES_TRACKER 占号；undo 不回收号（P6 裁决）
    wxString libName = aSymbol->GetLibId().GetLibItemName();
    auto    it       = libPrefix_.find( libName.ToStdString() );
    wxString prefix  = ( it != libPrefix_.end() )
                           ? wxString::FromUTF8( it->second.c_str() )
                           : libName;

    // 注意：上游 10.0.6 的 REFDES_TRACKER::GetNextRefDes（简单版）头声明但无定义
    // （codegraph 全库核实：唯一使用面是 GetNextRefDesForUnits@sch_reference_list.cpp）——
    // 故用公开原语 Contains/Insert 实现同语义（最小未用号+占号，= findNextAvailable 行为），
    // 全部落在真 tracker 状态上（含 reuse 语义由 Insert 拒重复保证）。
    std::string prefixStr = prefix.ToStdString();
    int         num       = 1;
    while( tracker_->Contains( prefixStr + std::to_string( num ) ) )
        ++num;
    tracker_->Insert( prefixStr + std::to_string( num ) );

    aSymbol->SetRef( &path_, wxString::Format( "%s%d", prefix, num ) );

    StageUndo( InteractionUndo::Kind::ADD, aSymbol, screen_ );
    screen_->Append( aSymbol, false );
    RefreshConnections();
}

// ── 文档装配辅助（K6 public）──────────────────────────────────────────────

void SchInteractionEngine::ClearDocument()
{
    if( !screen_ )
        return;

    ClearUndo();                 // 先清 undo：记录可能持有 item 指针
    selection_.clear();

    std::vector<SCH_ITEM*> items;
    for( SCH_ITEM* it : screen_->Items() )
        items.push_back( it );
    for( SCH_ITEM* it : items )
    {
        screen_->Remove( it, false );
        delete it;               // 无记录引用（undo 已清）→ 本文件装载路径所有
    }
    RefreshConnections();
}

void SchInteractionEngine::RefreshConnections()
{
    if( !screen_ )
        return;

    CONNECTION_GRAPH* g = sch_.ConnectionGraph();
    g->Recalculate( sch_.Hierarchy(), true, nullptr, nullptr );
}

void SchInteractionEngine::ScanRefDes()
{
    // 装载后一次扫描：已标注符号登记进 tracker（保持单调位号）
    for( SCH_ITEM* it : screen_->Items() )
    {
        if( it->Type() != SCH_SYMBOL_T )
            continue;

        SCH_SYMBOL* sym = static_cast<SCH_SYMBOL*>( it );
        wxString    ref = sym->GetRef( &path_, false );
        if( ref.IsEmpty() || ref.Contains( '?' ) )
            continue;

        bool numeric = true;
        for( size_t i = 1; numeric && i < ref.Len(); ++i )
            numeric = ( ref[i] >= '0' && ref[i] <= '9' );
        if( !numeric )
            continue;

        tracker_->Insert( ref.ToStdString() );
    }
}

// ── M1b：外部库模板注册 ─────────────────────────────────────────────────────

bool SchInteractionEngine::RegisterLibTemplate( std::unique_ptr<LIB_SYMBOL> aTpl,
                                                const wxString& aRefPrefix,
                                                const wxString& aLibId )
{
    if( !aTpl )
        return false;
    const wxString key = aLibId.empty() ? aTpl->GetName() : aLibId;
    for( const auto& entry : libTemplates_ )
        if( entry.first == key )
            return false;   // 同键重名（内置或已注册）不覆盖
    libPrefix_[ key.ToStdString() ] = aRefPrefix.ToUTF8().data();
    libTemplates_.emplace_back( key, std::move( aTpl ) );
    return true;
}

void SchInteractionEngine::UnregisterLibTemplate( const wxString& aLibId )
{
    for( auto it = libTemplates_.begin(); it != libTemplates_.end(); ++it )
        if( it->first == aLibId )
        {
            libTemplates_.erase( it );
            libPrefix_.erase( aLibId.ToStdString() );
            return;
        }
}

wxString SchInteractionEngine::CanonicalLibKey( const wxString& aLiteral,
                                                const wxString& aItemName ) const
{
    for( const auto& entry : libTemplates_ )
        if( entry.first == aLiteral )
            return aLiteral;
    const wxString* hit = nullptr;
    for( const auto& entry : libTemplates_ )
    {
        if( entry.first.AfterLast( ':' ) != aItemName )
            continue;
        if( hit != nullptr && *hit != entry.first )
            return aLiteral;   // 同名多键 → 歧义，不归一
        hit = &entry.first;
    }
    return hit != nullptr ? *hit : aLiteral;
}

void SchInteractionEngine::ListLibrarySymbols( std::vector<wxString>& aNamesOut ) const
{
    for( const auto& entry : libTemplates_ )
        aNamesOut.push_back( entry.first );
}

size_t SchInteractionEngine::LibSymbolPinCount( const wxString& aLibName ) const
{
    for( const auto& entry : libTemplates_ )
        if( entry.first == aLibName )
            return entry.second->GetPins().size();
    return 0;
}

} // namespace cicada::kicad_geometry
