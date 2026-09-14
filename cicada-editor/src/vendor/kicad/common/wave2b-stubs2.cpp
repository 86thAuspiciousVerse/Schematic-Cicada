/*
 * wave2b-stubs2.cpp — 切片 1 残余 LNK（第 2 批）降级桩（CICADA 自建，非 KiCad 原件）
 *
 * 覆盖 rel57 LNK1120=96 的残余：vtable 驱动的 GetMsgPanelInfo/Plot 族（体被剪、
 * 声明在头 → vtable 槽引用）、GAL/VIEW/PAINTER 渲染链、字体注册表/指标、
 * 管理器与静态成员、json 转换、工具函数。全部签名与 vendor 头逐字核对；
 * 语义取最无害降级（空体 / nil / 原样）。依赖真源的（MARKER_BASE/wildcards/
 * gr_text 笔宽/fmt）另以拷贝解决。
 */
#include <kiway.h>
#include <sch_field.h>
#include <sch_line.h>
#include <sch_junction.h>
#include <sch_sheet.h>
#include <sch_text.h>
#include <sch_pin.h>
#include <sch_rule_area.h>
#include <sch_shape.h>
#include <sch_label.h>
#include <sch_marker.h>
#include <sch_no_connect.h>
#include <sch_symbol.h>
#include <lib_symbol.h>
#include <sch_commit.h>
#include <schematic.h>
#include <settings/settings_manager.h>
#include <template_fieldnames.h>
#include <settings/app_settings.h>
#include <project.h>
#include <project/project_file.h>
#include <app_monitor.h>
#include <lockfile.h>
#include <design_block_library_adapter.h>
#include <gal/gal.h>
#include <gal/graphics_abstraction_layer.h>
#include <gal/gal_display_options.h>
#include <view/view.h>
#include <view/view_group.h>
#include <gal/painter.h>
#include <callback_gal.h>
#include <widgets/report_severity.h>
#include <font/font.h>
#include <font/font_metrics.h>
#include <font/stroke_font.h>
#include <text_eval/text_eval_wrapper.h>
#include <libeval/numeric_evaluator.h>

#include <nlohmann/json.hpp>
#include <reporter.h>
#include <libraries/library_manager.h>
#include <notifications_manager.h>
#include <confirm.h>
#include <wx/aui/aui.h>

// ── A) GetMsgPanelInfo 族（UI 面板；空体）──────────────────────────────────
void SCH_LINE::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_JUNCTION::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_SHEET::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_TEXT::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_PIN::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_RULE_AREA::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_SHAPE::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_LABEL_BASE::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_MARKER::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_FIELD::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

// ── B) Plot 族（plotters 排除区；空体）────────────────────────────────────
void SCH_PIN::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                    int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

void SCH_RULE_AREA::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                          int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

void SCH_SHAPE::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                      int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

void SCH_SHEET::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                      int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

void SCH_SYMBOL::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                       int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

// ── C) KIFONT 字体注册表/指标（font.cpp 排除；降级）───────────────────────
void KIFONT::FONT::Draw( KIGFX::GAL* aGal, const wxString& aText, const VECTOR2I& aPos,
                         const VECTOR2I& aSize, const TEXT_ATTRIBUTES& aAttrs,
                         const KIFONT::METRICS& aFontMetrics, std::optional<VECTOR2I> aClip,
                         wxString* aDrawingGlyph ) const
{
}

std::set<KIFONT::OUTLINE_FONT*> LIB_SYMBOL::GetFonts() const
{
    return {};
}

std::set<KIFONT::OUTLINE_FONT*> SCHEMATIC::GetFonts() const
{
    return {};
}

void LIB_SYMBOL::EmbedFonts()
{
}

void SCHEMATIC::EmbedFonts()
{
}

// ── D) KIGFX GAL/VIEW/PAINTER（渲染链；空体/占位）─────────────────────────
KIGFX::GAL::GAL( KIGFX::GAL_DISPLAY_OPTIONS& aOptions ) : m_options( aOptions )
{
}

KIGFX::GAL::~GAL()
{
}

double KIGFX::GAL::computeMinGridSpacing() const
{
    return 0.0;
}

bool KIGFX::GAL::updatedGalDisplayOptions( const KIGFX::GAL_DISPLAY_OPTIONS& aOptions )
{
    return false;
}

void KIGFX::GAL::OnGalDisplayOptionsChanged( const KIGFX::GAL_DISPLAY_OPTIONS& aOptions )
{
}

void KIGFX::GAL::BitmapText( const wxString& aText, const VECTOR2I& aPosition,
                             const EDA_ANGLE& aAngle )
{
}

void KIGFX::GAL::ComputeWorldScreenMatrix()
{
}

void CALLBACK_GAL::DrawGlyph( const KIFONT::GLYPH& aGlyph, int aNth, int aTotal )
{
}

KIGFX::PAINTER::~PAINTER()
{
}

