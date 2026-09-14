/*
 * wave2b-stubs.cpp — 切片 1 排除区符号降级桩（CICADA 自建，非 KiCad 原件）
 *
 * 原则：仅当"符号定义属排除区（api/proto、脚本嵌入、字体内嵌、库管理器、字段
 * 自动布局）、且在切片 1 数据模型路径上不需要真实行为"时才 stub；语义取最无害
 * 降级（空体 / 返回 nil / 默认值）。签名与 vendor 头逐字核对（10.0.6 caf7377e）。
 * 其余真缺口一律按"拷贝真源"处理（见 source-lists/lnk-wave2b.txt 映射）。
 */
#include <sch_line.h>
#include <sch_label.h>
#include <sch_symbol.h>
#include <lib_symbol.h>
#include <eda_shape.h>
#include <eda_text.h>
#include <netclass.h>
#include <embedded_files.h>
#include <settings/settings_manager.h>
#include <project.h>
#include <app_monitor.h>
#include <text_eval/text_eval_wrapper.h>

// ── 1) proto-Serialize 虚函数族（API 排除区）──────────────────────────────
// 参数为 Any&/const Any&，函数体不触碰 → 仅需前置声明即可编译；vtable 槽满足。
void EDA_SHAPE::Serialize( google::protobuf::Any& ) const
{
}

bool EDA_SHAPE::Deserialize( const google::protobuf::Any& )
{
    return false;
}

void EDA_TEXT::Serialize( google::protobuf::Any& ) const
{
}

bool EDA_TEXT::Deserialize( const google::protobuf::Any& )
{
    return false;
}

void NETCLASS::Serialize( google::protobuf::Any& ) const
{
}

bool NETCLASS::Deserialize( const google::protobuf::Any& )
{
    return false;
}

void SCH_LINE::Serialize( google::protobuf::Any& ) const
{
}

bool SCH_LINE::Deserialize( const google::protobuf::Any& )
{
    return false;
}

void SCH_LABEL::Serialize( google::protobuf::Any& ) const
{
}

bool SCH_LABEL::Deserialize( const google::protobuf::Any& )
{
    return false;
}

// ── 2) EMBEDDED_FILES（脚本/字体内嵌 = slice2 之后功能；数据容器降级）─────
EMBEDDED_FILES::EMBEDDED_FILES( EMBEDDED_FILES&& other ) noexcept : EMBEDDED_FILES()
{
}

EMBEDDED_FILES::EMBEDDED_FILES( const EMBEDDED_FILES& other ) : EMBEDDED_FILES()
{
}

EMBEDDED_FILES::EMBEDDED_FILES( const EMBEDDED_FILES& other, bool aDeepCopy )
{
}

EMBEDDED_FILES& EMBEDDED_FILES::operator=( EMBEDDED_FILES&& other ) noexcept
{
    return *this;
}

EMBEDDED_FILES& EMBEDDED_FILES::operator=( const EMBEDDED_FILES& other )
{
    return *this;
}

wxString EMBEDDED_FILES::EMBEDDED_FILE::GetLink() const
{
    return wxString();
}

EMBEDDED_FILES::EMBEDDED_FILE* EMBEDDED_FILES::AddFile( const wxFileName& aName, bool aOverwrite )
{
    return nullptr;
}

void EMBEDDED_FILES::AddFile( EMBEDDED_FILE* aFile )
{
    // 所有权转移语义：释放以免泄漏（切片 1 不实际使用内嵌文件）
    delete aFile;
}

void EMBEDDED_FILES::AddFile( std::shared_ptr<EMBEDDED_FILE> aFile )
{
}

const std::vector<wxString>* EMBEDDED_FILES::UpdateFontFiles()
{
    static const std::vector<wxString> v;
    return &v;
}

const std::vector<wxString>* EMBEDDED_FILES::GetFontFiles() const
{
    static const std::vector<wxString> v;
    return &v;
}

// ── 3) 字段自动布局（slice 2 才需要的摆放算法）───────────────────────────
void LIB_SYMBOL::AutoplaceFields( SCH_SCREEN* aScreen, AUTOPLACE_ALGO aAlgo )
{
}

void SCH_SYMBOL::AutoplaceFields( SCH_SCREEN* aScreen, AUTOPLACE_ALGO aAlgo )
{
}

// ── 4) SETTINGS_MANAGER 三个被模型 settings 代码引用的方法 ─────────────────
// （color_settings 迁移路径在 m_manager==null 时提前返回；schematic_settings
//   GetJunctionSize 的 Prj 路径在切片 1 不执行——管理器无生命周期）
COLOR_SETTINGS* SETTINGS_MANAGER::AddNewColorSettings( const wxString& aFilename )
{
    return nullptr;
}

void SETTINGS_MANAGER::Save( JSON_SETTINGS* aSettings )
{
}

PROJECT& SETTINGS_MANAGER::Prj() const
{
    static PROJECT p;
    return p;
}

// ── 5) APP_MONITOR::TRANSACTION（性能计时；sentry 已从头部裁剪）────────────
namespace APP_MONITOR
{

TRANSACTION::TRANSACTION( const std::string& aName, const std::string& aOperation )
{
}

TRANSACTION::~TRANSACTION()
{
}

void TRANSACTION::Start()
{
}

void TRANSACTION::StartSpan( const std::string& aOperation, const std::string& aDescription )
{
}

void TRANSACTION::FinishSpan()
{
}

void TRANSACTION::Finish()
{
}

} // namespace APP_MONITOR

// ── 6) EXPRESSION_EVALUATOR（表达式求值 = 生成式 lemon 解析器，排除区）──────
// 降级：Evaluate 原样返回输入（${var} 展开在切片 1 不需要）。
EXPRESSION_EVALUATOR::EXPRESSION_EVALUATOR( bool aClearVariablesOnEvaluate )
{
}

EXPRESSION_EVALUATOR::EXPRESSION_EVALUATOR( const EXPRESSION_EVALUATOR& aOther )
{
}

EXPRESSION_EVALUATOR::~EXPRESSION_EVALUATOR()
{
}

EXPRESSION_EVALUATOR& EXPRESSION_EVALUATOR::operator=( const EXPRESSION_EVALUATOR& aOther )
{
    return *this;
}

wxString EXPRESSION_EVALUATOR::Evaluate( const wxString& aInput )
{
    return aInput;
}
