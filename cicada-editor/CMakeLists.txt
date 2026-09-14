cmake_minimum_required(VERSION 3.24)
project(cicada_editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CICADA_EDITOR_BUILD_TESTS "Build contract tests" OFF)
option(CICADA_EDITOR_ENABLE_WEBVIEW "Link wxWebView backend (requires a working WebView2/IE backend)" OFF)
option(CICADA_EDITOR_ENABLE_CANVAS "Compile the wx canvas panel into the desktop shell" OFF)
option(CICADA_EDITOR_BUILD_WIN32_PROBE "Build a temporary wx-independent Win32 startup probe" OFF)
option(CICADA_EDITOR_ENABLE_KICAD_PROBE
  "Compile-only probe for the KiCad SCH item/screen header closure" OFF)
option(CICADA_EDITOR_ENABLE_KICAD_MODEL
  "Build the cicada_kicad_model data-model closure (slice 1 of the KiCad extraction)" OFF)
# 切片 1（KiCad 数据模型闭包）的依赖根：Boost 与 wx 均为项目内第三方
set(CICADA_BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/boost"
  CACHE PATH "Boost source root (header-only)")
set(CICADA_WX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/wxWidgets"
  CACHE PATH "wxWidgets project checkout")
set(CICADA_WX_LIBS "" CACHE STRING "wxWidgets libraries to link for the model closure")
set(CICADA_KICAD_SOURCE_DIR "" CACHE PATH
  "KiCad source checkout used by CICADA_EDITOR_ENABLE_KICAD_PROBE")
