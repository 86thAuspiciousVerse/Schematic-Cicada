// wire_preview.h — 画线预览（M1c 架构修正）：折角/吸附/终端判定全部回到引擎侧。
// 折角公式 = KiCad computeBreakPoint（sch_line_wire_bus_tool.cpp:521-654，LINE_MODE_45）；
// 吸附 = SnapEndpoints + SnapGrid（kicad_bridge/snap_utils.h；公式源自 GRID_HELPER）。
// 前端只发送指针输入（anchor/cursor/prevDir/posture + 屏幕换算的吸附半径），
// 渲染 {mid,end} 返回 —— 几何唯一权威在引擎（docs/01 §1 职责边界）。
#pragma once

#include <optional>
#include <utility>

#include <kicad_bridge/sch_interaction_engine.h>

namespace cicada::editor::service
{

struct WirePreviewIn
{
    std::pair<int, int>               anchor;
    std::pair<int, int>               cursor;
    std::optional<std::pair<int, int>> prevDir;
    bool                              posture = false;
    /// 吸附半径（IU）：前端按像素换算（max(acc, 12px/scale)），引擎不再碰 px。
    int                               snapRadIU = 12000;
};

struct WirePreviewOut
{
    std::pair<int, int> mid;
    std::pair<int, int> end;
    /// cursor 落在连接点（pin/线端点/junction/label）→ 冻结后自动结束画线。
    bool terminal = false;
};

/**
 * KiCad computeBreakPoint with LINE_MODE_45: chain A→mid→C. The first segment
 * keeps the movement along the dominant axis (or the posture direction), the
 * last segment is 45°; with no preference the tie-break is |dx| < |dy| →
 * vertical first. mid == C degenerates to a straight A→C.
 */
WirePreviewOut ComputeWireBreak( const std::pair<int, int>& aAnchor,
                                 const std::pair<int, int>& aCursor,
                                 const std::optional<std::pair<int, int>>& aPrevDir,
                                 bool aPosture );

/**
 * 服务侧全链路：cursor 吸附（SnapEndpoints 命中 → 端点，否则 SnapGrid）→
 * 折角 → terminal 判定。anchor 是已冻结链点（网格点），不再吸附。
 */
WirePreviewOut SnapWireInput( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                              const WirePreviewIn& aInput );

} // namespace cicada::editor::service
