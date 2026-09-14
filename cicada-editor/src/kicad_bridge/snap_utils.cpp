// snap_utils.cpp — 网格/端点吸附实现（公式出处见头文件注释）
#include "snap_utils.h"

#include <sch_symbol.h>
#include <sch_pin.h>

#include <cmath>
#include <limits>

namespace cicada::kicad_geometry
{

VECTOR2I SnapGrid( const VECTOR2I& aPt, int aGrid, const VECTOR2I& aOffset )
{
    if( aGrid <= 0 )
        return aPt;

    // == GRID_HELPER::computeNearest（grid_helper.cpp:445-450）==
    return VECTOR2I( static_cast<int>( std::lround( double( aPt.x - aOffset.x ) / aGrid ) ) * aGrid
                         + aOffset.x,
                     static_cast<int>( std::lround( double( aPt.y - aOffset.y ) / aGrid ) ) * aGrid
                         + aOffset.y );
}

std::optional<SnapPoint> SnapEndpoints( SCH_SCREEN* aScreen, const SCH_SHEET_PATH& aPath,
                                        const VECTOR2I& aPt, int aRadius )
{
    std::optional<SnapPoint> best;
    double                   bestDist = std::numeric_limits<double>::max();

    auto consider = [&]( const VECTOR2I& pt, EDA_ITEM* owner, bool isPin )
    {
        double d = ( pt - aPt ).EuclideanNorm();
        // 同距离时 pin 优先（pin 常同时出现在 GetConnections 与符号遍历中）
        if( d <= aRadius && ( d < bestDist || ( d == bestDist && isPin && best && !best->isPin ) ) )
        {
            bestDist = d;
            best     = SnapPoint{ pt, owner, isPin };
        }
    };

    // 1) 全部连接点（wire 端点/junction/label/… 去重排序，真源）
    for( const VECTOR2I& pt : aScreen->GetConnections() )
    {
        // owner 用 GetItem 近似（连接点通常落在某 item 上；无则不设）
        SCH_ITEM* owner = aScreen->GetItem( pt, 0 );
        consider( pt, owner, false );
    }

    // 2) 符号引脚（pin 不是 rtree item → 遍历符号 + 世界坐标，真源）
    // B3a-2 修正（R-wx1）：必须用 **LIB pin**（GetLibPins）+ GetPinPhysicalPosition ——
    // 代理 pin（GetPins(&path) 返回）的 GetPosition() 已含 transform+m_pos（sch_pin.cpp:254-260），
    // 再经 GetPinPhysicalPosition = 双重变换（B1 的 (160000,52000) 来源）。屏 RTree 的 GetPin
    // 亦走 GetLibPins 单变换路径（sch_screen.cpp:776），语义一致。
    for( SCH_ITEM* item : aScreen->Items() )
    {
        if( SCH_SYMBOL* sym = dynamic_cast<SCH_SYMBOL*>( item ) )
        {
            for( SCH_PIN* pin : sym->GetLibPins() )
                consider( sym->GetPinPhysicalPosition( pin ), sym, true );
        }
    }

    return best;
}

} // namespace cicada::kicad_geometry
