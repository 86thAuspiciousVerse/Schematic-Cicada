// shape_synth.cpp — M1e-1：形状块 → .kicad_sym 文本（docs/02 附录 A 确定性规则）
// 规则要点：同侧引脚等距+中心对称（±round(k*PITCH)，k=0,1,… 量化取整）；
// 引脚 at=最外连接点、length 向体内；角度指向体内（top=270/right=180/bottom=90/left=0）；
// 轮廓宽/高 = max(该向侧计数-1,0)*PITCH + 2*MARGIN，取偶；body 覆盖时须在合法范围。
// 所有坐标 IU（0.0001mm）；输出按 4 位小数毫米（= IU 精确回环，无量化漂移）。
#include "shape_synth.h"

#include <cmath>
#include <cstdio>
#include <map>
#include <set>

namespace cicada::editor::service
{

namespace
{

constexpr int PITCH   = 12700;   // 1.27mm
constexpr int PIN_LEN = 25400;   // 2.54mm
constexpr int MARGIN  = 12700;   // 1.27mm

const std::set<std::string> kElectricalSet = {
    "input", "output", "power_in", "power_out", "bidirectional", "passive", "unspecified"
};
const std::set<std::string> kSideSet = { "left", "right", "top", "bottom" };

bool is_electrical( const std::string& aValue )
{
    return kElectricalSet.find( aValue ) != kElectricalSet.end();
}

bool is_side( const std::string& aValue )
{
    return kSideSet.find( aValue ) != kSideSet.end();
}

// 该侧引脚沿轴等距对称：count 个相位 round(|center|*PITCH)（与 ai 侧 icPins 同量化规则）。
std::vector<int> symmetric_offsets( int aCount )
{
    std::vector<int> out;
    for( int i = 0; i < aCount; ++i )
    {
        const double center = i - ( aCount - 1 ) / 2.0;
        // 量化到 G（0.01mm）再乘回 IU：偶数的 center（±0.5、±1.5…）会落在半格上，若直接
        // 取整成 IU（6350、19050…）就与 runtime 缓存的 G（Math.round(iu/100)）差 0.01mm，
        // 同一根脚在 truth 文本与导出文本里就不一致（实测 36/48 脚，网表因此报 unconnected）。
        const double half = center < 0 ? -center : center;
        const int g = static_cast<int>( std::llround( half * ( PITCH / 100 ) ) );
        out.push_back( center < 0 ? -g * 100 : g * 100 );
    }
    return out;
}

// 四边均分（缺省 side）：perSide = ceil(n/4)，top→right→bottom→left 依序装满。
std::map<std::string, int> assign_sides( std::size_t aN )
{
    const int perSide = static_cast<int>( ( aN + 3 ) / 4 );
    int       n       = static_cast<int>( aN );
    int       t       = std::min( perSide, n );
    int       r       = std::min( perSide, n - t );
    int       b       = std::min( perSide, n - t - r );
    int       l       = n - t - r - b;
    return { { "top", t }, { "right", r }, { "bottom", b }, { "left", l } };
}

// IU → mm 字符串（4 位小数；%c 不使用科学计数）。
std::string iu_to_mm( int aIu )
{
    char buf[ 32 ];
    std::snprintf( buf, sizeof( buf ), "%.4f", aIu / 10000.0 );
    return buf;
}

std::string quote( const std::string& aValue )
{
    std::string out;
    for( char c : aValue )
    {
        if( c == '"' || c == '\\' )
            out.push_back( '\\' );
        out.push_back( c );
    }
    return out;
}

} // namespace

std::string ValidateShapeBlock( const ShapeBlock& aBlock )
{
    if( aBlock.name.empty() )
        return "name required";
    if( aBlock.name.size() > 32 )
        return "name too long (>32)";
    if( aBlock.refPrefix.size() > 8 )
        return "refPrefix too long (>8)";
    if( aBlock.pins.empty() )
        return "pins empty";
    if( aBlock.pins.size() > 256 )
        return "pins too many (>256)";
    std::set<std::string> numbers;
    for( const auto& pin : aBlock.pins )
    {
        if( pin.number.empty() )
            return "pin number empty";
        if( pin.number.size() > 16 )
            return "pin number too long";
        if( !numbers.insert( pin.number ).second )
            return "pin number duplicated: " + pin.number;
        if( !is_electrical( pin.electrical ) )
            return "pin " + pin.number + " invalid electrical '" + pin.electrical + "'";
        if( !pin.side.empty() && !is_side( pin.side ) )
            return "pin " + pin.number + " invalid side '" + pin.side + "'";
    }
    if( aBlock.bodyGiven )
    {
        if( aBlock.bodyW < 10000 || aBlock.bodyW > 1000000 || aBlock.bodyH < 10000
            || aBlock.bodyH > 1000000 )
            return "body out of range [10000, 1000000] IU";
    }
    return {};
}

bool SynthesizeSymbolText( const ShapeBlock& aBlock, std::string& aOutText,
                           std::string* aError )
{
    const std::string err = ValidateShapeBlock( aBlock );
    if( !err.empty() )
    {
        if( aError )
            *aError = err;
        return false;
    }

    // side 归组（未指定 → 四边均分）
    std::map<std::string, std::vector<const ShapeBlockPin*>> bySide;
    bool hasAssignedSide = false;
    for( const auto& pin : aBlock.pins )
        if( !pin.side.empty() )
            hasAssignedSide = true;

    if( !hasAssignedSide )
    {
        const auto   counts = assign_sides( aBlock.pins.size() );
        std::size_t  idx    = 0;
        // 四边均分顺序：top → right → bottom → left（物理号升序；显式序，勿依赖 map 字典序）
        for( const char* side : { "top", "right", "bottom", "left" } )
            for( int i = 0; i < counts.at( side ); ++i )
                bySide[ side ].push_back( &aBlock.pins[ idx++ ] );
    }
    else
    {
        for( const auto& pin : aBlock.pins )
            bySide[ pin.side.empty() ? "left" : pin.side ].push_back( &pin );
    }

    // 轮廓（未覆盖时按各向最大跨距计算）
    auto spanOf = []( const std::vector<const ShapeBlockPin*>& aPins ) -> int
    {
        if( aPins.empty() )
            return 0;
        return ( static_cast<int>( aPins.size() ) - 1 ) * PITCH;
    };
    int width = spanOf( bySide[ "top" ] ) > spanOf( bySide[ "bottom" ] )
                    ? spanOf( bySide[ "top" ] )
                    : spanOf( bySide[ "bottom" ] );
    int height = spanOf( bySide[ "left" ] ) > spanOf( bySide[ "right" ] )
                     ? spanOf( bySide[ "left" ] )
                     : spanOf( bySide[ "right" ] );
    width += 2 * MARGIN;
    height += 2 * MARGIN;
    if( aBlock.bodyGiven )
    {
        width  = aBlock.bodyW;
        height = aBlock.bodyH;
    }
    if( width % 2 != 0 )
        ++width;
    if( height % 2 != 0 )
        ++height;
    const int halfW = width / 2;
    const int halfH = height / 2;

    // 引脚下发点（连接点）按侧生成：at = 侧沿 + PIN_LEN，角度指向体内
    struct PinOut
    {
        int         x, y, angle;
        const ShapeBlockPin* pin;
    };
    std::vector<PinOut> pinOut;
    auto emitSide = [&]( const std::string& aSide, int aAngle, bool aHorizontal )
    {
        const auto& list = bySide[ aSide ];
        const auto offsets = symmetric_offsets( static_cast<int>( list.size() ) );
        for( std::size_t i = 0; i < list.size(); ++i )
        {
            const ShapeBlockPin* p = list[ i ];
            PinOut out;
            out.angle = aAngle;
            out.pin   = p;
            if( aHorizontal )
            {
                // top/bottom：引脚沿 X 分布，Y = ±(halfH + PIN_LEN)（连接点在体外侧）
                out.x = offsets[ i ];
                out.y = aSide == "top" ? halfH + PIN_LEN : -( halfH + PIN_LEN );
            }
            else
            {
                // left/right：引脚沿 Y 分布，X = ±(halfW + PIN_LEN)
                out.x = aSide == "right" ? halfW + PIN_LEN : -( halfW + PIN_LEN );
                out.y = offsets[ i ];
            }
            pinOut.push_back( out );
        }
    };
    emitSide( "top", 270, true );
    emitSide( "right", 180, false );
    emitSide( "bottom", 90, true );
    emitSide( "left", 0, false );
    const std::string refPrefix = aBlock.refPrefix.empty() ? "U" : aBlock.refPrefix;
    std::string out;
    out += "(kicad_symbol_lib (version 20251024) (generator \"cicada_synth\")\n";
    out += "  (symbol \"" + quote( aBlock.name ) + "\" (pin_names (offset 0.508))\n";
    out += "    (property \"Reference\" \"" + quote( refPrefix ) + "\" (at 0 0 0))\n";
    out += "    (property \"Value\" \"" + quote( aBlock.name ) + "\" (at 0 0 0))\n";
    out += "    (property \"Footprint\" \"\" (at 0 0 0))\n";
    out += "    (property \"Datasheet\" \"\" (at 0 0 0))\n";
    out += "    (property \"Description\" \"" + quote( aBlock.description ) + "\" (at 0 0 0))\n";
    out += "    (symbol \"" + quote( aBlock.name ) + "_0_1\"\n";
    out += "      (rectangle (start " + iu_to_mm( -halfW ) + " " + iu_to_mm( -halfH )
        + ") (end " + iu_to_mm( halfW ) + " " + iu_to_mm( halfH ) + ")))\n";
    out += "    (symbol \"" + quote( aBlock.name ) + "_1_1\"\n";
    for( const auto& po : pinOut )
    {
        out += "      (pin " + po.pin->electrical + " line (at " + iu_to_mm( po.x ) + " "
            + iu_to_mm( po.y ) + " " + std::to_string( po.angle ) + ") (length "
            + iu_to_mm( PIN_LEN ) + ") (name \"" + quote( po.pin->name ) + "\") (number \""
            + quote( po.pin->number ) + "\"))\n";
    }
    out += "    ))\n";
    out += ")\n";

    aOutText = std::move( out );
    return true;
}

} // namespace cicada::editor::service
