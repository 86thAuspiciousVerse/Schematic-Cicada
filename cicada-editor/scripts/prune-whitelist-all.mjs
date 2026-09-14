#!/usr/bin/env node
/**
 * 幂等全量白名单裁剪器（切片 1）。
 * 对 vendor/kicad 树重放所有已知的"白名单排除区"裁剪：
 *   - 删除函数块（api/protobuf 序列化、UI 消息面板、Plot、帮助弹窗、嵌入字体、
 *     sim/工具/库管理段等）
 *   - 删除 include 行（UI/仿真/工具/netlist/字体/curl/glad/API 等排除区头）
 * 重放后 vendor 树回到"切片 1 已裁剪状态"（wave1 git archive 覆盖后会退化，
 * 重跑本脚本一次恢复；幂等：已裁部分不再命中）。
 * 用法：node scripts/prune-whitelist-all.mjs [vendor/kicad 根]
 */
import fs from 'node:fs';
import path from 'node:path';

const root = path.resolve(process.argv[2] || 'src/vendor/kicad');

// ── include 行删除（子串匹配，命中即删整行）──
const DROP_INCLUDES = [
  // UI / 帧 / 绘制
  'sch_draw_panel.h', 'sch_edit_frame.h', 'sch_painter.h', 'sch_view.h',
  'dialogs/html_message_box.h', 'symbol_edit_frame.h', 'sch_editor_control.h',
  'kicad_manager_frame.h',
  // 仿真 / 工具 / 网表 / IO / 库管理
  '#include <sim/', '#include "sim/', '#include <tools/', '#include "tools/',
  '#include <netlist_exporters/', '#include "netlist_exporters/',
  '#include "netlist_exporter_', '#include <netlist_exporter_',
  '#include <sch_io/', '#include "sch_io/', '#include <dialogs/', '#include "dialogs/',
  // 字体 / curl / GL / python / API
  'fontconfig.h', 'outline_font.h', 'kicad_curl',
  '#include <kicad_gl', '#include "kicad_gl', '#include "ft2build.h"', '#include <ft2build.h>',
  'python_scripting.h',
  '#include <api/', '#include "api/', '#include <google/', '#include "google/',
  '#include <sim/sim_preferences.h>', '#include "sim/sim_preferences.h"',
  'symbol_editor_settings.h',
  'plotters/plotter.h', 'kiway.h', 'legacy_symbol_library.h', 'stroke_params_parser.h',
  '#include <dialogs/dialog_bom_cfg_lexer.h>', '#include "dialogs/dialog_bom_cfg_lexer.h"',
  // wave2b：帧/面板/绘制基础（sch_shape 岛与 sch_field 恢复段）
  'eda_draw_frame.h', 'gr_basic.h', 'template_fieldnames_lexer.h',
];

// 必备 include 补回（模型层合法头；wave archive 覆盖会丢）。
// 注意：必须在 stripIncludes 之后应用（否则命中 DROP_INCLUDES 的会被再次剥掉）。
// 插入位置 = 文件内第一条 #include 之前；文件无 include（被剥空）时插在许可块之后。
const ADD_INCLUDES = [
  ['eeschema/sch_line.cpp', '#include <sch_screen.h>\n#include <i18n_utility.h>\n'],
  ['eeschema/sch_screen.cpp', '#include <sch_screen.h>\n#include <schematic.h>\n'],
  ['eeschema/sch_field.cpp', '#include <sch_field.h>\n#include <sch_screen.h>\n#include <schematic.h>\n#include <schematic_settings.h>\n#include <sch_item.h>\n'],
  ['common/eda_shape.cpp', '#include <i18n_utility.h>\n'],
  ['common/api/serializable.cpp', '#include <api/serializable.h>\n'],
  ['common/wave2b-stubs2.cpp', '#include <kiway.h>\n'],
];

