// scene_json.h — M1a：引擎活文档 → 02 §3 快照 JSON（全 IU 坐标；前端零计算纯渲染）
#pragma once

#include <string>

namespace cicada::kicad_geometry
{
class SchInteractionEngine;
}

namespace cicada::editor::service
{

// 引擎 → /scene JSON。aFilePath 用于读取当前磁盘内容哈希（hash == 版本守卫的 fileHash）；
// 快照几何全部来自引擎活文档（原子 IU），不解析文件。
// version：.cicada_sch 方言版本（默认 "20260803"；后续可由装载的基底模型透出）。
std::string SceneJson( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                       const std::string& aFilePath,
                       const std::string& aVersion = "20260803" );

} // namespace cicada::editor::service
