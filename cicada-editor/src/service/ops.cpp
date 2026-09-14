// ops.cpp — M1a：/ops 高层映射实现
// JSON 字段提取为极简手写（契约字段固定，无外部依赖；风格照 old main.cpp json_*）。
#include "ops.h"

#include "json_util.h"
#include "scene_json.h"

#include <kicad_bridge/cicada_document_bridge.h>
#include <kicad_bridge/sch_interaction_engine.h>
#include <kicad_bridge/snap_utils.h>

#include <sch_symbol.h>
#include <sch_line.h>

#include <sstream>

namespace cicada::editor::service
{

namespace
{

// ── 极简 JSON 提取 ────────────────────────────────────────────────────────────

std::string field_str( const std::string& aJson, const std::string& aKey )
{
    const std::string needle = "\"" + aKey + "\"";
    const auto pos = aJson.find( needle );
    if( pos == std::string::npos )
        return {};
    const auto colon = aJson.find( ':', pos + needle.size() );
    if( colon == std::string::npos )
        return {};
    auto it = aJson.begin() + colon + 1;
    while( it != aJson.end() && ( *it == ' ' || *it == '\t' || *it == '\n' || *it == '\r' ) )
        ++it;
    if( it == aJson.end() || *it != '"' )
        return {};
    ++it;
    std::string out;
    while( it != aJson.end() && *it != '"' )
    {
        if( *it == '\\' && it + 1 != aJson.end() )
            ++it;
        out.push_back( *it );
        ++it;
    }
    return out;
}

long long field_int( const std::string& aJson, const std::string& aKey )
{
    const std::string needle = "\"" + aKey + "\"";
    const auto pos = aJson.find( needle );
    if( pos == std::string::npos )
        return 0;
    const auto colon = aJson.find( ':', pos + needle.size() );
    if( colon == std::string::npos )
        return 0;
    const char* p = aJson.c_str() + colon + 1;
    while( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' )
        ++p;
    return std::atoll( p );
}

// ["a.b","c.d"] → 端点对列表
bool parse_endpoint_pairs( const std::string& aJson, const std::string& aKey,
                           std::vector<std::pair<std::string, std::string>>& aOut )
{
    const std::string needle = "\"" + aKey + "\"";
    const auto pos = aJson.find( needle );
    if( pos == std::string::npos )
        return true;   // 字段可缺省
    const auto colon = aJson.find( ':', pos + needle.size() );
    const auto lb = aJson.find( '[', colon );
    if( lb == std::string::npos )
        return false;
    const auto rb = aJson.rfind( ']' );
    if( rb == std::string::npos || rb < lb )
        return false;

    for( size_t i = lb + 1; i < rb; ++i )
    {
        if( aJson[ i ] == '"' )
        {
            const auto j = aJson.find( '"', i + 1 );
            if( j == std::string::npos )
                return false;
            std::string first = aJson.substr( i + 1, j - i - 1 );
            const auto k = aJson.find( '"', j + 1 );
            if( k == std::string::npos )
                return false;
            const auto l = aJson.find( '"', k + 1 );
            if( l == std::string::npos )
                return false;
            std::string second = aJson.substr( k + 1, l - k - 1 );
            aOut.emplace_back( first, second );
            i = l;
        }
    }
    return true;
}

// ["R2","R3"] → 字符串数组
bool parse_string_array( const std::string& aJson, const std::string& aKey,
                         std::vector<std::string>& aOut )
{
    const std::string needle = "\"" + aKey + "\"";
    const auto pos = aJson.find( needle );
    if( pos == std::string::npos )
        return true;
    const auto colon = aJson.find( ':', pos + needle.size() );
    const auto lb = aJson.find( '[', colon );
    if( lb == std::string::npos )
        return false;
    const auto rb = aJson.find( ']', lb );
    if( rb == std::string::npos )
        return false;
    for( size_t i = lb + 1; i < rb; ++i )
    {
        if( aJson[ i ] == '"' )
        {
            const auto j = aJson.find( '"', i + 1 );
            if( j == std::string::npos )
                return false;
            aOut.push_back( aJson.substr( i + 1, j - i - 1 ) );
            i = j;
        }
    }
    return true;
}

// [[x,y],…] → 顶点数组（IU 整数）。
// 注意：外层 '[' 是数组本身——内层扫描必须从 outer+1 开始，否则第一个顶点被
// 解析成 (0, y)（inner 含前导 '['，atoll("[0")=0）——曾致提交导线起点恒 x=0。
bool parse_vertices( const std::string& aJson, const std::string& aKey,
                     std::vector<VECTOR2I>& aOut )
{
    const std::string needle = "\"" + aKey + "\"";
    const auto pos = aJson.find( needle );
    if( pos == std::string::npos )
        return true;
    const auto colon = aJson.find( ':', pos + needle.size() );
    const auto outer = aJson.find( '[', colon );
    if( outer == std::string::npos )
        return false;
    for( size_t i = outer + 1; i < aJson.size(); ++i )
    {
        if( aJson[ i ] != '[' )
            continue;
        const auto close = aJson.find( ']', i + 1 );
        if( close == std::string::npos )
            return false;
        // 内层 [x,y]：提取内容后手动拆两个整数
        const std::string inner = aJson.substr( i + 1, close - i - 1 );
        const auto comma = inner.find( ',' );
        if( comma == std::string::npos || comma == 0 )
            return false;
        aOut.emplace_back( std::atoll( inner.c_str() ),
                           std::atoll( inner.c_str() + comma + 1 ) );
        i = close + 1;
        // 下一个 '[' 直接找（跳过 "] , [" 噪声）
        continue;
    }
    return !aOut.empty();
}

// ── 辅助 ──────────────────────────────────────────────────────────────────────

// refdes → 符号（G3）
SCH_SYMBOL* resolve_refdes( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                            const std::string& aRefdes )
{
    SCH_SHEET_PATH& path = aEngine.sheetPath();
    for( SCH_ITEM* it : aEngine.screen()->Items() )
    {
        if( it->Type() != SCH_SYMBOL_T )
            continue;
        SCH_SYMBOL* sym = static_cast<SCH_SYMBOL*>( it );
        if( sym->GetRef( &path, false ).ToUTF8().data() == aRefdes )
            return sym;
    }
    return nullptr;
}

// uuid → 项（wires 删除引用，G4）
SCH_ITEM* resolve_uuid( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                        const std::string& aUuid )
{
    for( SCH_ITEM* it : aEngine.screen()->Items() )
        if( it->m_Uuid.AsString().ToUTF8().data() == aUuid )
            return it;
    return nullptr;
}

std::string err_json( const std::string& aCode, const std::string& aMessage )
{
    return "{\"error\":{\"code\":\"" + json_escape( aCode ) + "\",\"message\":\""
           + json_escape( aMessage ) + "\"}}";
}

} // namespace

std::string ApplyOps( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                      const cicada::editor::SchematicModel& aBase,
                      const std::string& aServicePath, const std::string& aRequestJson,
                      const std::string& aCurrentHash )
{
    // fileHash 守卫：请求必须与当前磁盘内容一致（版本冲突 → 409，前端重新同步）
    const std::string reqHash = field_str( aRequestJson, "fileHash" );
    if( !reqHash.empty() && reqHash != aCurrentHash )
        return err_json( "conflict", "schematic changed outside (AI write?); re-sync /scene" );

    const std::string op = field_str( aRequestJson, "op" );
    if( op.empty() )
        return err_json( "bad_request", "op required" );

    wxString wErr;
    bool ok = false;

    if( op == "place-symbol" )
    {
        const std::string libId = field_str( aRequestJson, "libId" );
        int x = static_cast<int>( field_int( aRequestJson, "x" ) );
        int y = static_cast<int>( field_int( aRequestJson, "y" ) );
        const int rotation = static_cast<int>( field_int( aRequestJson, "rotation" ) );

        // 放置网格对齐在引擎侧（几何唯一权威；前端只发指针 IU）。
        const VECTOR2I snapped = cicada::kicad_geometry::SnapGrid( VECTOR2I( x, y ) );
        x = snapped.x;
        y = snapped.y;

        // 键寻址（docs/09 §1）：整键交给引擎（精确键优先 → name-only 唯一回退）。传尾段会
        // 让精确键降级为 name-only，同名多键时误报 not_found。
        SCH_SYMBOL* sym = aEngine.CreateSymbol( wxString::FromUTF8( libId.c_str() ),
                                                VECTOR2I( x, y ) );
        if( !sym )
        {
            return err_json( "not_found",
                             "symbol '" + libId + "' is not in the loaded library" );
        }
        if( rotation != 0 )
            sym->SetOrientation( rotation == 90 ? SYM_ORIENT_90
                                : rotation == 180 ? SYM_ORIENT_180
                                : rotation == 270 ? SYM_ORIENT_270
                                                  : SYM_ORIENT_0 );
        const std::string value = field_str( aRequestJson, "value" );
        if( !value.empty() )
            if( SCH_FIELD* v = sym->GetField( FIELD_T::VALUE ) )
                v->SetText( wxString::FromUTF8( value.c_str() ) );
        aEngine.PlaceSymbol( sym );
        ok = true;
    }
    else if( op == "connect-pins" )
    {
        std::vector<std::pair<std::string, std::string>> pairs;
        if( !parse_endpoint_pairs( aRequestJson, "endpoints", pairs ) || pairs.empty() )
            return err_json( "bad_request", "endpoints required" );
        std::vector<std::pair<wxString, wxString>> wxPairs;
        for( const auto& [a, b] : pairs )
            wxPairs.emplace_back( wxString::FromUTF8( a.c_str() ), wxString::FromUTF8( b.c_str() ) );
        ok = aEngine.ConnectPins( wxPairs, &wErr );
    }
    else if( op == "draw-wire" )
    {
        std::vector<VECTOR2I> verts;
        if( !parse_vertices( aRequestJson, "points", verts ) || verts.size() < 2 )
            return err_json( "bad_request", "points (>=2) required" );
        aEngine.CommitWireChain( verts );
        ok = true;
    }
    else if( op == "move" )
    {
        std::vector<std::string> refdeses;
        if( !parse_string_array( aRequestJson, "refdeses", refdeses ) || refdeses.empty() )
            return err_json( "bad_request", "refdeses required" );
        const int dx = static_cast<int>( field_int( aRequestJson, "dx" ) );
        const int dy = static_cast<int>( field_int( aRequestJson, "dy" ) );
        std::vector<SCH_ITEM*> items;
        for( const auto& r : refdeses )
        {
            SCH_SYMBOL* sym = resolve_refdes( aEngine, r );
            if( !sym )
                return err_json( "not_found", "component " + r + " not found" );
            items.push_back( sym );
        }
        if( !items.empty() )
            aEngine.SetSelection( items );
        aEngine.MoveSelection( VECTOR2I( dx, dy ) );
        ok = true;
    }
    else if( op == "delete" )
    {
        std::vector<std::string> refdeses, wireUuids;
        parse_string_array( aRequestJson, "refdeses", refdeses );
        parse_string_array( aRequestJson, "wires", wireUuids );
        if( refdeses.empty() && wireUuids.empty() )
            return err_json( "bad_request", "refdeses or wires required" );
        std::vector<SCH_ITEM*> items;
        for( const auto& r : refdeses )
            if( SCH_SYMBOL* sym = resolve_refdes( aEngine, r ) )
                items.push_back( sym );
        for( const auto& u : wireUuids )
            if( SCH_ITEM* it = resolve_uuid( aEngine, u ) )
                items.push_back( it );
        if( !items.empty() )
            aEngine.SetSelection( items );
        aEngine.DeleteSelection();
        ok = true;
    }
    else if( op == "undo" )
    {
        ok = aEngine.Undo();
    }
    else if( op == "redo" )
    {
        ok = aEngine.Redo();
    }
    else if( op == "clear" )
    {
        aEngine.ClearDocument();
        ok = true;
    }
    else
    {
        return err_json( "bad_request", "unknown op: " + op );
    }

    if( !ok )
        return err_json( "bad_request", wErr.ToUTF8().data() );

    // 即时落盘（on_commit 语义）：引擎现态 → .cicada_sch；失败也必须如实报告
    if( !cicada::editor::cicada_doc::SaveCicadaSchIntoFile( aEngine, aBase,
                                                            wxString::FromUTF8( aServicePath.c_str() ),
                                                            &wErr ) )
        return err_json( "internal", "saveback failed: " + std::string( wErr.ToUTF8().data() ) );

    const std::string scene = SceneJson( aEngine, aServicePath );
    std::ostringstream o;
    o << "{\"ok\":true,\"scene\":" << scene << ",\"undoable\":"
      << ( aEngine.CanUndo() ? "true" : "false" ) << ",\"redoable\":"
      << ( aEngine.CanRedo() ? "true" : "false" ) << "}";
    return o.str();
}

} // namespace cicada::editor::service