// ── 函数块删除（锚点子串 → 括号匹配整块；含跨行签名则从所在行起）──
const DROP_FUNCTIONS = [
  // protobuf 序列化（API 排除区）
  'void SCH_LINE::Serialize', 'bool SCH_LINE::Deserialize',
  'void SCH_LABEL::Serialize', 'bool SCH_LABEL::Deserialize',
  'void SCH_DIRECTIVE_LABEL::Serialize', 'bool SCH_DIRECTIVE_LABEL::Deserialize',
  'void SCH_GLOBALLABEL::Serialize', 'bool SCH_GLOBALLABEL::Deserialize',
  'void SCH_HIERLABEL::Serialize', 'bool SCH_HIERLABEL::Deserialize',
  'void EDA_SHAPE::Serialize', 'bool EDA_SHAPE::Deserialize',
  'void EDA_TEXT::Serialize', 'bool EDA_TEXT::Deserialize',
  'void NETCLASS::Serialize', 'bool NETCLASS::Deserialize',
  'bool SERIALIZABLE::Deserialize', 'void SERIALIZABLE::Serialize',
  // UI 面板 / 帮助 / 绘图 / 库管理
  'SHOW_HELP', // 见下（sch_label ShowSyntaxHelp 单独锚）
  'HTML_MESSAGE_BOX* SCH_TEXT::ShowSyntaxHelp',
  'GetMsgPanelInfo( EDA_DRAW_FRAME',
  'void SCH_SCREEN::Plot(', 'void SCH_LINE::Plot(',
  'SCH_SCREEN::UpdateSymbolLinks', 'SCH_SCREENS::UpdateSymbolLinks',
  'LIB_SYMBOL::GetFonts() const', 'LIB_SYMBOL::EmbedFonts()',
  'BOM_CFG_PARSER::BOM_CFG_PARSER(', 'class BOM_CFG_PARSER', 'BOM_CFG_PARSER::Parse', 'BOM_CFG_PARSER::parseGenerator',
  'SCH_FIELD::GetRenderCache(',
  // sim / 工具实体
  'SCH_SYMBOL::GetSimModel', 'schsymbol_sim', 'SIM_LIB_MGR',
  // wave2b-1：SCH_SHAPE 绘图/面板/位图（模型层无调用方；GetItemDescription/GetMenuImage
  // 仅 GetItemDescription 被少量保留（无 KIUI 调用后），此处整体剪（模型层无调用方）
  'void SCH_SHAPE::Plot(', 'void SCH_SHAPE::GetMsgPanelInfo(',
  // wave2b-3：残留 Plot 族（wave2a 只剪了 sch_pin/sch_text/textbox，其余文件漏剪）
  'void LIB_SYMBOL::Plot(', 'void LIB_SYMBOL::PlotFields(',
  'void SCH_SYMBOL::Plot(', 'void SCH_SHEET::Plot(', 'void SCH_LABEL_BASE::Plot(',
  'void SCH_JUNCTION::Plot(', 'void SCH_RULE_AREA::Plot(', 'void SCH_RULE_AREA::PlotShape',
  'void SCH_NO_CONNECT::Plot(', 'void SCH_TEXTBOX::Plot(',
  // wave2b-2：sch_field 恢复段 UI/plot 剪除（OnScintillaCharAdded 为事故锚点，须剪）
  'void SCH_FIELD::Plot(', 'void SCH_FIELD::GetMsgPanelInfo(', 'void SCH_FIELD::DoHypertextAction(',
  'void SCH_FIELD::OnScintillaCharAdded(', 'SCH_FIELD::GetRenderCache(',
  // wave2b-2：EDA_TEXT 打印/渲染缓存链（wxDC/KIFONT/GAL 排除区；GetEffectiveTextShape
  // 与 AddRenderCacheGlyph 属 painter 渲染缓存）
  'void EDA_TEXT::Print( const RENDER_SETTINGS*', 'void EDA_TEXT::printOneLineOfText(',
  'void EDA_TEXT::AddRenderCacheGlyph(', 'std::shared_ptr<SHAPE_COMPOUND> EDA_TEXT::GetEffectiveTextShape(',
  // wave2b-3：reporter 状态栏（widgets/kistatusbar 排除区；宽锚循环删全家族）
  'STATUSBAR_WARNING_REPORTER',
  // wave2b-fix2：reporter 的 fontconfig 报告作用域类（fontconfig 排除区）
  'FONTCONFIG_REPORTER_SCOPE',
  // wave2b-3：PGM_BASE statusbar/design-block（UI/库管理排除区）
  'void PGM_BASE::PreloadDesignBlockLibraries( KIWAY*', 'void PGM_BASE::RegisterLibraryLoadStatusBar(',
  'void PGM_BASE::UnregisterLibraryLoadStatusBar(', 'void PGM_BASE::AddLibraryLoadMessages(',
  'void PGM_BASE::ClearLibraryLoadMessages()',
  // 切片3：sch_commit 帧语义剪除（Push/pushLibEdit/pushSchEdit/revertLibEdit/Revert/
  // ctor×2 帧重载）+ makeImage/undoLevelItem 的 lib-editor 分支（m_isLibEditor 恒 false）
  'void SCH_COMMIT::Push(', 'void SCH_COMMIT::pushLibEdit(', 'void SCH_COMMIT::pushSchEdit(',
  'void SCH_COMMIT::revertLibEdit(', 'void SCH_COMMIT::Revert(',
  'SCH_COMMIT::SCH_COMMIT( SCH_TOOL_BASE', 'SCH_COMMIT::SCH_COMMIT( EDA_DRAW_FRAME',
  'if( m_isLibEditor )',
  // wave2b-fix：残留 GetMsgPanelInfo/树模型/lexer 序列化（UI 与 widget 系）
  'EDA_SHAPE::ShapeGetMsgPanelInfo(', 'void SCH_MARKER::GetMsgPanelInfo(',
  'RC_TREE_MODEL::', 'ERC_TREE_MODEL::',
  'void TEMPLATE_FIELDNAME::Format( OUTPUTFORMATTER*', 'void TEMPLATE_FIELDNAME::Parse( TEMPLATE_FIELDNAMES_LEXER*',
  'void TEMPLATES::Format( OUTPUTFORMATTER*', 'void TEMPLATES::AddTemplateFieldNames(',
  // wave2b-fix2：gr_text 只留笔宽函数，GR 绘制族剪除（wxDC/GAL 排除区）
  'int GRTextWidth(', 'void GRPrintText(',
  // 切片2：font.cpp 的 GAL 绘制对（纯几何不需要；保留 FONT::Draw 桩）
  'void FONT::Draw(', 'void FONT::drawSingleLineText(',
];

