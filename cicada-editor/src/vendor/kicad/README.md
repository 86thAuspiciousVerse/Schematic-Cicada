# kiCAD 抽取闭包（Cicada-owned 裁剪拷贝）

本目录是从 KiCad 官方 **10.0.6 tag**（GitHub 镜像 commit `caf7377e9cb6fa1535ec3596dcb8c99bf44a996e`，
官方 GitLab `refs/tags/10.0.6`）选择性拷贝并按 Schematic-Cicada 白名单裁剪的源码闭包。

## 纪律

- `kicad/`（工作区参考树）永远只读；本目录是唯一允许改动/裁剪的拷贝区。
- 每次拷贝：登记来源路径、原始行数、裁剪理由、本地改动（见下"拷贝登记"）。
- 保留上游版权头；许可证 = **GPLv3-or-later**（`kicad/LICENSE.README`；源文件头多为
  GPL-2.0-or-later，有效许可取 GPLv3-or-later）。产品编辑器内核因此整体 GPL 开源。
- 白名单 fail-closed 由上层（`cicada_screen_adapter`/`tool_registry`/writer）负责；
  本目录只提供 KiCad 语义内核，不扩展白名单外图元。

## 子目录

| 子目录 | 来源 | 说明 |
|---|---|---|
| `include/` | `kicad/include`（kicommon 头，精选） | EDA_ITEM/KIID/LAYER_IDS/… 只拷被依赖的头 |
| `common/` | `kicad/common`（精选 .cpp） | kicommon 实现，编译驱动裁剪 |
| `kimath/` | `kicad/libs/kimath`（全量） | 纯数学/几何底座，零 boost 零 wx |
| `core/` | `kicad/libs/core`（全量） | kicad_algo/typeinfo 等 |
| `sexpr/` | `kicad/libs/sexpr`（全量） | S-expression 底座（与上层 `vendor/sexpr` 同源） |
| `eeschema/` | `kicad/eeschema`（白名单图元+传递依赖） | SCH_ITEM/SCH_LINE/…/SCH_SCREEN/RTREE/SHEET_PATH |
| `thirdparty/` | `kicad/thirdparty`（nlohmann_json 等 bundled） | header-only 第三方 |

## 拷贝登记

| 批次 | 源路径（kicad/，10.0.6=caf7377e） | 数量 | 裁剪说明 | 负责人 | 日期 |
|---|---|---|---|---|---|
| B1 | `libs/kimath`、`libs/core`、`libs/sexpr`（全量） | 115 文件 | 纯数学/几何/工具底座，零依赖裁剪 | 主代理 | 2026-09-03 |
| B2 | include 依赖闭包（collect-headers.mjs 产出） | 133 头 | 从 9 个种子头（sch_item/line/junction/no_connect/label/screen/rtree/sheet/sheet_path）递归收集的 KiCad 内部头 | 主代理 | 2026-09-03 |
| B3 | `eeschema/sch_{item,line,junction,no_connect,label,screen,sheet,sheet_path}.cpp` | 8 | 切片 1 数据模型 .cpp | 主代理 | 2026-09-03 |
| B4 | `thirdparty/{dynamic_bitset,rtree,nlohmann_json,clipper2}` 所需源码 | 12 | bundled 单头/算法，随源拷走 | 主代理 | 2026-09-03 |
| B5 | config.h（生成式） | 1 | `scripts/generate-config.mjs` 从 `cmake/config.h.cmake` 转最小版（@宏→off） | 主代理 | 2026-09-03 |
| B6 | 闭包补全（种子=9 头+8 cpp 重跑收集器） | 135 | .cpp 的 include 闭包：kicommon 头补全 + pgm_base/id/advanced_config 等 | 主代理 | 2026-09-03 |
| B7 | 编译驱动补拷：`include/wx_filename.h`、`eeschema/sim/*.h`、`thirdparty/clipper2`（8 头+src）、`thirdparty/pegtl` | ~460 | wx_filename 为 KiCad 自有头；sim 仅头（.cpp 不入目标）；clipper2 全量（含 src 编译）；pegtl 单头版 | 主代理 | 2026-09-03 |
| B8 | 编译宏：`USINGZ=1`（Clipper2 z 成员/回调）与 `_USE_MATH_DEFINES=1` | — | KiCad 官方构建同宏；`/utf-8` 为 fmt 硬要求 | 主代理 | 2026-09-03 |

本地改动（裁剪登记）：

| 文件 | 改动 | 理由 |
|---|---|---|
| `eeschema/sch_line.cpp` | 删除 `Serialize/Deserialize(protobuf::Any)` + api include ×3 | API 序列化属排除区；SCH_ITEM 基类无声明，无副作用 |
| `eeschema/sch_label.cpp` | 删除 4 组 `Serialize/Deserialize` + api include ×2 + `magic_enum.hpp`（未使用） | 同上 |
| `eeschema/sim/`、`include/dialogs/html_message_box.h` | 从拷贝树移除 | 仿真/对话框不在白名单，且无模型闭包引用 |

外部依赖（不拷贝，宿主平台提供）：Boost ≥1.71（header-only，`third_party/boost`）、wxWidgets 3.2.9 base/core/adv、MSVC C++20 标准库、Windows SDK。

