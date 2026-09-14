// sch_geometry_batch.h — 切片 2 shell 侧几何批次（CICADA 自研，非 vendor）
// 喂 KiCad 模型 → 折线/文本记录（世界坐标 = KiCad IU，画布自管 zoom/pan 换算）。
// 与 src/render/canvas_scene.h 的 CanvasPrimitive 同构（此处按报告 ⑤ 契约）。
#pragma once

#include <sch_screen.h>
#include <sch_line.h>
#include <sch_label.h>
#include <sch_symbol.h>
#include <sch_text.h>
#include <sch_junction.h>
#include <sch_render_settings.h>
#include <stroke_params.h>
#include <gal/color4d.h>
#include <math/vector2d.h>
#include <geometry/eda_angle.h>

#include <functional>
#include <vector>

namespace cicada::kicad_geometry
{

struct StrokeRec
{
    int layer = 0;
    KIGFX::COLOR4D color;
    int width = 0;
    LINE_STYLE style = LINE_STYLE::SOLID;
    bool filled = false;
    std::vector<VECTOR2I> outline; // 世界坐标（IU）
    const SCH_ITEM* source = nullptr; // B3：产生本折线的实模型项（选中/命中映射）
};

struct TextRec
{
    int layer = 0;
    KIGFX::COLOR4D color;
    wxString text;
    VECTOR2I pos;
    VECTOR2I size;
    EDA_ANGLE angle;
    bool mirror = false;
    GR_TEXT_H_ALIGN_T halign = GR_TEXT_H_ALIGN_LEFT;
    GR_TEXT_V_ALIGN_T valign = GR_TEXT_V_ALIGN_CENTER;
    int penWidth = 0;
    bool italic = false;
    bool multiline = true;
    const SCH_ITEM* source = nullptr; // B3：字段→所属符号；标签→自身
};

// 单屏单遍：SCH_SCREEN 全项 → StrokeRec/TextRec 回调。
// 覆盖 v1 范围：SCH_LINE / SCH_LABEL 系 / SCH_TEXT 系 / SCH_JUNCTION / SCH_SYMBOL 字段
// （symbol 本体图形、SHEET 标题栏、ERC 标记为后置候选）。
class SchGeometryBatch
{
public:
    void EmitScreen( SCH_SCREEN* aScreen, const SCH_SHEET_PATH& aSheet,
                     SCH_RENDER_SETTINGS& aSettings,
                     const std::function<void( const StrokeRec& )>& aStroke,
                     const std::function<void( const TextRec& )>& aText ) const;

    // 缺省渲染设置（无文件构造 COLOR_SETTINGS 的绕行：壳层自填层色）。
    static SCH_RENDER_SETTINGS MakeDefaultSettings();

private:
    void emitItem( SCH_ITEM* aItem, SCH_RENDER_SETTINGS& aSettings,
                   const std::function<void( const StrokeRec& )>& aStroke,
                   const std::function<void( const TextRec& )>& aText ) const;
    void emitLine( SCH_LINE& aLine, SCH_RENDER_SETTINGS& aSettings,
                   const std::function<void( const StrokeRec& )>& aStroke ) const;
    void emitTextBase( EDA_TEXT& aText, int aLayer, SCH_RENDER_SETTINGS& aSettings,
                       const std::function<void( const TextRec& )>& aTextOut,
                       const SCH_ITEM* aSource = nullptr ) const;
    void emitSymbol( SCH_SYMBOL& aSymbol, SCH_RENDER_SETTINGS& aSettings,
                     const std::function<void( const TextRec& )>& aText ) const;
};

} // namespace cicada::kicad_geometry