// 括号匹配：跳过字符串/字符/注释（KiCad 源码里 '{' 会出现在字符字面量，如 == '{'；
// 朴素计数会永久失衡 → 从锚点吞到 EOF——wave2b 修 sch_field.cpp 时的工具级事故
function findBlockEnd(lines, ai) {
  let depth = 0, started = false;
  for (let j = ai; j < lines.length; j++) {
    const line = lines[j];
    let inStr = false, inChar = false, inBC = false;
    for (let i = 0; i < line.length; i++) {
      const ch = line[i], nx = line[i + 1];
      if (inBC) { if (ch === '*' && nx === '/') { inBC = false; i++; } continue; }
      if (inStr) { if (ch === '\\') i++; else if (ch === '"') inStr = false; continue; }
      if (inChar) { if (ch === '\\') i++; else if (ch === "'") inChar = false; continue; }
      if (ch === '/' && nx === '/') break;            // 行注释
      if (ch === '/' && nx === '*') { inBC = true; i++; continue; }
      if (ch === '"') { inStr = true; continue; }
      if (ch === "'") { inChar = true; continue; }
      if (ch === '{') { depth++; started = true; }
      else if (ch === '}') { depth--; if (started && depth === 0) return j; }
    }
    if (started && depth === 0) return j;
  }
  return started ? lines.length - 1 : ai; // 无花括号的单行（=default/=0 等）只删本行
}

