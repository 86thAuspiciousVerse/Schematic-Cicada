// ops.h — M1a：/ops 高层映射（引擎原子 → 契约 8 op；fileHash 守卫与懒重载在服务层）
#pragma once

#include <string>

namespace cicada::kicad_geometry
{
class SchInteractionEngine;
}

namespace cicada::editor
{
struct SchematicModel;
}

namespace cicada::editor::service
{

// 应用一个 /ops 请求（单表达式 JSON 文本）。返回完整响应 JSON：
//   {"ok":true,"scene":{…},"undoable":bool,"redoable":bool}  或 {"error":{code,message}}
// 前置：aServicePath 为 .cicada_sch 绝对路径；aBase 为最近一次装载的基底模型。
// fileHash 不匹配（外部/AI 曾写入）→ {"error":{"code":"conflict",…}}。
std::string ApplyOps( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                      const cicada::editor::SchematicModel& aBase,
                      const std::string& aServicePath, const std::string& aRequestJson,
                      const std::string& aCurrentHash );

} // namespace cicada::editor::service