// ── E) 管理器/静态成员（模型无生命周期）──────────────────────────────────
SETTINGS_MANAGER::SETTINGS_MANAGER()
{
    // 壳层退化（frame-adjacent：磁盘配置 IO 后置）：成员与上游 ctor 初始化列表一致——
    // 指针/布尔成员置默认（容器与 m_app_settings_mutex 由类默认构造负责，否则
    // GetAppSettings<T> 的 std::lock_guard 会在未构造的 mutex 上崩溃——B3a-1 实测栈
    // Mtx_lock←GetAppSettings←SCH_ITEM::GetDefaultFont←…←Append）。
    // 真实回退语义：GetAppSettings 找不到注册项时返回 nullptr → GetDefaultFont 回落
    // KICAD_FONT_NAME（KiCad 自带 fallback，settings_manager.h 模板真源）。
    m_kiway = nullptr;
    m_common_settings = nullptr;
    m_migrateLibraryTables = true;
    m_ok = true;
}

SETTINGS_MANAGER::~SETTINGS_MANAGER()
{
}

bool SETTINGS_MANAGER::LoadProject( const wxString& aProject, bool aReuse )
{
    return false;
}

COLOR_SETTINGS* SETTINGS_MANAGER::GetColorSettings( const wxString& aName )
{
    return nullptr;
}

wxString SETTINGS_MANAGER::GetPathForSettingsFile( JSON_SETTINGS* aSettings )
{
    return wxString();
}

PROJECT::PROJECT()
{
    // GetProjectFile() 头内联体 *m_projectFile——空指针会崩（SetProject 首次即调用）
    // 注意：其余 PROJECT 指针成员（m_localSettings/m_settingsManager 等）仍为未初始化
    // 状态；SetProject/CacheExistingAnnotation 路径未触碰（记录在案）。
    m_projectFile = new PROJECT_FILE( wxEmptyString );
}

PROJECT::~PROJECT()
{
}

void SCH_COMMIT::Push( const wxString& aMessage, int aCommitFlags )
{
}

namespace APP_MONITOR
{

SENTRY::SENTRY()
{
}

SENTRY* SENTRY::m_instance = nullptr;

} // namespace APP_MONITOR

std::shared_mutex DESIGN_BLOCK_LIBRARY_ADAPTER::GlobalLibraryMutex;
LEAK_AT_EXIT<std::map<wxString, LIB_DATA>> DESIGN_BLOCK_LIBRARY_ADAPTER::GlobalLibraries;

bool APP_SETTINGS_BASE::MigrateFromLegacy( wxConfigBase* aCfg )
{
    return false;
}

void TEMPLATES::AddTemplateFieldNames( const wxString& aSerializedFieldNames )
{
}

// ── F) 工具函数（定义在排除区/已删文件的降级）────────────────────────────
bool NUMERIC_EVALUATOR::IsOldSchoolDecimalSeparator( wxUniChar aChar, double* aValue )
{
    return false;
}

bool NUMERIC_EVALUATOR::IsOldSchoolDecimalSeparator( char aChar, double* aValue )
{
    return false;
}

wxString SeverityToString( const SEVERITY& aSeverity )
{
    return wxEmptyString;
}

SEVERITY SeverityFromString( const wxString& aText )
{
    return RPT_SEVERITY_UNDEFINED;
}

wxString SCHEMATIC::GetOperatingPoint( const wxString& aNetName, int aPrecision,
                                       const wxString& aRange )
{
    return wxEmptyString;
}

// ── G) nlohmann json 转换（wxPoint/wxSize，原属 settings JSON 层）────────
void to_json( nlohmann::json& aJson, const wxPoint& aPoint )
{
    aJson = nlohmann::json{ { "x", aPoint.x }, { "y", aPoint.y } };
}

void from_json( const nlohmann::json& aJson, wxPoint& aPoint )
{
}

void to_json( nlohmann::json& aJson, const wxSize& aSize )
{
    aJson = nlohmann::json{ { "w", aSize.x }, { "h", aSize.y } };
}

void from_json( const nlohmann::json& aJson, wxSize& aSize )
{
}

// ── H) 第 3 轮残余（PROJECT 全量虚拟 / SCH_COMMIT 扩展 / 各杂项）───────────
bool PROJECT::TextVarResolver( wxString* aToken ) const
{
    return false;
}

std::map<wxString, wxString>& PROJECT::GetTextVars() const
{
    static std::map<wxString, wxString> m;
    return m;
}

void PROJECT::ApplyTextVars( const std::map<wxString, wxString>& aVarsMap )
{
}

const wxString PROJECT::GetProjectFullName() const
{
    return wxEmptyString;
}

const wxString PROJECT::GetProjectPath() const
{
    return wxEmptyString;
}

const wxString PROJECT::GetProjectDirectory() const
{
    return wxEmptyString;
}

const wxString PROJECT::GetProjectName() const
{
    return wxEmptyString;
}

bool PROJECT::IsNullProject() const
{
    return true;
}

const wxString PROJECT::GetSheetName( const KIID& aSheetID )
{
    return wxEmptyString;
}

const wxString PROJECT::FootprintLibTblName() const
{
    return wxEmptyString;
}