function dropBlocks(rel, anchors) {
  const p = path.join(root, rel);
  if (!fs.existsSync(p)) { console.log('skip (absent):', rel); return; }
  let lines = fs.readFileSync(p, 'utf8').split('\n');
  let removed = 0;
  for (const anchor of anchors) {
    let ai = lines.findIndex((l) => l.includes(anchor) && !l.includes('#include'));
    while (ai >= 0) {
      let end = findBlockEnd(lines, ai);
      lines.splice(ai, end - ai + 1);
      removed++;
      ai = lines.findIndex((l) => l.includes(anchor) && !l.includes('#include'));
    }
  }
  if (removed) fs.writeFileSync(p, lines.join('\n'));
  if (removed) console.log(`pruned ${removed} fn: ${rel}`);
}

function stripIncludes(rel) {
  const p = path.join(root, rel);
  if (!fs.existsSync(p)) return;
  const lines = fs.readFileSync(p, 'utf8').split('\n');
  const out = lines.filter((l) => {
    if (!l.trim().startsWith('#include')) return true;
    return !DROP_INCLUDES.some((d) => l.includes(d));
  });
  if (out.length !== lines.length) {
    fs.writeFileSync(p, out.length ? out.join('\n') : '\n');
    console.log(`includes purged: ${rel} (-${lines.length - out.length})`);
  }
}

// 全树应用 include 裁剪 —— 只作用于 .cpp（头文件保持完整；白名单纪律在源文件层）
function walk(dir) {
  for (const name of fs.readdirSync(dir)) {
    const p = path.join(dir, name);
    const st = fs.statSync(p);
    if (st.isDirectory()) { walk(p); continue; }
    if (/\.cpp$/.test(name)) stripIncludes(path.relative(root, p));
  }
}
walk(root);

