// lib_loader.h — M1b：.kicad_sym（精选/用户库）→ 引擎 LIB_SYMBOL
// 上游 KiCad 专用解析器在抽取树中被剪裁，本解析器基于通用 sexpr（src/vendor/sexpr）
// + 白名单子集：property(Reference)/rectangle/polyline/circle/pin。白名单外图形忽略。
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <wx/string.h>

#include <lib_symbol.h>

namespace cicada::editor::service
{

struct LibItem
{
    std::unique_ptr<LIB_SYMBOL> symbol;   // 引擎可装载的符号定义
    wxString                     refPrefix;  // Reference 字段的字母前缀（位号分配用）
    // 属性模板（M1b）：Reference/Value/Footprint/Datasheet/Description —— 官方五元字段，
    // 放置默认/导出 KiCad 用（键 = 属性名）。
    std::map<wxString, wxString> properties;
    wxString libId;   // 注册键 `category:name`（服务侧装载时填写；list/get 寻址用）
};

// 解析一个 .kicad_sym 文本（顶层 (kicad_symbol_lib ...)）。失败返回 false + aError。
bool ParseKicadSym( const std::string& aText, std::vector<LibItem>& aOutItems,
                    std::string* aError = nullptr );

// 解析一个 .kicad_sym 文件（读取 + ParseKicadSym）。
bool LoadKicadSymFile( const std::string& aPath, std::vector<LibItem>& aOutItems,
                       std::string* aError = nullptr );

} // namespace cicada::editor::service
