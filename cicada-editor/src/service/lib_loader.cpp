// lib_loader.cpp — M1b：.kicad_sym 白名单解析器
// 顶层结构（KiCad 官方）：(kicad_symbol_lib (version …) (symbol "R" …) (symbol "LED_RGB" …))
// 符号体：property / (symbol "NAME_<unit>_<style>" (rectangle|polyline|circle | pin …))。
// 本解析器只提取：Reference 属性、图形（rect/polyline/circle）、pin（at/length/name/number）。
// 坐标：.kicad_sym 内为 mm（浮点）；转换为 IU（0.0001mm 整数 = G×100；0.01mm = 100 IU）。
#include "lib_loader.h"

#include <map>

#include <sexpr/sexpr_parser.h>

#include <sch_shape.h>
#include <sch_pin.h>
#include <eda_shape.h>

#include <cmath>
#include <fstream>
#include <sstream>

namespace cicada::editor::service
{

namespace
{

struct NodeGuard
{
    const SEXPR::SEXPR* node;
};

std::string node_head( const SEXPR::SEXPR& aNode )
{
    if( !aNode.IsList() || aNode.GetNumberOfChildren() == 0 )
        return {};
    const auto* head = aNode.GetChild( 0 );
    if( head == nullptr || !head->IsSymbol() )
        return {};
    return head->GetSymbol();
}

/** Canonical KiCad electrical type name → enum (order = ELECTRICAL_PINTYPE; pin_type.h). */
ELECTRICAL_PINTYPE electrical_type_from( const std::string& aName )
{
    static const char* const names[] = {
        "input", "output", "bidirectional", "tri_state", "passive", "free",
        "unspecified", "power_in", "power_out", "open_collector", "open_emitter", "no_connect",
    };
    for( int i = 0; i < static_cast<int>( sizeof( names ) / sizeof( names[0] ) ); ++i )
        if( aName == names[i] )
            return static_cast<ELECTRICAL_PINTYPE>( i );
    return ELECTRICAL_PINTYPE::PT_PASSIVE;
}

const SEXPR::SEXPR* child( const SEXPR::SEXPR& aNode, const std::string& aHead )
{
    if( !aNode.IsList() )
        return nullptr;
    for( size_t i = 1; i < aNode.GetNumberOfChildren(); ++i )
    {
        const auto* c = aNode.GetChild( i );
        if( c != nullptr && c->IsList() && node_head( *c ) == aHead )
            return c;
    }
    return nullptr;
}

std::vector<const SEXPR::SEXPR*> children( const SEXPR::SEXPR& aNode,
                                           const std::string& aHead )
{
    std::vector<const SEXPR::SEXPR*> out;
    if( !aNode.IsList() )
        return out;
    for( size_t i = 1; i < aNode.GetNumberOfChildren(); ++i )
    {
        const auto* c = aNode.GetChild( i );
        if( c != nullptr && c->IsList() && node_head( *c ) == aHead )
            out.push_back( c );
    }
    return out;
}

// mm → IU（0.0001mm 整数 = mm × 10000；0.01mm = 100 IU）
int mmToIu( double aMm )
{
    return static_cast<int>( std::llround( aMm * 10000.0 ) );
}

double atom_double( const SEXPR::SEXPR* aNode, size_t aIndex, double aDef = 0.0 )
{
    if( aNode == nullptr || aIndex >= aNode->GetNumberOfChildren() )
        return aDef;
    const auto* c = aNode->GetChild( aIndex );
    if( c == nullptr )
        return aDef;
    if( c->IsDouble() )
        return c->GetDouble();
    if( c->IsInteger() )
        return static_cast<double>( c->GetInteger() );
    return aDef;
}

std::string atom_string( const SEXPR::SEXPR* aNode, size_t aIndex )
{
    if( aNode == nullptr || aIndex >= aNode->GetNumberOfChildren() )
        return {};
    const auto* c = aNode->GetChild( aIndex );
    if( c == nullptr )
        return {};
    if( c->IsString() )
        return c->GetString();
    if( c->IsSymbol() )
        return c->GetSymbol();
    return {};
}

bool parse_property( const SEXPR::SEXPR& aItem, const std::string& aName, wxString* aOut )
{
    for( const auto* p : children( aItem, "property" ) )
    {
        if( atom_string( p, 1 ) == aName )   // (property "Name" "Value" (at …))
        {
            *aOut = wxString::FromUTF8( atom_string( p, 2 ).c_str() );
            return true;
        }
    }
    return false;
}

// Reference 前缀：字母部分（"R"→R；"IC"→IC）
wxString ref_prefix_of( const wxString& aReference )
{
    size_t i = 0;
    while( i < aReference.Len() && !( aReference[ i ] >= '0' && aReference[ i ] <= '9' ) )
        ++i;
    return aReference.Left( i );
}

bool parse_symbol( const SEXPR::SEXPR& aSymbol, LibItem& aOut, std::string* aError )
{
    const std::string name = atom_string( &aSymbol, 1 );
    if( name.empty() )
    {
        if( aError )
            *aError = "symbol without name";
        return false;
    }

    aOut.symbol = std::make_unique<LIB_SYMBOL>( wxString::FromUTF8( name.c_str() ) );

    // (power)：电源符号语义标志（官方 POWER 库据此命名全局网络）。此前被忽略 →
    // 引擎保存回写后 DSH 语义层与 KiCad netlist 都把 GND/+5V 退化成 Net-(…)（2026-09-08 实测）。
    if( !children( aSymbol, "power" ).empty() )
        aOut.symbol->SetPowerSymbolProp( true );

    for( const std::string propName : { "Reference", "Value", "Footprint", "Datasheet", "Description" } )
    {
        wxString value;
        if( parse_property( aSymbol, propName.c_str(), &value ) )
        {
            aOut.properties[ wxString::FromUTF8( propName.c_str() ) ] = value;
            if( propName == "Reference" )
                aOut.refPrefix = ref_prefix_of( value );
        }
    }

    // 单元/体样式子节点：NAME_0_1 …（M1 只取 unit 0 + body style 1 的图形与引脚）
    for( const auto* unit : children( aSymbol, "symbol" ) )
    {
        for( const auto* c : children( *unit, "rectangle" ) )
        {
            const auto* startNode = child( *c, "start" );
            const auto* endNode   = child( *c, "end" );
            if( !startNode || !endNode )
                continue;
            auto* shape = new SCH_SHAPE( SHAPE_T::RECTANGLE, LAYER_DEVICE );
            shape->SetStart( VECTOR2I( mmToIu( atom_double( startNode, 1, 0 ) ),
                                       mmToIu( atom_double( startNode, 2, 0 ) ) ) );
            shape->SetEnd( VECTOR2I( mmToIu( atom_double( endNode, 1, 0 ) ),
                                     mmToIu( atom_double( endNode, 2, 0 ) ) ) );
            aOut.symbol->AddDrawItem( shape );
        }
        for( const auto* c : children( *unit, "polyline" ) )
        {
            const auto* pts = child( *c, "pts" );
            if( !pts )
                continue;
            auto* shape = new SCH_SHAPE( SHAPE_T::POLY, LAYER_DEVICE );
            for( const auto* xy : children( *pts, "xy" ) )
                shape->AddPoint( VECTOR2I( mmToIu( atom_double( xy, 1, 0 ) ),
                                           mmToIu( atom_double( xy, 2, 0 ) ) ) );
            aOut.symbol->AddDrawItem( shape );
        }
        for( const auto* c : children( *unit, "circle" ) )
        {
            const auto* centerNode = child( *c, "center" );
            const auto* radiusNode = child( *c, "radius" );
            if( !centerNode || !radiusNode )
                continue;
            auto* shape = new SCH_SHAPE( SHAPE_T::CIRCLE, LAYER_DEVICE );
            shape->SetCenter( VECTOR2I( mmToIu( atom_double( centerNode, 1, 0 ) ),
                                        mmToIu( atom_double( centerNode, 2, 0 ) ) ) );
            shape->SetRadius( mmToIu( atom_double( radiusNode, 1, 0 ) ) );
            aOut.symbol->AddDrawItem( shape );
        }
        for( const auto* c : children( *unit, "pin" ) )
        {
            // (pin passive line (at X Y ANGLE) (length L) (name "N") (number "2"))
            const auto* atNode     = child( *c, "at" );
            const auto* lengthNode = child( *c, "length" );
            const std::string number = atom_string( c, 0 );
            if( number.empty() )
                continue;   // pin 名在子节点，number 才是第 0 子节点
            auto* pin = new SCH_PIN( aOut.symbol.get() );
            if( atNode )
            {
                // 与 runtime 的 G 取整同规则：库里的 0.635mm 半格值（-0.635 → -6350 IU = -63.5 G）
                // 必须落到整 G，否则同一个脚在两侧文本里差 0.01mm（实测 curated AMS1117 的 2 个脚）。
                const auto snapG = []( const int iu ) { return static_cast<int>( std::llround( iu / 100.0 ) ) * 100; };
                pin->SetPosition( VECTOR2I( snapG( mmToIu( atom_double( atNode, 1, 0 ) ) ),
                                            snapG( mmToIu( atom_double( atNode, 2, 0 ) ) ) ) );
                const int angle = static_cast<int>( atom_double( atNode, 3, 0 ) );
                pin->SetOrientation( static_cast<PIN_ORIENTATION>( ( ( angle + 360 ) / 90 ) % 4 ) );
            }
            if( lengthNode )
                pin->SetLength( mmToIu( atom_double( lengthNode, 1, 0 ) ) );
            const auto* nameNode = child( *c, "name" );
            if( nameNode )
                pin->SetName( wxString::FromUTF8( atom_string( nameNode, 1 ).c_str() ) );
            // (pin <electrical> <shape> …)：子节点 0 = 头 "pin"，电气类型在索引 1。
            pin->SetType( electrical_type_from( atom_string( c, 1 ) ) );
            const auto* numNode = child( *c, "number" );
            if( numNode )
                pin->SetNumber( wxString::FromUTF8( atom_string( numNode, 1 ).c_str() ) );
            aOut.symbol->AddDrawItem( pin );
        }
    }
    return true;
}

} // namespace

bool LoadKicadSymFile( const std::string& aPath, std::vector<LibItem>& aOutItems,
                       std::string* aError )
{
    std::ifstream input( aPath, std::ios::binary );
    if( !input )
    {
        if( aError )
            *aError = "cannot open " + aPath;
        return false;
    }
    std::ostringstream contents;
    contents << input.rdbuf();
    return ParseKicadSym( contents.str(), aOutItems, aError );
}

bool ParseKicadSym( const std::string& aText, std::vector<LibItem>& aOutItems,
                    std::string* aError )
{
    try
    {
        SEXPR::PARSER parser;
        auto          root = parser.Parse( aText );
        if( !root || !root->IsList() || node_head( *root ) != "kicad_symbol_lib" )
        {
            if( aError )
                *aError = "not a kicad_symbol_lib";
            return false;
        }
        for( const auto* sym : children( *root, "symbol" ) )
        {
            LibItem item;
            if( !parse_symbol( *sym, item, aError ) )
                return false;
            aOutItems.push_back( std::move( item ) );
        }
        return true;
    }
    catch( const std::exception& e )
    {
        if( aError )
            *aError = e.what();
        return false;
    }
}

} // namespace cicada::editor::service