const wxString PROJECT::SymbolLibTableName() const
{
    return wxEmptyString;
}

const wxString PROJECT::DesignBlockLibTblName() const
{
    return wxEmptyString;
}

const wxString& PROJECT::GetRString( RSTRING_T aStringId )
{
    static const wxString s;
    return s;
}

void PROJECT::SetRString( RSTRING_T aStringId, const wxString& aString )
{
}

PROJECT::_ELEM* PROJECT::GetElem( PROJECT::ELEM aIndex )
{
    return nullptr;
}

void PROJECT::SetElem( PROJECT::ELEM aIndex, PROJECT::_ELEM* aElem )
{
}

const wxString PROJECT::AbsolutePath( const wxString& aFileName ) const
{
    return aFileName;
}

FOOTPRINT_LIBRARY_ADAPTER* PROJECT::FootprintLibAdapter( KIWAY& aKiway )
{
    return nullptr;
}

DESIGN_BLOCK_LIBRARY_ADAPTER* PROJECT::DesignBlockLibs()
{
    return nullptr;
}

void PROJECT::elemsClear()
{
}

void PROJECT::setProjectFullName( const wxString& aFullPathAndName )
{
}

// SCH_COMMIT 扩展（undo 细节已由 commit.cpp 真源承载；这些模型的剪除体在此补全）
void SCH_COMMIT::Revert()
{
}

// 状态栏 / 库管理器 / 通知 / 对话框
STATUSBAR_WARNING_REPORTER::~STATUSBAR_WARNING_REPORTER()
{
}

REPORTER& STATUSBAR_WARNING_REPORTER::Report( const wxString& aText, SEVERITY aSeverity )
{
    return *this;
}

LIBRARY_MANAGER::~LIBRARY_MANAGER()
{
}

LIBRARY_MANAGER_ADAPTER::~LIBRARY_MANAGER_ADAPTER()
{
}

void LIBRARY_MANAGER_ADAPTER::ProjectChanged()
{
}

bool LIBRARY_MANAGER_ADAPTER::IsWritable( const wxString& aNickname ) const
{
    return false;
}

std::optional<LIBRARY_ERROR> LIBRARY_MANAGER_ADAPTER::LibraryError( const wxString& aNickname ) const
{
    return std::nullopt;
}

void NOTIFICATIONS_MANAGER::Load()
{
}

void DisplayInfoMessage( wxWindow* aParent, const wxString& aHeading, const wxString& aMessage )
{
}

// 残余 virtual 补齐（vtable 槽）
wxString SCH_SHEET::GetItemDescription( UNITS_PROVIDER* aUnitsProvider, bool aFull ) const
{
    return wxEmptyString;
}

void SCH_SYMBOL::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
}

void SCH_FIELD::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                      int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

void SCH_FIELD::DoHypertextAction( EDA_DRAW_FRAME* aFrame, const VECTOR2I& aPosition ) const
{
}

void SCH_JUNCTION::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                         int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

void SCH_LABEL_BASE::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                           int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

void SCH_LINE::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                     int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

void SCH_TEXT::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                     int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

void SCH_TEXT::DoHypertextAction( EDA_DRAW_FRAME* aFrame, const VECTOR2I& aPosition ) const
{
}

void LIB_SYMBOL::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                       int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
}

bool KIGFX::GAL::SetNativeCursorStyle( KICURSOR aCursor, bool aDefaultCursor )
{
    return false;
}

class KIFACE; // 前置声明（kiface_base.h 属框架层，仅取指针数组定义）
std::array<KIFACE*, KIWAY::KIWAY_FACE_COUNT> KIWAY::m_kiface;

// nlohmann json 转换（wxAuiPaneInfo / wxRect；aui/json 层排除区）
void to_json( nlohmann::json& aJson, const wxAuiPaneInfo& aPane )
{
}

void from_json( const nlohmann::json& aJson, wxAuiPaneInfo& aPane )
{
}

void to_json( nlohmann::json& aJson, const wxRect& aRect )
{
    aJson = nlohmann::json{ { "x", aRect.x }, { "y", aRect.y },
                            { "w", aRect.width }, { "h", aRect.height } };
}

void from_json( const nlohmann::json& aJson, wxRect& aRect )
{
}

// K6 补充：SCH_NO_CONNECT vtable 桩（sch_no_connect.cpp 的 Plot 为排除区绘制函数被剪，但
// vtable 引用它——桥装载 no_connect 时实例化 vtable 触发 LNK2001；空体即可）
class PLOTTER;
struct SCH_PLOT_OPTS;
void SCH_NO_CONNECT::Plot( PLOTTER* aPlotter, bool aBackground, const SCH_PLOT_OPTS& aPlotOpts,
                           int aUnit, int aBodyStyle, const VECTOR2I& aOffset, bool aDimmed )
{
    (void) aPlotter; (void) aBackground; (void) aPlotOpts;
    (void) aUnit; (void) aBodyStyle; (void) aOffset; (void) aDimmed;
}
