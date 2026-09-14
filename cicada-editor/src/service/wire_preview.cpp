// wire_preview.cpp — 画线预览几何（来源见头文件注释）。公式与 KiCad 原版逐行一致：
// sch_line_wire_bus_tool.cpp:521-654 的 45° 分支（wx 壳 computeWireBreak 同构；
// wx 壳退役后本实现继续作为服务端权威存在）。
#include "wire_preview.h"

#include <kicad_bridge/snap_utils.h>

#include <sch_screen.h>

#include <cmath>
#include <cstdlib>

namespace cicada::editor::service
{

namespace
{

int ssign( int v ) { return v < 0 ? -1 : 1; }

} // namespace

WirePreviewOut ComputeWireBreak( const std::pair<int, int>& aAnchor,
                                 const std::pair<int, int>& aCursor,
                                 const std::optional<std::pair<int, int>>& aPrevDir,
                                 bool aPosture )
{
    const int dx = aCursor.first - aAnchor.first;
    const int dy = aCursor.second - aAnchor.second;
    const int xDir = dx > 0 ? 1 : -1;
    const int yDir = dy > 0 ? 1 : -1;

    bool preferV = aPosture && aPrevDir && aPrevDir->second != 0;
    bool preferH = aPosture && aPrevDir && aPrevDir->first != 0;

    std::pair<int, int> mid{ 0, 0 };
    auto breakVertical = [&]()
    {
        mid = !aPosture
                  ? std::pair<int, int>{ aAnchor.first, aCursor.second - yDir * std::abs( dx ) }
                  : std::pair<int, int>{ aCursor.first, aAnchor.second + yDir * std::abs( dx ) };
    };
    auto breakHorizontal = [&]()
    {
        mid = !aPosture
                  ? std::pair<int, int>{ aCursor.first - xDir * std::abs( dy ), aAnchor.second }
                  : std::pair<int, int>{ aAnchor.first + xDir * std::abs( dy ), aCursor.second };
    };

    if( preferV )
        breakVertical();
    else if( preferH )
        breakHorizontal();

    // 45° 形状校验：!posture 看符号错位；posture 看段超长 → 双偏好失效
    const int dmx = mid.first - aAnchor.first;
    const int dmy = mid.second - aAnchor.second;
    if( !aPosture && ( ssign( dmx ) != ssign( dx ) || ssign( dmy ) != ssign( dy ) ) )
    {
        preferV = preferH = false;
    }
    else if( aPosture && ( std::abs( dmx ) > std::abs( dx ) || std::abs( dmy ) > std::abs( dy ) ) )
    {
        preferV = preferH = false;
    }

    if( !preferH && !preferV )
    {
        if( std::abs( dx ) < std::abs( dy ) )
            breakVertical();
        else
            breakHorizontal();
    }

    return { mid, { aCursor.first, aCursor.second }, false };
}

WirePreviewOut SnapWireInput( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                              const WirePreviewIn& aInput )
{
    const VECTOR2I raw( aInput.cursor.first, aInput.cursor.second );
    auto sn = cicada::kicad_geometry::SnapEndpoints( aEngine.screen(), aEngine.sheetPath(),
                                                     raw, aInput.snapRadIU );
    const VECTOR2I cur = sn ? sn->pt : cicada::kicad_geometry::SnapGrid( raw );

    WirePreviewOut out = ComputeWireBreak( aInput.anchor,
                                           { cur.x, cur.y }, aInput.prevDir, aInput.posture );
    out.terminal = aEngine.screen()->IsTerminalPoint( cur, LAYER_WIRE );
    return out;
}

} // namespace cicada::editor::service