切片 1 为编译驱动裁剪：未列出的 kicommon .cpp 将按链接错误逐批补齐（补时追加登记）。

| B9 | wave2b 真源批（~35 .cpp + 3 头）：sch_shape/symbol/string_utils/increment/refdes_utils/validators/layer_id/markup_parser/undo_redo_container/rc_item/env_vars/kicad_io_utils/nested_settings/config_params/bom_settings/remote_provider_settings/io_base/origin_transforms/json_conversions/color4d/gal/中文编码 | 45 | LNK 驱动的定义文件拷贝（wave2b 探路报告映射）；template_fieldnames 剪生成 lexer 段；gr_text 仅留笔宽函数 | 主代理 | 2026-09-04 |
| B10 | 底层类真源：commit/view/view_group/view_item/view_overlay/glyph /progress_reporter/dpi_scaling/serializable/sch_marker/erc{_item,_settings}/refdes_tracker/pin_type/symb_transforms_utils/kiplatform os/{windows,common} | 24 | 同上（ERC/MARKER 由 vendor connection_graph 的 ERC 引擎真实引用） | 主代理 | 2026-09-04 |
| B11 | thirdparty 补：fmt/src/{format,os}.cc（**排除 fmt.cc——C++20 module 接口**）、picosha2/picosha2.h、fast_float/include | 4 | fmt vformat 由 .cc 提供（GLOB 补 `*.cc`）；picosha2 为 remote_provider_settings | 主代理 | 2026-09-04 |
| B12 | 缺失头：include/{rc_json_schema,kicad_build_version(生成式),view/{view,view_group,view_item,view_overlay}(含被引链),widgets/progress_reporter_base,text_eval/text_eval_{parser,units},libeval/numeric_evaluator}.h | 10 | 岛扫描缺口；kicad_build_version.h 为手写最小生成版（WriteVersionHeader.cmake 模板） | 主代理 | 2026-09-04 |
| B13 | wave2b-stubs.cpp / wave2b-stubs2.cpp（CICADA 自建桩，非 KiCad 原件） | 2 | 排除区符号降级（proto/EMBEDDED_FILES/AUTOPLACE/SETTINGS_MGR 部分/TRANSACTION/EXPRESSION/GetMsgPanelInfo/Plot 族/KIFONT/GAL/PROJECT/SCH_COMMIT/static…）；每桩签名与 vendor 头逐字核对 | 主代理 | 2026-09-04 |

| B14 | 切片 2（渲染）真源六件：common/render_settings.cpp、eeschema/sch_render_settings.cpp（剪 GetShowPageLimits）、common/font/{font.cpp（剪 outline 分支+GAL Draw 对）,stroke_font.cpp}、common/newstroke_font.cpp+include/newstroke_font.h | 6 | 纯计算渲染链（RENDER_SETTINGS 的 Dash/Gap 协议 + STROKE_FONT 笔画字形全闭合；65,749L 为内嵌字形数据表）；字体锁 STROKE_FONT：零 FreeType/fontconfig/ft2build/GL/cairo（侦察已 grep 实证） | 主代理 | 2026-09-04 |

| B15 | 切片 3 真源：eeschema/sch_commit.cpp（剪版——Push/pushLibEdit/pushSchEdit/revertLibEdit/Revert/ctor×2 帧重载剪除，makeImage·undoLevelItem 的 lib-editor 分支剪除）| 1 | Stage×2/makeImage/undoLevelItem/ctor/dtor 真源化（wave2b-stubs2 对应 6 桩同批删除；Push/Revert 桩保留——帧语义判排除，vendored schematic.cpp 仍需其定义）| 主代理 | 2026-09-04 |
| B16 | PROJECT_FILE 链尾波五件：project/project_file.cpp（真实 PROJECT_FILE 启用——PROJECT stub ctor 修复引发）、project/{component_class_settings,tuning_profiles,board_project_settings}.cpp、settings/layer_settings_utils.cpp | 5 | SCHEMATIC::SetProject 首次真调牵出：PROJECT::GetProjectFile() 头内联体 *m_projectFile 需非空 → stub ctor 置 new PROJECT_FILE(wxEmptyString)；其 JSON_SETTINGS 子类与 PARAM_LAYER_* 定义逐波补齐 | 主代理 | 2026-09-04 |
| B17 | 切片 3 运行时前置修正（vendor 剪版小修，非新增文件）：common/pgm_base.cpp（InitPgm 恢复 settings 分配一行——wave2 剪版删除整块→m_settings_manager 恒 null→GetSettingsManager 空引用锁崩；B3a-1 栈实证）、common/wave2b-stubs2.cpp（SETTINGS_MANAGER/SCH_JUNCTION 族 ctor 补"真实默认构造"定义——上游 ctor 有定义而 vendor 未随文件带入，GetAppSettings<T> 模板锁定 mutex 已构造） | 2 | 壳层无 wxApp 生命周期 → 引擎代管 Pgm（CicadaPgm+裸 wxApp+InitPgm 单元测试模式）；GetAppSettings 未注册→wxFAIL→nullptr→GetDefaultFont 回落 KICAD_FONT_NAME（KiCad 自带回退）；REFDES_TRACKER::GetNextRefDes（简单版）上游头声明无定义（codegraph 全库核实）→壳层用 Contains/Insert 原语实现同语义 | 主代理 | 2026-09-05 |