set(CICADA_KICAD_BINARY_DIR "" CACHE PATH
  "KiCad configured build directory supplying generated headers")
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/wxWidgets")
  if(NOT DEFINED wxWidgets_ROOT_DIR)
    set(wxWidgets_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/wxWidgets" CACHE PATH "Project wxWidgets checkout")
  endif()
  if(NOT DEFINED wxWidgets_LIB_DIR)
    set(wxWidgets_LIB_DIR "${wxWidgets_ROOT_DIR}/lib/vc_x64_lib" CACHE PATH "Project wxWidgets libraries")
  endif()
  if(NOT DEFINED wxWidgets_CONFIGURATION)
    set(wxWidgets_CONFIGURATION mswu CACHE STRING "wxWidgets configuration")
  endif()
  if(NOT DEFINED wxWidgets_USE_STATIC)
    set(wxWidgets_USE_STATIC ON CACHE BOOL "Use static wxWidgets libraries")
  endif()
endif()
if(CICADA_EDITOR_ENABLE_WEBVIEW)
  find_package(wxWidgets COMPONENTS core base webview QUIET)
else()
  # Keep the default desktop shell independent of wxWebView's legacy ActiveX
  # module.  Some static wx builds initialise that module before OnInit and
  # crash on modern Windows even when no webview is requested at runtime.
  find_package(wxWidgets COMPONENTS core base QUIET)
endif()

if(MSVC)
  # 自研源含中文注释：/utf-8 防 GBK 码页吞换行（全角括号字节当 GBK 前导）
  add_compile_options(/utf-8)
  # Debug 配置：多 CL 并发写同一 .pdb 需要 /FS（C1041，model 库大编译单元并行时必现）
  add_compile_options($<$<CONFIG:Debug>:/FS>)
  # 大编译单元（connection_graph.cpp 等）Debug 下节数超限：C1128 → /bigobj（KiCad 官方 MSVC 同款）
  add_compile_options(/bigobj)
  # 排障符号：无条件 /Zi + /DEBUG（Debug 亦为默认值；Release 由此产出 .pdb 供崩溃栈解析）
  add_compile_options(/Zi)
  add_link_options(/DEBUG)
endif()

add_library(cicada_editor_core STATIC
  src/app/editor_controller.cpp
  src/app/host_spawner.cpp
  src/app/host_process.cpp
  src/app/stdout_parser.cpp
  src/bridge/editor_http_client.cpp
  src/bridge/event_dispatcher.cpp
  src/bridge/editor_ws_client.cpp
  src/bridge/winhttp_transport.cpp
  src/bridge/winhttp_websocket.cpp
  src/core/editor_contract.cpp
  src/core/cicada_screen_adapter.cpp
  src/core/kicad_sexpr_adapter.cpp
  src/core/sexpr_model.cpp
  src/core/sexpr_writer.cpp
  src/core/connection_graph.cpp
  src/core/document_editor.cpp
  src/core/tool_registry.cpp
  src/render/canvas_placeholder.cpp
  src/render/canvas_document.cpp
  src/render/canvas_scene.cpp
  src/vendor/sexpr/sexpr.cpp
  src/vendor/sexpr/sexpr_parser.cpp
)
target_include_directories(cicada_editor_core PUBLIC src src/vendor/sexpr/include)
if(WIN32)
  target_link_libraries(cicada_editor_core PRIVATE winhttp)
endif()

if(wxWidgets_FOUND)
  include(${wxWidgets_USE_FILE})
  # wxIMPLEMENT_APP supplies WinMain on Windows; mark this as a GUI target so
  # MSVC links the Windows subsystem entry point instead of requiring main().
  add_executable(cicada-editor WIN32 src/app/main.cpp)
  if(CICADA_EDITOR_ENABLE_CANVAS)
    target_sources(cicada-editor PRIVATE src/render/wx_canvas_panel.cpp)
    target_compile_definitions(cicada-editor PRIVATE CICADA_EDITOR_HAS_CANVAS=1)
  endif()
  if(CICADA_EDITOR_ENABLE_CANVAS AND CICADA_EDITOR_ENABLE_KICAD_MODEL)
    # 切片 2 几何批次 → 画布演示（真实 KiCad 几何；kicad_scene_adapter 非 vendor）
    target_sources(cicada-editor PRIVATE src/kicad_bridge/kicad_scene_adapter.cpp)
    # K6：.cicada_sch ↔ 引擎文档桥（旧白名单解析器 × 新引擎重建）
    target_sources(cicada-editor PRIVATE src/kicad_bridge/cicada_document_bridge.cpp)
    target_sources(cicada-editor PRIVATE src/bridge/webview2_host.cpp)
    target_compile_definitions(cicada-editor PRIVATE CICADA_EDITOR_HAS_WEBVIEW2=1)
    target_include_directories(cicada-editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview2/include)
    if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview2/lib/WebView2LoaderStatic.lib")
      target_link_libraries(cicada-editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview2/lib/WebView2LoaderStatic.lib")
    endif()
    target_compile_definitions(cicada-editor PRIVATE CICADA_EDITOR_HAS_KICAD_MODEL=1)
    if(MSVC)
      target_compile_options(cicada-editor PRIVATE /utf-8)
    endif()
    target_link_libraries(cicada-editor PRIVATE cicada_kicad_geometry cicada_kicad_model)
    if(WIN32 AND EXISTS "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib")
      target_link_libraries(cicada-editor PRIVATE
        "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxbase32u.lib"
        "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_core.lib"
        "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_adv.lib"
        "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxpng.lib"
        "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxtiff.lib"
        "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxjpeg.lib"
        "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxzlib.lib"
        "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxregexu.lib"
        "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxexpat.lib"
        "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_aui.lib"
        "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_propgrid.lib"
        comctl32 rpcrt4 advapi32 comdlg32 gdi32 ole32 oleaut32 shell32 shlwapi
        user32 version winmm winspool ws2_32)
    endif()
  endif()
  if(CICADA_EDITOR_ENABLE_WEBVIEW)
    target_compile_definitions(cicada-editor PRIVATE CICADA_EDITOR_HAS_WEBVIEW=1)
  endif()
  target_link_libraries(cicada-editor PRIVATE cicada_editor_core ${wxWidgets_LIBRARIES})
  if(WIN32)
    target_link_libraries(cicada-editor PRIVATE winhttp)
  endif()
else()
  message(STATUS "wxWidgets not found: contract/core library remains available for CI and scaffolding.")
endif()

if(CICADA_EDITOR_BUILD_TESTS)
  enable_testing()
  add_executable(cicada-editor-contract-tests tests/unit/contract_test.cpp)
  target_link_libraries(cicada-editor-contract-tests PRIVATE cicada_editor_core)
  # Contract tests use the standard assert() primitive.  Keep assertions
  # active even for Release/RelWithDebInfo so a green CTest run means checks
  # actually executed rather than merely returning from main().
  if(MSVC)
    target_compile_options(cicada-editor-contract-tests PRIVATE /UNDEBUG)
  else()
    target_compile_options(cicada-editor-contract-tests PRIVATE -UNDEBUG)
  endif()
  add_test(NAME cicada-editor-contract COMMAND cicada-editor-contract-tests)
endif()

# ── 切片 1 DoD 测试（需要 KiCad 模型闭包）──
if(CICADA_EDITOR_BUILD_TESTS AND CICADA_EDITOR_ENABLE_KICAD_MODEL)
  add_executable(kicad-model-slice1-test tests/slice1/rtree_hit_test.cpp)
  target_link_libraries(kicad-model-slice1-test PRIVATE cicada_kicad_model)
  if(WIN32 AND EXISTS "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib")
    target_link_libraries(kicad-model-slice1-test PRIVATE
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxbase32u.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_core.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_adv.lib"
      # wx 官方构建自带的编解码/正则/xml 依赖库：wxpng/wxtiff/wxjpeg 由 wxImage 的
      # 所有处理器（image.obj 无条件引用）拉入；wxzlib/wxregexu/wxexpat 同理
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxpng.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxtiff.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxjpeg.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxzlib.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxregexu.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxexpat.lib"
      # aui/propgrid：cicada 构建只产出了 base/core/adv+编解码；同源码树的默认构建
      # 目录产出全套（含 aui/propgrid——wxPGChoices/wxAuiPaneInfo 被 settings/属性
      # 代码真实引用）。两库均不用 OLE，与 cicada 构建 ABI 兼容（同一 wx 源码版本）。
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_aui.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_propgrid.lib"
      # wxMSW 静态库的系统依赖：core/adv 的对象拉入 comctl32(ImageList/InitCommonControls)
      # 与 rpcrt4(UuidCreate/UuidToStringW/RpcStringFreeW) 等；补齐标准集合避免后续拉入新
      # wx 对象时再次出现 LNK1120 假象
      comctl32 rpcrt4 advapi32 comdlg32 gdi32 ole32 oleaut32 shell32 shlwapi
      user32 version winmm winspool ws2_32)
  endif()
  if(MSVC)
    target_compile_options(kicad-model-slice1-test PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(kicad-model-slice1-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME kicad-model-slice1 COMMAND kicad-model-slice1-test)

  # ── 切片 2 渲染冒烟（B3：纯计算渲染内核：RENDER_SETTINGS/STROKE_FONT/Stroke/GetTextBox）
  add_executable(kicad-model-slice2-test tests/slice2/render_smoke_test.cpp)
  target_link_libraries(kicad-model-slice2-test PRIVATE cicada_kicad_model)
  if(WIN32 AND EXISTS "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib")
    target_link_libraries(kicad-model-slice2-test PRIVATE
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxbase32u.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_core.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_adv.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxpng.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxtiff.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxjpeg.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxzlib.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxregexu.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxexpat.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_aui.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_propgrid.lib"
      comctl32 rpcrt4 advapi32 comdlg32 gdi32 ole32 oleaut32 shell32 shlwapi
      user32 version winmm winspool ws2_32)
  endif()
  if(MSVC)
    target_compile_options(kicad-model-slice2-test PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(kicad-model-slice2-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME kicad-model-slice2 COMMAND kicad-model-slice2-test)

  # ── 切片 2 B4：几何批次桥（自研，非 vendor）──
  add_library(cicada_kicad_geometry STATIC src/kicad_bridge/sch_geometry_batch.cpp src/kicad_bridge/snap_utils.cpp src/kicad_bridge/sch_interaction_engine.cpp)
  target_include_directories(cicada_kicad_geometry PUBLIC src)
  target_link_libraries(cicada_kicad_geometry PRIVATE cicada_kicad_model)
  target_compile_features(cicada_kicad_geometry PUBLIC cxx_std_20)
  if(MSVC)
    target_compile_options(cicada_kicad_geometry PRIVATE /utf-8)
  endif()

  add_executable(kicad-model-slice2-geometry-test tests/slice2/geometry_batch_test.cpp)
  target_link_libraries(kicad-model-slice2-geometry-test PRIVATE cicada_kicad_geometry cicada_kicad_model)
  if(WIN32 AND EXISTS "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib")
    target_link_libraries(kicad-model-slice2-geometry-test PRIVATE
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxbase32u.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_core.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_adv.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxpng.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxtiff.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxjpeg.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxzlib.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxregexu.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxexpat.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_aui.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_propgrid.lib"
      comctl32 rpcrt4 advapi32 comdlg32 gdi32 ole32 oleaut32 shell32 shlwapi
      user32 version winmm winspool ws2_32)
  endif()
  if(MSVC)
    target_compile_options(kicad-model-slice2-geometry-test PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(kicad-model-slice2-geometry-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME kicad-model-slice2-geometry COMMAND kicad-model-slice2-geometry-test)

  # ── 切片 3：CONNECTION_GRAPH::Recalculate 真源化冒烟 ──
  add_executable(kicad-model-slice3-test tests/slice3/connection_smoke_test.cpp)
  target_include_directories(kicad-model-slice3-test PRIVATE src)
  target_link_libraries(kicad-model-slice3-test PRIVATE cicada_kicad_geometry cicada_kicad_model)
  if(WIN32 AND EXISTS "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib")
    target_link_libraries(kicad-model-slice3-test PRIVATE
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxbase32u.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_core.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_adv.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxpng.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxtiff.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxjpeg.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxzlib.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxregexu.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxexpat.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_aui.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_propgrid.lib"
      comctl32 rpcrt4 advapi32 comdlg32 gdi32 ole32 oleaut32 shell32 shlwapi
      user32 version winmm winspool ws2_32)
  endif()
  if(MSVC)
    target_compile_options(kicad-model-slice3-test PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(kicad-model-slice3-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME kicad-model-slice3 COMMAND kicad-model-slice3-test)

  # ── 切片 3 B3a-1：壳层交互引擎（HitTest/拖移/端点拖/框选/undo/删除/放置 原子操作）──
  add_executable(kicad-model-slice3-b3-test tests/slice3/interaction_engine_test.cpp)
  target_include_directories(kicad-model-slice3-b3-test PRIVATE src)
  target_link_libraries(kicad-model-slice3-b3-test PRIVATE cicada_kicad_geometry cicada_kicad_model)
  if(WIN32 AND EXISTS "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib")
    target_link_libraries(kicad-model-slice3-b3-test PRIVATE
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxbase32u.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_core.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_adv.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxpng.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxtiff.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxjpeg.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxzlib.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxregexu.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxexpat.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_aui.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_propgrid.lib"
      comctl32 rpcrt4 advapi32 comdlg32 gdi32 ole32 oleaut32 shell32 shlwapi
      user32 version winmm winspool ws2_32)
  endif()
  if(MSVC)
    target_compile_options(kicad-model-slice3-b3-test PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(kicad-model-slice3-b3-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME kicad-model-slice3-b3 COMMAND kicad-model-slice3-b3-test)

  # K6 minidump symbolizer (raw RVA parse + dbghelp; PDB-aware)
  add_executable(kicad-dump-tool tests/k6/dump_analyze.cpp)

  # K6 reload crash repro: headless load of a .cicada_sch (argv[1]) with SEH stack dump
  add_executable(kicad-model-slice3-reload-test tests/slice3/reload_probe_test.cpp src/kicad_bridge/cicada_document_bridge.cpp src/kicad_bridge/kicad_scene_adapter.cpp)
  target_include_directories(kicad-model-slice3-reload-test PRIVATE src)
  target_link_libraries(kicad-model-slice3-reload-test PRIVATE cicada_kicad_geometry cicada_kicad_model cicada_editor_core)
  if(WIN32 AND EXISTS "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib")
    target_link_libraries(kicad-model-slice3-reload-test PRIVATE
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxbase32u.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_core.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_adv.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxpng.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxtiff.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxjpeg.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxzlib.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxregexu.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxexpat.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_aui.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_propgrid.lib"
      comctl32 rpcrt4 advapi32 comdlg32 gdi32 ole32 oleaut32 shell32 shlwapi
      user32 version winmm winspool ws2_32)
  endif()
  if(MSVC)
    target_compile_options(kicad-model-slice3-reload-test PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(kicad-model-slice3-reload-test PRIVATE -UNDEBUG)
  endif()

  # ── M1a: 引擎服务（headless）。scene_json 先行为单测目标；cicada-engine 服务进程
  #    随后续 http_server/ops 落位时以同一 src/service 集合追加目标（wx 静态库清单同 reload 测试）。
  # ──
  set(CICADA_WX_STATIC_LIBS "")
  if(WIN32 AND EXISTS "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib")
    set(CICADA_WX_STATIC_LIBS
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxbase32u.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_core.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxmsw32u_adv.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxpng.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxtiff.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxjpeg.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxzlib.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxregexu.lib"
      "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/wxexpat.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_aui.lib"
      "${CICADA_WX_ROOT}/lib/vc_x64_lib/wxmsw32u_propgrid.lib"
      comctl32 rpcrt4 advapi32 comdlg32 gdi32 ole32 oleaut32 shell32 shlwapi
      user32 version winmm winspool ws2_32)
  endif()
  add_executable(cicada-engine-scene-test tests/service/scene_json_test.cpp src/service/scene_json.cpp
    src/kicad_bridge/cicada_document_bridge.cpp src/kicad_bridge/kicad_scene_adapter.cpp)
  target_include_directories(cicada-engine-scene-test PRIVATE src)
  target_link_libraries(cicada-engine-scene-test PRIVATE cicada_kicad_geometry cicada_kicad_model cicada_editor_core)
  if(WIN32)
    target_link_libraries(cicada-engine-scene-test PRIVATE ${CICADA_WX_STATIC_LIBS})
  endif()
  if(MSVC)
    target_compile_options(cicada-engine-scene-test PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(cicada-engine-scene-test PRIVATE -UNDEBUG)
  endif()

  # ── M1b0: lib_symbols 归一容错回归（读 fixture → 不对称/缺条目场景 → 往返断言）──
  add_executable(cicada-engine-libtest tests/service/lib_roundtrip_test.cpp src/service/lib_loader.cpp
    src/kicad_bridge/cicada_document_bridge.cpp src/kicad_bridge/kicad_scene_adapter.cpp)
  target_include_directories(cicada-engine-libtest PRIVATE src)
  target_link_libraries(cicada-engine-libtest PRIVATE cicada_kicad_geometry cicada_kicad_model cicada_editor_core)
  if(WIN32)
    target_link_libraries(cicada-engine-libtest PRIVATE ${CICADA_WX_STATIC_LIBS})
  endif()
  if(MSVC)
    target_compile_options(cicada-engine-libtest PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(cicada-engine-libtest PRIVATE -UNDEBUG)
  endif()

  # ── M1a: cicada-engine 服务进程（headless；Beast/Asio header-only，经 model PUBLIC boost 头）──
  # ── M1c：画线预览几何（折角=KiCad computeBreakPoint 45°；吸附=SnapEndpoints/SnapGrid）──
  add_executable(cicada-engine-wiretest tests/service/wire_preview_test.cpp src/service/wire_preview.cpp)
  target_include_directories(cicada-engine-wiretest PRIVATE src)
  target_link_libraries(cicada-engine-wiretest PRIVATE cicada_kicad_geometry cicada_kicad_model cicada_editor_core)
  if(WIN32)
    target_link_libraries(cicada-engine-wiretest PRIVATE ${CICADA_WX_STATIC_LIBS})
  endif()
  if(MSVC)
    target_compile_options(cicada-engine-wiretest PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(cicada-engine-wiretest PRIVATE -UNDEBUG)
  endif()
  add_test(NAME cicada-engine-wire COMMAND cicada-engine-wiretest)

  # ── M1e-1: 形状块 → .kicad_sym 确定性生成（docs/02 附录 A）+ lib_loader 回环 ──
  add_executable(cicada-engine-shapesynth-test tests/service/shape_synth_test.cpp
    src/service/shape_synth.cpp src/service/lib_loader.cpp)
  target_include_directories(cicada-engine-shapesynth-test PRIVATE src)
  target_link_libraries(cicada-engine-shapesynth-test PRIVATE cicada_kicad_geometry cicada_kicad_model cicada_editor_core)
  if(WIN32)
    target_link_libraries(cicada-engine-shapesynth-test PRIVATE ${CICADA_WX_STATIC_LIBS})
  endif()
  if(MSVC)
    target_compile_options(cicada-engine-shapesynth-test PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(cicada-engine-shapesynth-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME cicada-engine-shapesynth COMMAND cicada-engine-shapesynth-test)

  add_executable(cicada-engine src/service/main_service.cpp src/service/http_server.cpp src/service/lib_loader.cpp
    src/service/scene_json.cpp src/service/ops.cpp src/service/wire_preview.cpp src/service/shape_synth.cpp
    src/kicad_bridge/cicada_document_bridge.cpp src/kicad_bridge/kicad_scene_adapter.cpp)
  target_include_directories(cicada-engine PRIVATE src)
  target_link_libraries(cicada-engine PRIVATE cicada_kicad_geometry cicada_kicad_model cicada_editor_core)
  if(WIN32)
    target_link_libraries(cicada-engine PRIVATE ${CICADA_WX_STATIC_LIBS} ws2_32 crypt32)
  endif()
  if(MSVC)
    target_compile_options(cicada-engine PRIVATE /UNDEBUG /utf-8)
  else()
    target_compile_options(cicada-engine PRIVATE -UNDEBUG)
  endif()
endif()

if(WIN32 AND CICADA_EDITOR_BUILD_WIN32_PROBE)
  add_executable(cicada-editor-win32-probe WIN32 tests/win32_probe.cpp)
  target_link_libraries(cicada-editor-win32-probe PRIVATE user32)
endif()

# This is deliberately an opt-in OBJECT target.  The KiCad checkout in this
# repository is a read-only source reference and is not a second subproject of
# cicada-editor.  Building the probe therefore checks only whether the planned
# native SCH model headers can be compiled with the selected KiCad dependency
# roots; it does not link KiCad, alter kicad/, or affect the normal contract
# build.
if(CICADA_EDITOR_ENABLE_KICAD_PROBE)
  if(NOT CICADA_KICAD_SOURCE_DIR OR NOT EXISTS "${CICADA_KICAD_SOURCE_DIR}/eeschema/sch_item.h")
    message(FATAL_ERROR
      "CICADA_EDITOR_ENABLE_KICAD_PROBE requires CICADA_KICAD_SOURCE_DIR "
      "pointing to a KiCad checkout containing eeschema/sch_item.h")
  endif()

  add_library(cicada-editor-kicad-model-probe OBJECT
    tests/kicad_model_probe.cpp)
  set(_cicada_kicad_probe_generated_dir
    "${CMAKE_CURRENT_BINARY_DIR}/cicada-kicad-probe-generated")
  file(MAKE_DIRECTORY "${_cicada_kicad_probe_generated_dir}")
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/kicad_probe_config.h.in"
    "${_cicada_kicad_probe_generated_dir}/config.h" COPYONLY)
  target_include_directories(cicada-editor-kicad-model-probe PRIVATE
    "${_cicada_kicad_probe_generated_dir}"
    "${CICADA_KICAD_SOURCE_DIR}/include"
    "${CICADA_KICAD_SOURCE_DIR}/include/core"
    "${CICADA_KICAD_SOURCE_DIR}/libs/core/include"
    "${CICADA_KICAD_SOURCE_DIR}/common"
    "${CICADA_KICAD_SOURCE_DIR}/eeschema"
    "${CICADA_KICAD_SOURCE_DIR}/libs/kimath/include"
    "${CICADA_KICAD_SOURCE_DIR}/libs/sexpr/include")
  if(CICADA_KICAD_BINARY_DIR)
    target_include_directories(cicada-editor-kicad-model-probe PRIVATE
      "${CICADA_KICAD_BINARY_DIR}"
      "${CICADA_KICAD_BINARY_DIR}/include")
  endif()
  if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/wxWidgets/include")
    target_include_directories(cicada-editor-kicad-model-probe PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/third_party/wxWidgets/include"
      "${CMAKE_CURRENT_BINARY_DIR}/third_party/wxWidgets/include")
  endif()
  target_compile_features(cicada-editor-kicad-model-probe PRIVATE cxx_std_20)
endif()

# ── 切片 1：KiCad 数据模型闭包（CICADA_EDITOR_ENABLE_KICAD_MODEL=ON 时构建）──
if(CICADA_EDITOR_ENABLE_KICAD_MODEL)
  add_subdirectory(src/vendor/kicad)
endif()
