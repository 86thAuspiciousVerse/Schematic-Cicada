// scene_json.cpp — M1a：引擎 → /scene JSON 序列化
// 骨架语义照搬 cicada_document_bridge.cpp collect_engine（遍历/filter）；几何取 IU 原值；
// pins 双端点：连接点 = SCH_SYMBOL::GetPinPhysicalPosition（现成），体内端 = transform(GetPinRoot)+pos
//（kicad_scene_adapter.cpp:153,195 同款公式）；net 从引擎连通图（RefreshConnections 已建）。
#include "scene_json.h"

#include "json_util.h"

#include <kicad_bridge/sch_interaction_engine.h>

#include <sch_line.h>
#include <sch_shape.h>
#include <sch_label.h>
#include <sch_junction.h>
#include <sch_no_connect.h>
#include <sch_symbol.h>
#include <lib_symbol.h>
#include <connection_graph.h>
#include <sch_connection.h>

#include <sstream>

namespace cicada::editor::service
{

namespace
{

void emit_int( std::ostringstream& o, long long v )
{
    o << v;
}

// 符号旋转（与保存端同 switch；镜像位 v1 不暴露，见 02 §3）
int rotation_deg( const SCH_SYMBOL* aSym )
{
    switch( aSym->GetOrientation() )
    {
    case SYM_ORIENT_90:  return 90;
    case SYM_ORIENT_180: return 180;
    case SYM_ORIENT_270: return 270;
    default:             return 0;
    }
}

// lib 图形 RECTANGLE 世界 bbox（多矩形并集；无则 {0,0}）——世界化 = transform(local)+pos
void emit_body( std::ostringstream& o, const SCH_SYMBOL* aSym )
{
    const TRANSFORM&   t   = aSym->GetTransform();
    const VECTOR2I     pos = aSym->GetPosition();
    auto world = [&]( const VECTOR2I& p ) { return t.TransformCoordinate( p ) + pos; };

    bool     has = false;
    VECTOR2I minV, maxV;
    for( const SCH_ITEM& it : aSym->GetLibSymbolRef()->GetDrawItems() )
    {
        if( it.Type() != SCH_SHAPE_T )
            continue;
        const SCH_SHAPE* sh = static_cast<const SCH_SHAPE*>( &it );
        if( sh->GetShape() != SHAPE_T::RECTANGLE )
            continue;
        const VECTOR2I a = world( sh->GetStart() );
        const VECTOR2I b = world( sh->GetEnd() );
        const VECTOR2I lo( std::min( a.x, b.x ), std::min( a.y, b.y ) );
        const VECTOR2I hi( std::max( a.x, b.x ), std::max( a.y, b.y ) );
        if( !has )
        {
            minV = lo;
            maxV = hi;
            has = true;
        }
        else
        {
            minV.x = std::min( minV.x, lo.x );
            minV.y = std::min( minV.y, lo.y );
            maxV.x = std::max( maxV.x, hi.x );
            maxV.y = std::max( maxV.y, hi.y );
        }
    }
    o << "{\"rect\":{\"w\":";
    emit_int( o, has ? maxV.x - minV.x : 0 );
    o << ",\"h\":";
    emit_int( o, has ? maxV.y - minV.y : 0 );
    o << "}}";
}

} // namespace

std::string SceneJson( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                       const std::string& aFilePath, const std::string& aVersion )
{
    std::ostringstream o;
    o << "{\"file\":\"" << json_escape( aFilePath ) << "\","
      << "\"hash\":\"" << file_sha256( aFilePath ) << "\","
      << "\"version\":\"" << json_escape( aVersion ) << "\",";

    SCH_SCREEN*     screen = aEngine.screen();
    SCH_SHEET_PATH& path   = aEngine.sheetPath();

    // components / wires / junctions / labels / no_connects 五集合
    std::ostringstream comps, wires, juncs, labels, ncs;
    bool firstComp = true, firstWire = true, firstJunc = true, firstLabel = true, firstNc = true;

    CONNECTION_GRAPH* graph = screen ? aEngine.schematic().ConnectionGraph() : nullptr;

    for( SCH_ITEM* it : screen->Items() )
    {
        switch( it->Type() )
        {
        case SCH_LINE_T:
        {
            const auto* line = static_cast<const SCH_LINE*>( it );
            if( line->GetLayer() != LAYER_WIRE )
                break;
            const VECTOR2I s0 = line->GetStartPoint(), e0 = line->GetEndPoint();
            if( s0 == e0 )
                break;
            std::string net;
            if( graph )
            {
                if( CONNECTION_SUBGRAPH* sg = graph->GetSubgraphForItem( const_cast<SCH_ITEM*>( static_cast<const SCH_ITEM*>( line ) ) ) )
                    net = sg->GetNetName().ToUTF8().data();
            }
            if( !firstWire )
                wires << ",";
            firstWire = false;
            wires << "{\"uuid\":\"" << it->m_Uuid.AsString().ToUTF8().data() << "\","
                  << "\"points\":[[" ;
            emit_int( wires, s0.x ); wires << ","; emit_int( wires, s0.y ); wires << "],[";
            emit_int( wires, e0.x ); wires << ","; emit_int( wires, e0.y ); wires << "]],"
                  << "\"net\":\"" << json_escape( net ) << "\"}";
            break;
        }
        case SCH_JUNCTION_T:
        {
            const auto* j = static_cast<const SCH_JUNCTION*>( it );
            if( !firstJunc )
                juncs << ",";
            firstJunc = false;
            juncs << "{\"x\":";
            emit_int( juncs, j->GetPosition().x );
            juncs << ",\"y\":";
            emit_int( juncs, j->GetPosition().y );
            juncs << "}";
            break;
        }
        case SCH_LABEL_T:
        case SCH_GLOBAL_LABEL_T:
        case SCH_HIER_LABEL_T:
        {
            const auto* label = static_cast<const SCH_LABEL_BASE*>( it );
            if( !firstLabel )
                labels << ",";
            firstLabel = false;
            labels << "{\"text\":\"" << json_escape( label->GetText().ToUTF8().data() ) << "\",\"x\":";
            emit_int( labels, label->GetPosition().x );
            labels << ",\"y\":";
            emit_int( labels, label->GetPosition().y );
            labels << "}";
            break;
        }
        case SCH_NO_CONNECT_T:
        {
            const auto* n = static_cast<const SCH_NO_CONNECT*>( it );
            if( !firstNc )
                ncs << ",";
            firstNc = false;
            ncs << "{\"x\":";
            emit_int( ncs, n->GetPosition().x );
            ncs << ",\"y\":";
            emit_int( ncs, n->GetPosition().y );
            ncs << "}";
            break;
        }
        case SCH_SYMBOL_T:
        {
            const auto* sym = static_cast<const SCH_SYMBOL*>( it );
            const wxString itemName = sym->GetLibId().GetLibItemName();
            // 键模型唯一权威（docs/09 §1；与保存端同规则）：字面命中 → name-only 唯一回退
            const wxString canonicalWx = aEngine.CanonicalLibKey(
                wxString::FromUTF8( sym->GetLibId().Format().c_str() ), itemName );
            const std::string libId( canonicalWx.ToUTF8().data() );

            if( !firstComp )
                comps << ",";
            firstComp = false;

            comps << "{\"refdes\":\"" << json_escape( sym->GetRef( &path, false ).ToUTF8().data() ) << "\","
                  << "\"libId\":\"" << json_escape( libId ) << "\","
                  << "\"name\":\"" << json_escape( itemName.ToUTF8().data() ) << "\",\"x\":";
            emit_int( comps, sym->GetPosition().x );
            comps << ",\"y\":";
            emit_int( comps, sym->GetPosition().y );
            comps << ",\"rotation\":" << rotation_deg( sym ) << ",\"value\":\""
                  << json_escape( sym->GetField( FIELD_T::VALUE )
                                      ? sym->GetField( FIELD_T::VALUE )->GetText().ToUTF8().data()
                                      : "" )
                  << "\",\"fields\":{";
            bool firstField = true;
            for( const auto& f : sym->GetFields() )
            {
                const std::string fname( f.GetName().ToUTF8().data() );
                if( fname == "Reference" || fname == "Value" )
                    continue;
                if( !firstField )
                    comps << ",";
                firstField = false;
                comps << "\"" << json_escape( fname ) << "\":\""
                      << json_escape( f.GetText().ToUTF8().data() ) << "\"";
            }
            comps << "},\"pins\":[";
            bool firstPin = true;
            const TRANSFORM& t   = sym->GetTransform();
            const VECTOR2I   pos = sym->GetPosition();
            for( SCH_PIN* lp : sym->GetLibPins() )
            {
                const VECTOR2I phys = sym->GetPinPhysicalPosition( lp );
                const VECTOR2I core = t.TransformCoordinate( lp->GetPinRoot() ) + pos;
                if( !firstPin )
                    comps << ",";
                firstPin = false;
                comps << "{\"number\":\"" << json_escape( lp->GetNumber().ToUTF8().data() ) << "\","
                      << "\"name\":\"" << json_escape( lp->GetName().ToUTF8().data() ) << "\",\"x\":";
                emit_int( comps, phys.x );
                comps << ",\"y\":";
                emit_int( comps, phys.y );
                comps << ",\"ix\":";
                emit_int( comps, core.x );
                comps << ",\"iy\":";
                emit_int( comps, core.y );
                comps << "}";
            }
            comps << "],\"body\":";
            emit_body( comps, sym );
            comps << "}";
            break;
        }
        default:
            break;
        }
    }

    o << "\"components\":[" << comps.str() << "],"
      << "\"wires\":[" << wires.str() << "],"
      << "\"junctions\":[" << juncs.str() << "],"
      << "\"labels\":[" << labels.str() << "],"
      << "\"no_connects\":[" << ncs.str() << "]}";
    return o.str();
}

} // namespace cicada::editor::service
