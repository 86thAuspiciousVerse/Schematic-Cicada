// shape_synth.h — M1e-1：形状块 → .kicad_sym 文本（确定性几何，docs/02 附录 A）
// AI（或 datasheet 知识）只给出引脚表/侧别/类型等语义；一切坐标由本模块按固定规则生成。
// 生成文本经 lib_loader::ParseKicadSym 回环解析（引擎装载唯一真值路径），
// 产出格式：K8+ 单符号（version 20251024 头；pin at/length 用 4 位小数毫米——0.0001mm 无漂移）。
#pragma once

#include <string>
#include <vector>

namespace cicada::editor::service
{

// ── 形状块（M1e-1，docs/02 §3 /lib/synthesize）─────────────────────────────

struct ShapeBlockPin
{
    std::string number;      // 引脚号（唯一、非空）
    std::string name;        // 引脚名（可为空）
    std::string electrical;  // 引擎枚举：input/output/power_in/power_out/bidirectional/passive/unspecified
    std::string side;        // ""（缺省=四边均分）| left | right | top | bottom
};

struct ShapeBlock
{
    std::string name;        // 符号名（如 AMS1117）；≤32 字符
    std::string refPrefix;   // Reference 前缀（缺省 "U"）
    std::string description; // Description 属性（可空）
    std::vector<ShapeBlockPin> pins;
    bool bodyGiven = false;  // body 宽高覆盖（IU）
    int  bodyW = 0;
    int  bodyH = 0;
};

// 校验（附录 A 校验顺序；返回空 = 通过，否则人类可读错误信息）。
std::string ValidateShapeBlock( const ShapeBlock& aBlock );

// 确定性合成 .kicad_sym 文本。校验失败 → false + aError。
bool SynthesizeSymbolText( const ShapeBlock& aBlock, std::string& aOutText,
                           std::string* aError = nullptr );

} // namespace cicada::editor::service
