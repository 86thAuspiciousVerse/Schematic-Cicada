// kicad_scene_adapter.h — 把真实 KiCad 几何（SchGeometryBatch 输出）装配成
// CanvasScene 演示场景（视图坐标；文字字形由 STROKE_FONT 预计算为折线）。
// 供 shell 画布"第一次看见"真实 KiCad 渲染；后续 DSH 模型接入时复用同一转换。
// B3（引擎编辑）与渲染共用同一换算源（R11）：IuToView/ViewToIu 成对定义于此，
// 屏内任何 px↔IU 换算只允许走这两者；引擎侧一律 IU 域、不接触 px。
#pragma once

#include "render/canvas_scene.h"

#include <math/vector2d.h>
#include <cmath>

#include <vector>

namespace cicada::kicad_geometry
{
class SchInteractionEngine; // sch_interaction_engine.h（cpp 内包含）
} // namespace cicada::kicad_geometry

class SCH_ITEM;   // sch_item.h（全局命名空间；仅指针引用）
class SCH_SYMBOL; // sch_symbol.h

namespace cicada::editor::kicad_adapter
{

// 视图单位 / KiCad IU（1IU=0.01mm → 0.008px/IU ≈ 12.5px/mm）+ 平移锚点
constexpr double    kScale = 0.008;
constexpr CanvasPoint kPan{ 60.0, 60.0 };

inline CanvasPoint IuToView( const VECTOR2I& aP )
{
    return CanvasPoint{ aP.x * kScale + kPan.x, aP.y * kScale + kPan.y };
}

inline VECTOR2I ViewToIu( const CanvasPoint& aV )
{
    return VECTOR2I{ static_cast<int>( std::lround( ( aV.x - kPan.x ) / kScale ) ),
                     static_cast<int>( std::lround( ( aV.y - kPan.y ) / kScale ) ) };
}

// ── B3a-2 持久文档路径 ──────────────────────────────────────────────────────
// 演示内容注入引擎活文档（wire/dash/junction/label + 引擎内置库真形状 R 符号）；
// 末尾 Recalculate + 位号重扫。之后的一切编辑走引擎原子操作。
void SeedDemoScreen( cicada::kicad_geometry::SchInteractionEngine& aEngine );

// 从引擎文档全量重建画布场景（只读、无副作用；demo 规模 <1ms）。
// aSelection：选中集（指针比较；字段 TextRec.source=所属符号 → 随符号高亮）。
CanvasScene BuildScene( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                        const std::vector<SCH_ITEM*>* aSelection = nullptr );

// B3b：放置幽灵（未入屏符号）半调色绘制，附加到已有场景（面板在 builder 后调用）。
void AddGhostPrimitives( CanvasScene& aScene, SCH_SYMBOL& aSymbol );

// 内置演示页（一次性、无引擎；保留给 from_model 占位兼容路径，R10 纪律）：
// 实线 wire + 虚线 wire + 结点 + NET1 标签 + R 符号（REFERENCE/VALUE 字段）。
CanvasScene MakeDemoScene();

} // namespace cicada::editor::kicad_adapter
