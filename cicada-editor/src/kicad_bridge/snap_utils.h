// snap_utils.h — 切片 3 B1 网格/端点吸附纯函数（CICADA 自研，公式源自 GRID_HELPER）
// 出处：kicad common/tool/grid_helper.cpp:445-450 computeNearest（6 行公式，全类排除仅取公式）
#pragma once

#include <sch_screen.h>
#include <sch_sheet_path.h>
#include <eda_item.h>
#include <math/vector2d.h>

#include <optional>

namespace cicada::kicad_geometry
{

// computeNearest 纯形式：p 对齐到 aGrid 网格（+aOffset 原点偏移）。
// 默认网格 = eeschema 常规 50mil（1mm=10000IU → 50mil=12700IU）。
VECTOR2I SnapGrid( const VECTOR2I& aPt, int aGrid = 12700, const VECTOR2I& aOffset = VECTOR2I( 0, 0 ) );

struct SnapPoint
{
    VECTOR2I    pt;
    EDA_ITEM*   owner = nullptr; // 命中项的持有者（pin 场景=符号；连接点场景=所在 item）
    bool        isPin = false;   // 吸附到引脚（pin 世界坐标）时为真
};

// 端点/引脚吸附：候选 = screen->GetConnections()（去重连接点）
// + 屏幕上每个符号 GetPins(&path) 的 GetPinPhysicalPosition。
// 返回 aRadius 内最近者；无命中返回 nullopt。
std::optional<SnapPoint> SnapEndpoints( SCH_SCREEN* aScreen, const SCH_SHEET_PATH& aPath,
                                        const VECTOR2I& aPt, int aRadius );

} // namespace cicada::kicad_geometry