// 函数块：按文件分组应用
const perFile = {
  'eeschema/sch_line.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_LINE')),
  'eeschema/sch_label.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_LABEL') || a.includes('SCH_TEXT::ShowSyntaxHelp') || a.includes('GetMsgPanelInfo')) ,
  'eeschema/sch_junction.cpp': DROP_FUNCTIONS.filter((a) => a.includes('GetMsgPanelInfo') || a.includes('SCH_JUNCTION::Plot')),
  'eeschema/sch_sheet.cpp': DROP_FUNCTIONS.filter((a) => a.includes('GetMsgPanelInfo') || a.includes('SCH_SHEET::Plot')),
  'eeschema/sch_screen.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_SCREEN::Plot') || a.includes('UpdateSymbolLinks')),
  'eeschema/sch_item.cpp': DROP_FUNCTIONS.filter((a) => a.includes('UpdateSymbolLinks')),
  'common/eda_shape.cpp': DROP_FUNCTIONS.filter((a) => a.includes('EDA_SHAPE')),
  'common/eda_text.cpp': DROP_FUNCTIONS.filter((a) => a.includes('EDA_TEXT')),
  'common/netclass.cpp': DROP_FUNCTIONS.filter((a) => a.includes('NETCLASS')),
  'include/eda_shape.h': DROP_FUNCTIONS.filter((a) => a.includes('EDA_SHAPE')),
  'include/eda_text.h': DROP_FUNCTIONS.filter((a) => a.includes('EDA_TEXT')),
  'include/netclass.h': DROP_FUNCTIONS.filter((a) => a.includes('NETCLASS')),
  'eeschema/eeschema_settings.cpp': DROP_FUNCTIONS.filter((a) => a.includes('BOM_CFG_PARSER') || a.includes('migrateBom')),
  'eeschema/sch_field.cpp': DROP_FUNCTIONS.filter((a) => a.includes('GetRenderCache') || a.includes('GetMsgPanelInfo') || a.includes('SCH_FIELD::') && (a.includes('Plot') || a.includes('DoHypertextAction') || a.includes('OnScintillaCharAdded'))),
  'eeschema/lib_symbol.cpp': DROP_FUNCTIONS.filter((a) => a.includes('GetFonts') || a.includes('EmbedFonts') || a.includes('LIB_SYMBOL::GetJumper') || a.includes('GetEmbedded') || a.includes('LIB_SYMBOL::Plot')),
  'common/font/text_attributes.cpp': ['std::ostream& operator<<( std::ostream& aStream, const TEXT_ATTRIBUTES& aAttributes )'],
  // wave2b 新增
  'eeschema/sch_shape.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_SHAPE::Plot') || a.includes('SCH_SHAPE::GetMsgPanelInfo')),
  'eeschema/sch_symbol.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_SYMBOL::Plot')),
  'eeschema/sch_rule_area.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_RULE_AREA::Plot')),
  'eeschema/sch_no_connect.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_NO_CONNECT::Plot')),
  'eeschema/sch_textbox.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_TEXTBOX::Plot')),
  'common/reporter.cpp': DROP_FUNCTIONS.filter((a) => a.includes('STATUSBAR_WARNING_REPORTER') || a.includes('FONTCONFIG_REPORTER_SCOPE')),
  'common/pgm_base.cpp': DROP_FUNCTIONS.filter((a) => a.includes('PGM_BASE::PreloadDesignBlockLibraries') || a.includes('PGM_BASE::RegisterLibraryLoadStatusBar') || a.includes('PGM_BASE::UnregisterLibraryLoadStatusBar') || a.includes('PGM_BASE::AddLibraryLoadMessages') || a.includes('PGM_BASE::ClearLibraryLoadMessages')),
  'common/template_fieldnames.cpp': DROP_FUNCTIONS.filter((a) => a.includes('TEMPLATES::parse') || a.includes('TEMPLATE_FIELDNAME::') || a.includes('TEMPLATES::Format') || a.includes('TEMPLATES::AddTemplateFieldNames')),
  'common/eda_shape.cpp': DROP_FUNCTIONS.filter((a) => a.includes('EDA_SHAPE::ShapeGetMsgPanelInfo')),
  'common/rc_item.cpp': DROP_FUNCTIONS.filter((a) => a.includes('RC_TREE_MODEL')),
  'eeschema/sch_marker.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_MARKER::GetMsgPanelInfo')),
  'eeschema/erc/erc_item.cpp': DROP_FUNCTIONS.filter((a) => a.includes('ERC_TREE_MODEL')),
  'eeschema/sch_rule_area.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_RULE_AREA::Plot') || a.includes('GetMsgPanelInfo( EDA_DRAW_FRAME')),
  'common/gr_text.cpp': DROP_FUNCTIONS.filter((a) => a.includes('GRTextWidth') || a.includes('GRPrintText')),
  'common/font/font.cpp': DROP_FUNCTIONS.filter((a) => a.includes('FONT::Draw') || a.includes('drawSingleLineText')),
  'common/marker_base.cpp': DROP_FUNCTIONS.filter((a) => a.includes('ShowHtml') || a.includes('ShowReport')),
  'eeschema/sch_commit.cpp': DROP_FUNCTIONS.filter((a) => a.includes('SCH_COMMIT::Push') || a.includes('pushLibEdit') || a.includes('pushSchEdit') || a.includes('revertLibEdit') || a.includes('SCH_COMMIT::Revert') || a.includes('SCH_COMMIT::SCH_COMMIT( SCH_TOOL_BASE') || a.includes('SCH_COMMIT::SCH_COMMIT( EDA_DRAW_FRAME') || a.includes('if( m_isLibEditor )')),
};
for (const [rel, anchors] of Object.entries(perFile)) dropBlocks(rel, anchors);

