// cicada_document_bridge.h — K6：.cicada_sch（DSH 白名单方言）↔ KiCad 引擎文档
// DSH 侧唯一真相文件 = {cwd}/.cicada/schematic.cicada_sch（cicada-format 白名单方言）；
// 本桥用既有 CicadaScreenAdapter（K1 白名单解析，单测覆盖）读文本 → 旧 SchematicModel
// （坐标=centi-mm，1 单位=0.01mm）→ 引擎重建（IU：centi-mm×100 = 1mm=10000IU）。
// 反向：引擎收集 → K1 SchematicModel → SexprModelWriter（DSH 兼容方言）写回同一文件 →
// runtime watcher 感知 → user_edit changelog → agent 下次回合知道（K6 双向闭环）。
#pragma once

#include <wx/string.h>

namespace cicada::kicad_geometry
{
class SchInteractionEngine;
}

namespace cicada::editor
{
struct SchematicModel;
}

namespace cicada::editor::cicada_doc
{

// 清空并装载：成功返回 true；失败清空错误说明（解析失败不改动引擎文档）。
// aOutModel（可选）带回解析出的模型（lib_symbols/version/uuid/sheet_instances 等
// 原文件信息——写回时作为"基底"，保证未编辑部分原样保留且不丢数据）。
bool LoadCicadaSchIntoEngine( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                              const wxString& aPath, wxString* aError = nullptr,
                              cicada::editor::SchematicModel* aOutModel = nullptr );

// 引擎 → .cicada_sch：以 aBase 为基底（版本/库符号/uuid/sheet_instances 源），
// 遍历引擎当前全部可序列化条目收集为模型（坐标 IU→centi-mm），SexprModelWriter 写出。
// 引擎中新增符号（画布放置）若基底无其 lib_symbols → 用引擎内置库模板合成体
// （cicada:<name>，几何=模板真值）。成功返回 true；失败填 aError 且不写文件。
bool SaveCicadaSchIntoFile( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                            const cicada::editor::SchematicModel& aBase,
                            const wxString& aPath, wxString* aError = nullptr );

// 引擎 → 标准 KiCad 10 .kicad_sch（M1 验收"导出 KiCad 可打开"；不改唯一真相文件）。
// 复用 collect_engine 的键归一/库体（模板单权威），仅序列化方言不同：
// 根 = kicad_sch v20250610 + lib_symbols + wires/junctions/labels/no_connects/symbols +
// sheet_instances；坐标 G（0.01mm）→ mm 两位小数；缺 uuid 生成 v4 随机。
bool ExportKicadSchIntoFile( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                             const cicada::editor::SchematicModel& aBase,
                             const wxString& aPath, wxString* aError = nullptr );

} // namespace cicada::editor::cicada_doc
