// sch_interaction_engine.h — 切片 3 B3 壳层交互引擎（CICADA 自研，非 vendor）
// 依据：docs/07-子代理过程产物/KiCad抽取施工/B3壳层交互引擎探路.md（B3a-1 基座）
// 职责边界：本引擎只做"模型级原子操作"（命中/移动/端点拖/框选/删除/undo/放置），
// 一律 IU 域；wx 事件状态机（点选→拖移/端点拖/框选）在 WxCanvasPanel 侧驱动。
// 铁律（探路报告 §0）：几何改动后必须 screen->Update(item,false)（B3a-1 收敛到 CommitEdits）；
// Append/Update/Remove 一律传 aUpdateLibSymbol=false；编辑后 Recalculate。
#pragma once

#include <sch_screen.h>
#include <sch_sheet_path.h>
#include <schematic.h>
#include <sch_item.h>
#include <sch_symbol.h>
#include <sch_line.h>
#include <lib_symbol.h>
#include <refdes_tracker.h>
#include <math/vector2d.h>

#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <wx/string.h>

namespace cicada::kicad_geometry
{

// undo 快照记录（探路报告 E.3b 方案 b）：记录持有 item*（RTree 非拥有，Remove 不 delete），
// image = item->Clone() 的深拷贝。MODIFY 用单一镜像槽：SwapItemData 是对调语义
// （item↔image 双向交换），undo/redo 对同一镜像反复调用即来回切换（B3a-1 实测：
// 双槽 before/after 经一轮 undo+redo 后镜像内容互换→还原失效——单槽修正）。
struct InteractionUndo
{
    enum class Kind
    {
        MODIFY,    // 几何/文本改动：undo=SwapItemData(image)+Update；redo=同镜像再 Swap
        ADD,       // 放置/补点：undo=Remove(item,false)；redo=Append(item,false)
        REMOVE,    // 删除：undo=Append(item,false)；redo=Remove(item,false)
        CHAIN_ADD  // B3c 画线链（整条一次撤销）：undo=逐段 Remove；redo=逐段 Append
    };

    Kind        kind = Kind::MODIFY;
    SCH_ITEM*   item = nullptr;   // 记录持有；栈清空/truncate 时才释放
    SCH_ITEM*   image = nullptr;  // Clone 镜像（MODIFY：单槽对调；ADD/REMOVE：null）
    SCH_SCREEN* screen = nullptr;
    std::vector<SCH_ITEM*> chain; // CHAIN_ADD：链上各段（记录持有指针，不 delete）

    ~InteractionUndo();
    InteractionUndo() = default;
    InteractionUndo( const InteractionUndo& ) = delete;            // 记录即所有权，禁拷贝
    InteractionUndo& operator=( const InteractionUndo& ) = delete;
    InteractionUndo( InteractionUndo&& aOther ) noexcept;
    InteractionUndo& operator=( InteractionUndo&& aOther ) noexcept;
};

// 活文档 + 交互原子操作集合。装配模板 = connection_smoke_test.cpp:22-64（B2 已实证）。
class SchInteractionEngine
{
public:
    SchInteractionEngine();
    ~SchInteractionEngine();

    SchInteractionEngine( const SchInteractionEngine& ) = delete;
    SchInteractionEngine& operator=( const SchInteractionEngine& ) = delete;

    SCHEMATIC&       schematic() { return sch_; }
    SCH_SHEET_PATH&  sheetPath() { return path_; }
    SCH_SCREEN*      screen() { return screen_; }

    // ── 命中（探路报告 A.3 窄查序列）──
    // 返回命中项；命中 pin 时升级为父符号（aPinOwner 输出持有者，可为空）。
    SCH_ITEM* HitTest( const VECTOR2I& aPtIU, int aAccIU, SCH_SYMBOL** aPinOwner = nullptr );

    // 可选白名单（报告 D.2）：替代不存在的 IsHitTestable/IsSelectable
    bool EngineSelectable( const SCH_ITEM* aItem ) const;

    // ── 选择集（壳层自研集合，非拷贝 KiCad SELECTION）──
    void SetSelection( const std::vector<SCH_ITEM*>& aItems ) { selection_ = aItems; }
    const std::vector<SCH_ITEM*>& Selection() const { return selection_; }
    void ClearSelection() { selection_.clear(); }

    // ── 框选（报告 D.2）：v1 固定 contained 语义（两角→BOX2I(pos,size)）──
    std::vector<SCH_ITEM*> BoxSelect( const VECTOR2I& aCorner1, const VECTOR2I& aCorner2,
                                      bool aContained = true ) const;

    // ── 拖移（报告 B.2 最小等价）：aDeltaIU 已吸附（SnapGrid∘SnapEndpoints 由调用方完成）──
    // 逐项 Move + 注册 undo(MODIFY)，然后 CommitEdits；返回被改动的项。
    std::vector<SCH_ITEM*> MoveSelection( const VECTOR2I& aDeltaIU );

    // ── 端点拖（报告 C.1/C.4）：把线一端移到 aPtIU（调用方已吸附），落位后补点+重算 ──
    void MoveLineEndpoint( SCH_LINE* aLine, bool aStart, const VECTOR2I& aPtIU );