// ADD_INCLUDES 应用（置于 stripIncludes/函数剪之后；详见上方注释）
for (const [rel, block] of ADD_INCLUDES) {
  const p = path.join(root, rel);
  if (!fs.existsSync(p)) continue;
  const text = fs.readFileSync(p, 'utf8').replace(/\r\n/g, '\n');
  const addLines = block.trimEnd().split('\n');
  const missing = addLines.filter((l) => !text.includes(l));
  if (missing.length) {
    const add = missing.join('\n') + '\n';
    const m = text.match(/\n(#[ \t]*include[^\n]*)/);
    if (m) {
      fs.writeFileSync(p, text.replace(m[1], add + m[1]));
    } else {
      fs.writeFileSync(p, text.replace('*/', '*/\n\n' + add));
    }
    console.log('includes added:', rel, '(+' + missing.length + ')');
  }
}

// ── 点修复（幂等文本替换；git archive 恢复原版后可重放）──
const FIX_TEXT = [
  ['common/pgm_base.cpp', '    KICAD_CURL::Cleanup();\n\n', ''],
  ['common/pgm_base.cpp', '    KICAD_CURL::Init();\n\n', ''],
  // cicada B3a-1: 剪掉的 InitPgm settings 分配整块恢复为一条"只分配 settings"（上游 :454 全块；
  // library/backgrounds/notifications 保持 null，路径未触及）。
  ['common/pgm_base.cpp', '    m_settings_manager = std::make_unique<SETTINGS_MANAGER>();\n', ''],
  ['common/pgm_base.cpp', '    APP_MONITOR::SENTRY::Instance()->Cleanup();\n', ''],
  ['common/pgm_base.cpp', '    APP_MONITOR::SENTRY::Instance()->Init();\n', ''],
  ['common/pgm_base.cpp', '#include <python_scripting.h>\n', ''],
  ['common/pgm_base.cpp', 'm_python_scripting = std::make_unique<SCRIPTING>();\n', ''],
  ['common/richio.cpp', 'getc_unlocked(', 'getc('],
  ['common/singleton.cpp', 'BS::this_thread::set_os_thread_priority(', 'BS::this_thread::set_os_thread_priority('], // 锚（下方块替换）
  ['common/singleton.cpp', '    m_GLContextManager = new GL_CONTEXT_MANAGER();\n', ''],
  ['common/reporter.cpp', 'FONTCONFIG_REPORTER_SCOPE::FONTCONFIG_REPORTER_SCOPE(', 'FONTCONFIG_REPORTER_SCOPE::FONTCONFIG_REPORTER_SCOPE('], // 剪函数见下
  // font/text_attributes.cpp：font 区脱钩（GetName 整语句置空；operator<< 剪函数见下）
  ['common/font/text_attributes.cpp', 'fontName = m_Font->GetName();', 'fontName = wxString();'],
  ['common/font/text_attributes.cpp', 'rhsFontName = aRhs.m_Font->GetName();', 'rhsFontName = wxString();'],
  // wave2b-3：pgm_base 排除区脱钩（sentry/wxPG/版本/settings+library+notifications+background-jobs 管理器）
  ['common/pgm_base.cpp', 'APP_MONITOR::SENTRY::Instance()->AddTag( "kicad.app", pgm_name );', ''],
  ['common/pgm_base.cpp', '    if( !wxPGGlobalVars )\n        wxPGInitResourceModule();\n', ''],
  ['common/pgm_base.cpp', 'GetMajorMinorVersion()', 'wxT( "10.0" )'],
  ['common/pgm_base.cpp', 'APP_MONITOR::SENTRY::Instance()->LogException( ioe.What(), aUnhandled );', ''],
  ['common/pgm_base.cpp', 'APP_MONITOR::SENTRY::Instance()->LogException( e.what(), aUnhandled );', ''],
  ['common/pgm_base.cpp', 'APP_MONITOR::SENTRY::Instance()->LogException( "Unhandled exception of unknown type", aUnhandled );', ''],
  ['common/pgm_base.cpp', 'APP_MONITOR::SENTRY::Instance()->LogAssert( key, assertStr );', ''],
  ['common/pgm_base.cpp', '    m_settings_manager = std::make_unique<SETTINGS_MANAGER>();\n', ''],
  ['common/pgm_base.cpp', '    m_library_manager = std::make_unique<LIBRARY_MANAGER>();\n', ''],
  ['common/pgm_base.cpp', '    m_background_jobs_monitor = std::make_unique<BACKGROUND_JOBS_MONITOR>();\n', ''],
  ['common/pgm_base.cpp', '    m_notifications_manager = std::make_unique<NOTIFICATIONS_MANAGER>();\n', ''],
  ['common/pgm_base.cpp', '    if( !m_settings_manager->IsOK() )\n        return false;\n', ''],
  ['common/pgm_base.cpp', '    COMMON_SETTINGS* commonSettings = GetCommonSettings();\n    commonSettings->InitializeEnvironment();\n', ''],
  ['common/pgm_base.cpp', '    m_settings_manager->ReloadColorSettings();\n', ''],
  ['common/pgm_base.cpp', '    GetSettingsManager().Load( commonSettings );\n', ''],
  ['common/pgm_base.cpp', 'return m_settings_manager ? m_settings_manager->GetCommonSettings() : nullptr;', 'return nullptr;'],
  // wave2b-3：KIUI/库管理/计时 引用剪除（widgets/app_monitor/collectors 排除区）
  ['eeschema/sch_symbol.cpp', 'KIUI::EllipsizeMenuText( GetField( FIELD_T::REFERENCE )->GetText() )', 'GetField( FIELD_T::REFERENCE )->GetText()'],
  ['eeschema/sch_symbol.cpp', 'KIUI::EllipsizeMenuText( GetLibId().GetLibItemName() )', 'GetLibId().GetLibItemName()'],
  ['eeschema/sch_label.cpp', 'KIUI::EllipsizeMenuText( GetText() )', 'GetText()'],
  ['eeschema/sch_label.cpp', 'KIUI::EllipsizeMenuText( firstField.GetText() )', 'firstField.GetText()'],
  ['eeschema/sch_field.cpp', 'KIUI::EllipsizeMenuText( GetText() )', 'GetText()'],
  ['eeschema/schematic.cpp', 'CollectOtherUnits( ref, unit, libId, sheet, &otherUnits );', ''],
  ['eeschema/sch_symbol.cpp', 'CollectOtherUnits( ref, m_unit, m_lib_id, sheet, &otherUnits );', ''],
  // wave2b-3：eeschema_settings 面板静态名（EDA_DRAW_FRAME 排除区）
  ['eeschema/eeschema_settings.cpp', 'EDA_DRAW_FRAME::PropertiesPaneName()', 'wxS( "Properties" )'],
  ['eeschema/eeschema_settings.cpp', 'EDA_DRAW_FRAME::DesignBlocksPaneName()', 'wxS( "Design Blocks" )'],
  ['eeschema/eeschema_settings.cpp', 'EDA_DRAW_FRAME::RemoteSymbolPaneName()', 'wxS( "Remote Symbols" )'],
  // wave2b-1：template_fieldnames 生成 lexer 命名空间（TFIELD_T 在生成的 lexer 头里）
  ['common/template_fieldnames.cpp', 'using namespace TFIELD_T;', ''],
  // wave2b-fix：sch_field 返回类型孤行（GetRenderCache 拆行签名剪除后的残留）
  ['eeschema/sch_field.cpp', 'std::vector<std::unique_ptr<KIFONT::GLYPH>>*', ''],
  // wave2b-fix：GetItemDescription %s 传入 UTF8（原 Ellipsize 包裹被去掉后变裸 UTF8）
  ['eeschema/sch_symbol.cpp', 'wxString( GetLibId().GetLibItemName() )', 'wxString( GetLibId().GetLibItemName().c_str() )'],
  // wave2b-fix2：paths.cpp 版本函数字面量化（build_version.cpp 为生成/排除区）
  ['common/paths.cpp', 'wxT( "10.0" ).ToStdString()', '"10.0"'],
  // wave2b-fix3：env_vars.cpp 获取版本号（build_version 排除区；改字面量）
  ['common/env_vars.cpp', 'version = wxT( "10.0" );', 'version = 10;'],
  // 切片3：sch_commit ctor 去 SCH_BASE_FRAME 探测（m_toolMgr 可 nullptr；帧排除）
  ['eeschema/sch_commit.cpp', '    SCH_BASE_FRAME* frame = static_cast<SCH_BASE_FRAME*>( m_toolMgr->GetToolHolder() );\n    m_isLibEditor = frame && frame->IsType( FRAME_SCH_SYMBOL_EDITOR );\n', ''],
  // 切片2：font.cpp 去 OUTLINE_FONT::LoadFont 分支（字体锁 STROKE_FONT；未知名字回落默认）
  ['common/font/font.cpp', '        font = OUTLINE_FONT::LoadFont( aFontName, aBold, aItalic, aEmbeddedFiles,\n                                       aForDrawingSheet );\n', ''],
  // 切片2：sch_render_settings 去 Kiface 链（GetShowPageLimits→false）
  ['eeschema/sch_render_settings.cpp', '    EESCHEMA_SETTINGS* cfg = dynamic_cast<EESCHEMA_SETTINGS*>( Kiface().KifaceSettings() );\n    return cfg && cfg->m_Appearance.show_page_limits && !IsPrinting();', '    return false;'],
];
function applyTextFixes() {
  for (const [rel, old, nw] of FIX_TEXT) {
    const p = path.join(root, rel);
    if (!fs.existsSync(p)) continue;
    const t = fs.readFileSync(p, 'utf8').replace(/\r\n/g, '\n');
    if (t.includes(old)) {
      fs.writeFileSync(p, t.split(old).join(nw));
      console.log('text-fixed:', rel);
    }
  }
  // singleton priority pool block (simplify without the os_thread lambda)
  const sing = path.join(root, 'common/singleton.cpp');
  if (fs.existsSync(sing)) {
    let t = fs.readFileSync(sing, 'utf8');
    const oldBlock = t.match(/m_ThreadPool = new BS::priority_thread_pool\([\s\S]*?\);\n/);
    if (oldBlock) {
      t = t.replace(oldBlock[0], '    m_ThreadPool = new BS::priority_thread_pool( num_threads );\n');
      fs.writeFileSync(sing, t);
      console.log('text-fixed: singleton pool');
    }
  }
  // singleton.h: drop GL forward + member + ctor init
  const singH = path.join(root, 'include/singleton.h');
  if (fs.existsSync(singH)) {
    let t = fs.readFileSync(singH, 'utf8');
    let n0 = t;
    t = t.replace('class GL_CONTEXT_MANAGER;\n', '');
    t = t.replace(/[^\n]*m_GLContextManager[^\n]*\n/g, '');
    if (t !== n0) { fs.writeFileSync(singH, t); console.log('text-fixed: singleton.h gl'); }
  }
  // pgm_base.h: scripting/GL decls
  const pgmH = path.join(root, 'include/pgm_base.h');
  if (fs.existsSync(pgmH)) {
    let t = fs.readFileSync(pgmH, 'utf8');
    let n0 = t;
    t = t.replace('class SCRIPTING;\n', '');
    t = t.replace(/[^\n]*(class SCRIPTING|std::unique_ptr<SCRIPTING>|m_python_scripting|GL_CONTEXT_MANAGER\* GetGLContextManager|m_GLContextManager)[^\n]*\n/g, '');
    if (t !== n0) { fs.writeFileSync(pgmH, t); console.log('text-fixed: pgm_base.h'); }
  }
}
applyTextFixes();
console.log('prune-whitelist-all done');
