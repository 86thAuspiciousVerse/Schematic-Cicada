// cicada_document_bridge.cpp — K6 装载实现（见头文件）
#include "cicada_document_bridge.h"

#include "kicad_bridge/sch_interaction_engine.h"
#include "core/cicada_screen_adapter.h"
#include "core/sexpr_writer.h"
#include "core/sexpr_model.h"
#include <map>

#include <sch_line.h>#include <sch_label.h>
#include <sch_junction.h>
#include <sch_no_connect.h>
#include <sch_symbol.h>
#include <sch_pin.h>
#include <sch_field.h>
#include <lib_symbol.h>
#include <lib_id.h>
#include <template_fieldnames.h>
#include <layer_ids.h>
#include <eda_shape.h>
#include <sch_shape.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <sstream>

namespace cicada::editor::cicada_doc
{

namespace
{

// centroid-mm → IU（1 unit = 0.01mm = 100 IU；schIUScale 1mm=10000IU）
inline int toIu( int aCentiMm )
{
    return aCentiMm * 100;
}

// 解析 lib_id "Device:R" → 部件名 "R"（首字符大写保持；简化 split ':'）
wxString libItemName( const std::string& aLibId )
{
    const size_t pos = aLibId.rfind( ':' );
    const std::string name = pos == std::string::npos ? aLibId : aLibId.substr( pos + 1 );
    return wxString::FromUTF8( name.c_str() );
}

// M1b0：lib_id 归一等价匹配——nickname 视 device/空/cicada 为同族 + itemName 相同。
// 历史文件/引擎归一（device:→cicada:）造成的"实例有条目无"错配，以此容错。
bool lib_id_match( const std::string& a, const std::string& b )
{
    auto parts = []( const std::string& s ) -> std::pair<std::string, std::string>
    {
        const size_t pos = s.rfind( ':' );
        return pos == std::string::npos ? std::make_pair( std::string(), s )
                                        : std::make_pair( s.substr( 0, pos ), s.substr( pos + 1 ) );
    };
    auto norm = []( const std::string& n ) -> std::string
    {
        return ( n == "device" || n.empty() ) ? std::string( "cicada" ) : n;
    };
    const auto [ na, ia ] = parts( a );
    const auto [ nb, ib ] = parts( b );
    if( ia != ib )
        return false;
    return norm( na ) == norm( nb );
}

// 尾段同名兜底（唯一才认）：lib_id_match 只归一 device/空/cicada 同族，跨族错配
// （实例 cicada:X vs 条目 IC:X —— 模板道旧文件 / 数据手册道被改写）不覆盖。此处按
// item name 匹配；同名多键 = 无法判定，返回 nullptr（绝不猜）。
const LibSymbolRecord* unique_tail_record( const std::vector<LibSymbolRecord>& aRecords,
                                           const std::string&                  aLibId )
{
    auto tail = []( const std::string& s ) -> std::string
    {
        const size_t pos = s.rfind( ':' );
        return pos == std::string::npos ? s : s.substr( pos + 1 );
    };
    const std::string      want = tail( aLibId );
    const LibSymbolRecord* hit  = nullptr;
    for( const auto& lr : aRecords )
    {
        if( tail( lr.lib_id ) != want )
            continue;
        if( hit != nullptr && hit->lib_id != lr.lib_id )
            return nullptr;
        hit = &lr;
    }
    return hit;
}

} // namespace

bool LoadCicadaSchIntoEngine( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                              const wxString& aPath, wxString* aError,
                              cicada::editor::SchematicModel* aOutModel )
{
    std::ifstream input( std::filesystem::path( aPath.ToStdWstring() ), std::ios::binary );
    if( !input )
    {
        if( aError )
            *aError = wxString::Format( "cannot open %s", aPath );
        return false;
    }
    std::ostringstream contents;
    contents << input.rdbuf();

    // 解析（K1 白名单解析器，单测覆盖；失败不改动引擎）
    CicadaScreenAdapter parse;
    std::string         err;
    if( !parse.load( contents.str(), &err ) )
    {
        if( aError )
            *aError = wxString::FromUTF8( err.c_str() );
        return false;
    }
    const SchematicModel& model = parse.model();

    // 清空并重建（直调引擎公开面）
    aEngine.ClearDocument();

    SCH_SCREEN*      screen = aEngine.screen();
    SCH_SHEET_PATH&  path   = aEngine.sheetPath();

    // wires：每记录点列按相邻对成段（白名单导出按段存，兼容多点）
    for( const WireRecord& w : model.wires )
    {
        for( size_t i = 1; i < w.points.size(); ++i )
        {
            const int x1 = toIu( w.points[i - 1].first ), y1 = toIu( w.points[i - 1].second );
            const int x2 = toIu( w.points[i].first ),     y2 = toIu( w.points[i].second );
            if( x1 == x2 && y1 == y2 )
                continue;
            auto* line = new SCH_LINE( VECTOR2I( x1, y1 ), LAYER_WIRE );
            line->SetStartPoint( VECTOR2I( x1, y1 ) );
            line->SetEndPoint( VECTOR2I( x2, y2 ) );
            screen->Append( line, false );
        }
    }

    // labels
    for( const LabelRecord& l : model.labels )
    {
        auto* label = new SCH_LABEL( VECTOR2I( toIu( l.x_g ), toIu( l.y_g ) ),
                                     wxString::FromUTF8( l.text.c_str() ) );
        screen->Append( label, false );
    }

    // junctions
    for( const JunctionRecord& j : model.junctions )
        screen->Append( new SCH_JUNCTION( VECTOR2I( toIu( j.x_g ), toIu( j.y_g ) ) ), false );

    // no_connects
    for( const NoConnectRecord& n : model.no_connects )
        screen->Append( new SCH_NO_CONNECT( VECTOR2I( toIu( n.x_g ), toIu( n.y_g ) ) ), false );

    // symbols：模板 = 引擎已注册键（精确键优先 → name-only 唯一回退）否则通用模板
    // （文件自带 pin 表）；refdes/value 从模型写回
    for( const SymbolRecord& s : model.symbols )
    {
        const wxString name = libItemName( s.lib_id );
        // 键寻址与 ops place-symbol / docs/09 §1 同规则：整键交给引擎解析。传尾段会把
        // 精确键降级成 name-only（同名多键即歧义），且非内置键（datasheet 道 IC:xxx）
        // 会被误判为"引擎没有"。
        SCH_SYMBOL*    sym  = aEngine.CreateSymbol( wxString::FromUTF8( s.lib_id.c_str() ),
                                                   VECTOR2I( toIu( s.x_g ), toIu( s.y_g ) ) );
        if( !sym )
        {
            // 通用模板：建 LIB_SYMBOL + 模型 pin 表（局部坐标 centi-mm→IU）
            auto tpl = std::make_unique<LIB_SYMBOL>( name );
            // 从模型 lib record 找 pin 表（model.lib_symbols 按 lib_id 键）
            // 从模型 lib record 找 pin 表（lib_symbols 按 lib_id 匹配，vector；M1b0 归一容错）
            const LibSymbolRecord* libRec = nullptr;
            for( const LibSymbolRecord& lr : model.lib_symbols )
                if( lr.lib_id == s.lib_id || lib_id_match( lr.lib_id, s.lib_id ) )
                {
                    libRec = &lr;
                    break;
                }
            // 跨族错配（实例 cicada:X vs 条目 IC:X）再走尾段同名兜底——否则引擎里这个
            // 符号会 0 引脚：命中测试/连线吸附全部失灵（体仍在，故导出不受影响）。
            if( libRec == nullptr )
                libRec = unique_tail_record( model.lib_symbols, s.lib_id );
            if( libRec )
            {
                for( const LibPinRecord& pr : libRec->pins )
                {
                    auto* pin = new SCH_PIN( tpl.get() );
                    // KiCad 的 lib 引脚文本是 Y-down：解析器自己会取反（sch_io_kicad_sexpr_parser.h
                    // 的 parseXY(true)，lib 引脚用在同一文件 :1671），而 TRANSFORM() 默认是单位矩阵
                    // ⇒ 世界坐标 = at − lib_y。core 模型一直如此（connection_graph.cpp:50 的 -pin->y_g），
                    // 只有桥这条通路反了，导致 /scene 与 runtime/画布互相镜像（2026-09-13 实测：
                    // 37 个 NC 标记在 /scene 帧里 0 命中）。这里对齐 KiCad 语义。
                    pin->SetPosition( VECTOR2I( toIu( pr.x_g ), -toIu( pr.y_g ) ) );
                    pin->SetNumber( wxString::FromUTF8( pr.number.c_str() ) );
                    tpl->AddDrawItem( pin );
                }
            }
            // 文件键原样保留（绝不改写族名）：datasheet 道的 IC:xxx 在引擎无模板，而体
            // 就在文件自带 lib_symbols 里——把族名换成 cicada 会造出"实例键 ≠ 条目键"，
            // 写回/导出随即找不到体（2026-09-12 导出 500 的根因）。
            LIB_ID fileKey;
            fileKey.Parse( s.lib_id );
            sym = new SCH_SYMBOL( *tpl, fileKey, &path, 1 );
            sym->SetPosition( VECTOR2I( toIu( s.x_g ), toIu( s.y_g ) ) );
        }

        // 属性：refdes/value 写回（不占 tracker——装载后 ScanRefDes 统一重扫）
        sym->SetRef( &path, wxString::FromUTF8( s.refdes.c_str() ) );
        if( SCH_FIELD* v = sym->GetField( FIELD_T::VALUE ) )
            v->SetText( wxString::FromUTF8( s.value.c_str() ) );

        // 姿态（SYM_ORIENT_* 等价 rotation/90 逆时针步）
        switch( ( s.rotation / 90 ) % 4 )
        {
        case 1: sym->SetOrientation( SYM_ORIENT_90 ); break;
        case 2: sym->SetOrientation( SYM_ORIENT_180 ); break;
        case 3: sym->SetOrientation( SYM_ORIENT_270 ); break;
        default: break;
        }

        screen->Append( sym, false );
    }

    aEngine.RefreshConnections();
    aEngine.ScanRefDes();   // 已标注 refdes 进位号 tracker（后续放置不撞号）

    if( aOutModel )
        *aOutModel = model;   // 写回基底（lib_symbols/version/uuid/sheet_instances 保留）

    if( aError )
        aError->clear();
    return true;
}

} // namespace cicada::editor::cicada_doc