    // ── 提交单点（铁律 1/3 + C.3 补点 + Recalculate）──
    // changed 项 screen->Update → GetNeededJunctions 补点（每点注册 undo ADD）→ Recalculate
    void CommitEdits( const std::deque<SCH_ITEM*>& aChanged );

    // ── 删除（报告 F.3）：注册 undo(REMOVE)后摘树；对象由记录持有 ──
    void DeleteSelection();

    // ── undo/redo（报告 E.3b；MODIFY 单镜像槽对调语义）──
    void StageUndo( InteractionUndo::Kind aKind, SCH_ITEM* aItem, SCH_SCREEN* aScreen,
                    SCH_ITEM* aImage = nullptr );
    void StageUndoChain( const std::vector<SCH_ITEM*>& aItems, SCH_SCREEN* aScreen );
    bool CanUndo() const { return undoPos_ > 0; }
    bool CanRedo() const { return undoPos_ < undoStack_.size(); }
    bool Undo();
    bool Redo();
    void ClearUndo();

    // ── 放置（B3b）──
    void EnsureBuiltinLibrary();
    void ListBuiltinSymbols( std::vector<wxString>& aNamesOut ) const;
    // 创建未入屏符号实例（每实例 Flatten 深拷贝，模板不变；REF="R?"；调用方设置姿态后 PlaceSymbol）
    SCH_SYMBOL* CreateSymbol( const wxString& aLibName, const VECTOR2I& aPosIU );
    // 落位：分配位号（REFDES_TRACKER，undo 不回收号）+ Append + undo(ADD) + Recalculate
    void PlaceSymbol( SCH_SYMBOL* aSymbol );

    // ── 文档装配辅助（B3a-2：SeedDemoScreen/build 校验用）──
    void RefreshConnections();                       // Recalculate（内部含 TestDanglingEnds）
    void ScanRefDes();                               // 已标注符号重扫进位号 tracker
    void ClearDocument();                            // K6：清空文档（undo/选择/所有 item），供文件装载

    // ── M1b：外部库模板注册（.kicad_sym 解析产物；引擎接管所有权）──
    // 重名（含内置 R/C/LED）→ false（不覆盖）；aRefPrefix = Reference 字母前缀（位号分配）。
    bool RegisterLibTemplate( std::unique_ptr<LIB_SYMBOL> aTpl, const wxString& aRefPrefix,
                              const wxString& aLibId = wxEmptyString );
    // M1e-1：注销已注册键（/lib/synthesize 同键覆盖用；键不在 → no-op）
    void UnregisterLibTemplate( const wxString& aLibId );
    void ListLibrarySymbols( std::vector<wxString>& aNamesOut ) const;   // 内置 + 已注册
    size_t LibSymbolPinCount( const wxString& aLibName ) const;          // 0 = 不存在
    // M1b 键模型（docs/09 §1）：实例 lib_id → 引擎实际键——唯一权威归一规则
    // （字面键命中 → name-only 唯一回退 → 歧义/无命中保持字面）。scene 序列化与
    // 保存端共用，禁止他处再造键规则。
    wxString CanonicalLibKey( const wxString& aLiteral, const wxString& aItemName ) const;

    // ── B3c 画线链 ──
    // vertices = [起点, 拐点…, 终点]；追加 N×2点 SCH_LINE 链（段间端点相接），
    // 整条一次 CommitEdits（链末补点+Recalculate），undo=批量组（CHAIN_ADD，一次撤整条）。
    void CommitWireChain( const std::vector<VECTOR2I>& aVertices );

    // ── M1a G5：pin-到-pin 语义连接（人机同语义，与 DSH connect_pins 对齐）──
    // endpoints = [["R2.1","R3.1"], …]（refdes.pin 字符串对）；每对独立画 L 型连接：
    // 端点解析（库引脚物理坐标）→ 路由（v1：水平优先 L 型；同轴则为直线）→
    // CommitWireChain（补点 + 单条 CHAIN_ADD undo）。失败填 aError（端点缺失/重复/无变化）。
    bool ConnectPins( const std::vector<std::pair<wxString, wxString>>& aEndpoints,
                      wxString* aError = nullptr );

    // K6 写回：按名称取内置库模板（保存时合成 lib_symbols 体；无则为 nullptr）
    const LIB_SYMBOL* FindBuiltinLibSymbol( const wxString& aLibName ) const;

private:
    void BuildBuiltinLibrary();

    // 活文档（声明序即构造序：prj_ 先于 sch_）
    PROJECT       prj_;
    SCHEMATIC     sch_;
    SCH_SHEET_PATH path_;
    SCH_SCREEN*   screen_ = nullptr;

    std::vector<SCH_ITEM*> selection_;
    std::vector<InteractionUndo> undoStack_;
    size_t  undoPos_ = 0;

    /** 注册表项 = (libId 键, 模板)。键 = `category:name`（M1b 精选库键模型，
     *  docs/09）："R:R"/"C:C"/"LED:LED"/"R:R_Small"/"POWER:+5V"… 键是唯一寻址值，
     *  模板内部 name 保持官方真名。 */
    std::vector<std::pair<wxString, std::unique_ptr<LIB_SYMBOL>>> libTemplates_;
    std::map<std::string, std::string>        libPrefix_; // libName → 位号前缀（R/C/D）
    std::shared_ptr<REFDES_TRACKER>           tracker_;
    bool libBuilt_ = false;
};

} // namespace cicada::kicad_geometry