// ── K6 反向：引擎 → .cicada_sch（DSH 白名单方言；SexprModelWriter 复用）─────────
namespace cicada::editor::cicada_doc
{
namespace
{

// 收集内置库模板 → 合成 lib_symbols 体（几何=模板真值；格式对齐 DSH 金样）
bool synthesize_lib_body( const LIB_SYMBOL* aTpl, const std::string& aLibId,
                          std::string& aOut, wxString* aErr )
{
    if( !aTpl ) return false;
    const wxString name = aTpl->GetName();
    const std::string libName = aLibId;   // 完整 lib id（cicada:R）
    // 语义标志必须随保存回写保留（2026-09-08 验收实测缺陷）：丢掉 (power) 后，
    // DSH 语义层与 KiCad netlist 双双退回 Net-(C1-P1) 自动命名，GND/+5V 网络名丢失。
    const bool isPower = aTpl->IsPower();
    std::string refText = aTpl->GetReferenceField().GetText().ToUTF8().data();
    if( isPower && refText.empty() )
        refText = "#PWR";
    std::ostringstream out;
    out << "(symbol \"" << libName << "\"";
    if( isPower ) out << " (power)";
    out << " (pin_numbers (hide yes)) (pin_names (offset 0))";
    out << " (property \"Reference\" \"" << refText << "\" (at 2.032 0 90))";
    out << " (property \"Value\" \"" << name.ToUTF8().data() << "\" (at 0 0 90))";
    // 单元：体矩形 + 引脚（BuiltinLibrary 现态；单位 R_0_1/R_1_1 命名与 DSH 一致）
    out << " (symbol \"" << name.ToUTF8().data() << "_0_1\"";
    for( const auto& item : aTpl->GetDrawItems() )
    {
        if( item.Type() == SCH_SHAPE_T )
        {
            const auto* shape = static_cast<const SCH_SHAPE*>(
                static_cast<const SCH_ITEM*>( &item ) );
            if( shape->GetShape() != SHAPE_T::RECTANGLE ) continue;
            const VECTOR2I s0 = shape->GetStart(), e0 = shape->GetEnd();
            out << " (rectangle (start " << std::fixed << std::setprecision( 2 )
                << ( s0.x / 10000.0 ) << " " << ( s0.y / 10000.0 ) << ") (end "
                << ( e0.x / 10000.0 ) << " " << ( e0.y / 10000.0 ) << "))";
        }
    }
    out << ")";
    out << " (symbol \"" << name.ToUTF8().data() << "_1_1\"";
    for( const auto& item : aTpl->GetDrawItems() )
    {
        if( item.Type() != SCH_PIN_T ) continue;
        const auto* pin = static_cast<const SCH_PIN*>(
            static_cast<const SCH_ITEM*>( &item ) );
        const VECTOR2I p0 = pin->GetPosition();
        // 角度 = 真实朝向（PIN_ORIENTATION×90：RIGHT=0/UP=90/LEFT=180/DOWN=270
        // —— 与 KiCad 文件 at-angle 约定一致；曾用 y 侧启发式致左右引脚朝向错）
        const int angle = static_cast<int>( pin->GetOrientation() ) * 90;
        // 电气类型取真值（曾硬编码 passive：电源/输出引脚在 netlist 里全变 passive）。
        // 用 pin_type.h 的 inline 自由函数（SCH_PIN 的同名静态成员在抽取树里无实现）。
        const std::string pinType =
            GetCanonicalElectricalTypeName( pin->GetType() ).ToUTF8().data();
        out << " (pin " << ( pinType.empty() ? "passive" : pinType )
            << " line (at " << std::fixed << std::setprecision( 2 )
            << ( p0.x / 10000.0 ) << " " << ( p0.y / 10000.0 ) << " " << angle << ") (length "
            << ( pin->GetLength() / 10000.0 ) << ") (name \""
            << pin->GetName().ToUTF8().data() << "\") (number \""
            << pin->GetNumber().ToUTF8().data() << "\"))";
    }
    out << "))";
    aOut += out.str();
    (void) aErr;
    return true;
}

void collect_engine( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                     const cicada::editor::SchematicModel& aBase,
                     cicada::editor::SchematicModel& aModel, wxString* aErr )
{
    SCH_SCREEN*     screen = aEngine.screen();
    SCH_SHEET_PATH& path   = aEngine.sheetPath();

    // 保存端键归一：引擎实际键（EnsureBuiltinLibrary 幂等；规则唯一权威 = 引擎 CanonicalLibKey）
    aEngine.EnsureBuiltinLibrary();

    // 已用 lib_id 集合：新增符号（基底缺失/与 engine 不一致）→ 引擎模板合成兜底
    std::map<std::string, bool> libUsed;
    auto libBodyFor = [&]( const std::string& aLibId ) -> std::string
    {
        // 引擎模板单权威：已注册键（内置 + 库符号）→ 模板合成体（几何=真值；与 DSH
        // libEntryText 同为 bbox+引脚 外形，M1 ARC/折线 容忍）。模板缺位（引擎未挂该库
        // /未知符号）→ 基底同键/同族体；再没有 → 显式失败（绝不静默丢条目）。
        if( const LIB_SYMBOL* tpl = aEngine.FindBuiltinLibSymbol(
                wxString::FromUTF8( aLibId.c_str() ) ) )
        {
            std::string body;
            if( synthesize_lib_body( tpl, aLibId, body, aErr ) ) return body;
        }
        // M1b0：归一容错匹配（device:/cicada:/空 同族 + 同名）
        for( const auto& lr : aBase.lib_symbols )
            if( lr.lib_id == aLibId || lib_id_match( lr.lib_id, aLibId ) )
                return lr.body;
        // 尾段同名兜底（唯一才认）：同族规则只归一 device/空/cicada，跨族（如实例
        // cicada:NE555P vs 条目 IC:NE555P）不覆盖 → 旧的模板道文件写回/导出会 500。
        // 同名多键 = 无法判定 → 保持空（仍由调用方显式失败，不猜）。
        if( const LibSymbolRecord* tailHit = unique_tail_record( aBase.lib_symbols, aLibId ) )
            return tailHit->body;
        // 非内置 + 基底无 → 交给调用方报错（绝不静默丢条目——自传播损坏的根源）
        return {};
    };

    for( SCH_ITEM* it : screen->Items() )
    {
        switch( it->Type() )
        {
        case SCH_LINE_T:
        {
            const auto* line = static_cast<const SCH_LINE*>( it );
            if( line->GetLayer() != LAYER_WIRE ) break;   // 仅 wire 可序列化（其余上层图元跳过）
            const VECTOR2I s0 = line->GetStartPoint(), e0 = line->GetEndPoint();
            if( s0 == e0 ) break;
            aModel.wires.push_back( { { { s0.x / 100, s0.y / 100 }, { e0.x / 100, e0.y / 100 } },
                                      it->m_Uuid.AsString().ToUTF8().data() } );
            break;
        }
        case SCH_LABEL_T:
        case SCH_GLOBAL_LABEL_T:
        case SCH_HIER_LABEL_T:
        {
            const auto* label = static_cast<const SCH_LABEL_BASE*>( it );
            aModel.labels.push_back( { label->GetText().ToUTF8().data(), it->m_Uuid.AsString().ToUTF8().data(),
                                       it->GetPosition().x / 100, it->GetPosition().y / 100, 0 } );
            break;
        }
        case SCH_JUNCTION_T:
        {
            const auto* j = static_cast<const SCH_JUNCTION*>( it );
            aModel.junctions.push_back( { j->m_Uuid.AsString().ToUTF8().data(), j->GetPosition().x / 100,
                                          j->GetPosition().y / 100 } );
            break;
        }
        case SCH_NO_CONNECT_T:
        {
            const auto* n = static_cast<const SCH_NO_CONNECT*>( it );
            aModel.no_connects.push_back( { n->m_Uuid.AsString().ToUTF8().data(), n->GetPosition().x / 100,
                                            n->GetPosition().y / 100 } );
            break;
        }
        case SCH_SYMBOL_T:
        {
            const auto* sym = static_cast<const SCH_SYMBOL*>( it );
            const std::string itemName( sym->GetLibId().GetLibItemName().c_str() );
            const std::string nickname( sym->GetLibId().GetLibNickname().c_str() );
            // docs/09 §1：文件 lib_id = 引擎实际键（saveback 写规范键）。旧引用
            // （cicada:R / device:R / 空族 name-only 实例）经引擎唯一权威归一；歧义保持字面。
            const std::string literal( sym->GetLibId().Format().c_str() );
            const wxString    canonicalWx = aEngine.CanonicalLibKey(
                wxString::FromUTF8( ( literal.empty() ? nickname + ":" + itemName : literal ).c_str() ),
                sym->GetLibId().GetLibItemName() );
            const std::string normLib( canonicalWx.ToUTF8().data() );

            if( !libUsed[ normLib ] )
            {
                libUsed[ normLib ] = true;
                // lib_symbols 体：引擎模板合成（单权威：builtin + 已注册库符号）；未注册 → 基底
                const auto body = libBodyFor( normLib );
                if( body.empty() )
                {
                    // M1b0：绝不静默丢条目（自传播损坏根源）——显式失败告知调用方
                    if( aErr )
                        *aErr = wxString::Format(
                            "symbol %s has no lib_symbols body (no engine template; missing in base)",
                            normLib );
                    return;
                }
                aModel.lib_symbols.push_back( { normLib, {}, body } );
            }

            cicada::editor::SymbolRecord rec;
            rec.lib_id = normLib;
            rec.refdes = sym->GetRef( &path, false ).ToUTF8().data();
            if( const SCH_FIELD* v = sym->GetField( FIELD_T::VALUE ) )
                rec.value = v->GetText().ToUTF8().data();
            rec.uuid = sym->m_Uuid.AsString().ToUTF8().data();
            rec.x_g = sym->GetPosition().x / 100;
            rec.y_g = sym->GetPosition().y / 100;
            switch( sym->GetOrientation() )
            {
            case SYM_ORIENT_90:  rec.rotation = 90;  break;
            case SYM_ORIENT_180: rec.rotation = 180; break;
            case SYM_ORIENT_270: rec.rotation = 270; break;
            default:             rec.rotation = 0;   break;
            }
            for( const auto& f : sym->GetFields() )
            {
                const std::string fname( f.GetName().ToUTF8().data() );
                if( fname == "Reference" || fname == "Value" ) continue;   // writer 由 refdes/value 推导
                rec.properties[ fname ] = f.GetText().ToUTF8().data();
            }
            for( const SCH_PIN* p : sym->GetLibPins() )
                rec.pins.push_back( { p->GetNumber().ToUTF8().data(), {} } );   // 实例 pin uuid 无权威源，v1 省略
            aModel.symbols.push_back( std::move( rec ) );
            break;
        }
        default:
            break;   // 其他图元（TEXT/SHEET/…）不序列化
        }
    }

    // sheet_instances：基底已有则原样保留（writer 要求非空 body 才输出）
    if( !aModel.sheet_instances_body.empty() )
        aModel.has_sheet_instances = true;
    (void) aErr;
}

} // namespace

namespace
{

// v4 随机 uuid（缺失实例/引脚 uuid 的导出兜底；KiCad 可读）
std::string random_uuid()
{
    std::random_device rd;
    char buf[ 37 ];
    std::snprintf( buf, sizeof( buf ), "%08x-%04x-4%03x-%04x-%012x", rd(), rd() & 0xffff,
                   rd() & 0xfff, rd() & 0xffff, rd() );
    return buf;
}

// G（0.01mm）→ mm 两位小数（KiCad 文件惯例）
std::string g_mm( int aG )
{
    char buf[ 32 ];
    std::snprintf( buf, sizeof( buf ), "%.2f", aG / 100.0 );
    return buf;
}

} // namespace

namespace
{

// 直接全量写（二进制截断；失败抛 std::runtime_error 由调用方 try 包住）
bool write_text_file( const wxString& aPath, const std::string& aText )
{
    std::ofstream output( std::filesystem::path( aPath.ToStdWstring() ),
                          std::ios::binary | std::ios::trunc );
    if( !output )
        throw std::runtime_error( "unable to open schematic file for writing" );
    output.write( aText.data(), static_cast<std::streamsize>( aText.size() ) );
    if( !output )
        throw std::runtime_error( "unable to write schematic file" );
    return true;
}

} // namespace

bool SaveCicadaSchIntoFile( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                            const cicada::editor::SchematicModel& aBase,
                            const wxString& aPath, wxString* aError )
{
    try
    {
        // 基底：版本/生成器/uuid/lib_symbols/sheet_instances 保留；条目从引擎现态收集
        cicada::editor::SchematicModel model = aBase;
        model.wires.clear();
        model.labels.clear();
        model.junctions.clear();
        model.no_connects.clear();
        model.symbols.clear();
        model.lib_symbols.clear();
        wxString collectErr;
        collect_engine( aEngine, aBase, model, &collectErr );
        if( !collectErr.empty() )
        {
            if( aError )
                *aError = collectErr;
            return false;
        }

        const std::string text = SexprModelWriter().write( model );

        // 直接全量写（DSH watcher 触发 user_edit；竞态/原子性权衡见施工日志）
        if( aError )
            aError->clear();
        write_text_file( aPath, text );
        return true;
    }
    catch( const std::exception& ex )
    {
        if( aError )
            *aError = wxString::FromUTF8( ex.what() );
        return false;
    }
}

bool ExportKicadSchIntoFile( cicada::kicad_geometry::SchInteractionEngine& aEngine,
                             const cicada::editor::SchematicModel& aBase,
                             const wxString& aPath, wxString* aError )
{
    try
    {
        // 复用收集：键归一 + 库体（引擎模板单权威）与保存端同规则
        cicada::editor::SchematicModel model = aBase;
        model.wires.clear();
        model.labels.clear();
        model.junctions.clear();
        model.no_connects.clear();
        model.symbols.clear();
        model.lib_symbols.clear();
        wxString collectErr;
        collect_engine( aEngine, aBase, model, &collectErr );
        if( !collectErr.empty() )
        {
            if( aError )
                *aError = collectErr;
            return false;
        }

        std::ostringstream out;
        out << "(kicad_sch\n"
            << "\t(version 20250610)\n"
            << "\t(generator \"eeschema\")\n"
            << "\t(generator_version \"10.0\")\n"
            << "\t(uuid \"" << ( model.uuid.empty() ? random_uuid() : model.uuid ) << "\")\n"
            << "\t(paper \"A4\")\n";
        out << "\t(lib_symbols\n";
        for( const auto& ls : model.lib_symbols )
            out << "\t\t" << ls.body << "\n";
        out << "\t)\n";

        auto uuidOr = []( const std::string& aUuid ) { return aUuid.empty() ? random_uuid() : aUuid; };
        for( const auto& j : model.junctions )
            out << "\t(junction (at " << g_mm( j.x_g ) << " " << g_mm( j.y_g )
                << ") (diameter 0) (color 0 0 0 0) (uuid \"" << uuidOr( j.uuid ) << "\"))\n";
        for( const auto& n : model.no_connects )
            out << "\t(no_connect (at " << g_mm( n.x_g ) << " " << g_mm( n.y_g )
                << ") (uuid \"" << uuidOr( n.uuid ) << "\"))\n";
        for( const auto& w : model.wires )
        {
            if( w.points.size() < 2 )
                continue;
            out << "\t(wire (pts";
            for( const auto& p : w.points )
                out << " (xy " << g_mm( p.first ) << " " << g_mm( p.second ) << ")";
            out << ") (stroke (width 0) (type default)) (uuid \"" << uuidOr( w.uuid ) << "\"))\n";
        }
        for( const auto& l : model.labels )
            out << "\t(label \"" << l.text << "\" (at " << g_mm( l.x_g ) << " " << g_mm( l.y_g )
                << " 0) (effects (font (size 1.27 1.27)) (justify left bottom)) (uuid \""
                << uuidOr( l.uuid ) << "\"))\n";

        for( const auto& s : model.symbols )
        {
            const std::string px = g_mm( s.x_g );
            const std::string py = g_mm( s.y_g );
            out << "\t(symbol (lib_id \"" << s.lib_id << "\") (at " << px << " " << py << " "
                << s.rotation << ") (unit 1) (in_bom yes) (on_board yes) (dnp no)\n";
            out << "\t\t(uuid \"" << uuidOr( s.uuid ) << "\")\n";
            auto prop = [&]( const char* aName, const std::string& aValue )
            {
                // 位置随意（KiCad 渲染用 at 定位；值为符号位 + 偏移避免堆叠）
                static int k = 0;
                out << "\t\t(property \"" << aName << "\" \"" << aValue << "\" (at " << px
                    << " " << py << " 0) (effects (font (size 1.27 1.27))))\n";
                (void) k;
            };
            prop( "Reference", s.refdes );
            prop( "Value", s.value );
            for( const auto& f : s.properties )
                if( f.first != "Reference" && f.first != "Value" )
                    prop( f.first.c_str(), f.second );
            for( const auto& p : s.pins )
                out << "\t\t(pin \"" << p.number << "\" (uuid \"" << random_uuid() << "\"))\n";
            out << "\t)\n";
        }

        out << "\t(sheet_instances (path \"/\" (page \"1\")))\n";
        out << ")\n";

        if( aError )
            aError->clear();
        return write_text_file( aPath, out.str() );
    }
    catch( const std::exception& ex )
    {
        if( aError )
            *aError = wxString::FromUTF8( ex.what() );
        return false;
    }
}

} // namespace cicada::editor::cicada_doc

