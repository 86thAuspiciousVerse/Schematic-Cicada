# cicada_kicad_model: KiCad eeschema 数据模型闭包（切片 1）
# 来源：KiCad 官方 10.0.6 tag（caf7377e），白名单裁剪拷贝，GPLv3-or-later。
# 本库只承载"数据模型闭合"：SCH_ITEM/LINE/JUNCTION/NO_CONNECT/LABEL/SCREEN
# + 传递依赖（SHEET_PATH/RTREE/SHEET/...）+ 最小 kicommon + kimath/libs-core/sexpr
# 底座 + thirdparty bundled。明确排除 GAL/PAINTER/VIEW/TOOL/IO/连接图。
# 拷贝纪律见 vendor/kicad/README.md 拷贝登记；源文件先按 git archive 精确清单
# 拷贝，本阶段用 GLOB 编译已拷入的全部 .cpp（编译驱动裁剪，稳定后转显式清单）。

# 源清单：全树 GLOB——vendor 树本身就是"git archive 按需拷入"的精确集合
#（拷贝纪律见 README.md 拷贝登记）；新增按需源自动进入编译。
file(GLOB_RECURSE KICAD_LIB_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
# fmt 源码树含 C++20 module 接口文件（fmt.cc `export module fmt;`），本库以经典
# 头/源方式编译，仅取 format.cc + os.cc 两个经典源
list(REMOVE_ITEM KICAD_LIB_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/fmt/src/fmt.cc")

add_library(cicada_kicad_model STATIC ${KICAD_LIB_SRCS})
target_compile_features(cicada_kicad_model PUBLIC cxx_std_20)
# KiCad 上游构建定义 USINGZ（Clipper2 的 3D/Z 支持，shape_* 依赖其 z 成员/回调）
target_compile_definitions(cicada_kicad_model PUBLIC _USE_MATH_DEFINES=1 USINGZ=1 WIN32_LEAN_AND_MEAN=1 KICOMMON_DLL=1 GAL_DLL=1)
if(MSVC)
  # fmt 要求 /utf-8（Unicode 支持）；KiCad 官方 MSVC 构建亦如此
  target_compile_options(cicada_kicad_model PRIVATE /utf-8)
endif()

set(_vk "${CMAKE_CURRENT_SOURCE_DIR}")

# 生成 config.h：KiCad 源码要求 <config.h>。手写最小生成头
#（由 scripts/generate-config.mjs 从 10.0.6 cmake/config.h.cmake 生成，
#  新增宏按编译错误逐个开启；严禁整包拷贝 KiCad 配置生成物）。
set(_cicada_kicad_config_dir "${CMAKE_CURRENT_BINARY_DIR}/cicada-kicad-config")
file(MAKE_DIRECTORY "${_cicada_kicad_config_dir}")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
    "${_cicada_kicad_config_dir}/config.h" COPYONLY)
else()
  message(FATAL_ERROR
    "vendor/kicad/config.h missing: hand-written minimal config header is "
    "required before the model closure can compile")
endif()

target_include_directories(cicada_kicad_model PUBLIC
  "${_cicada_kicad_config_dir}"
  "${_vk}/include"
  "${_vk}/common"
  "${_vk}/common/dialogs"
  "${_vk}/eeschema"
  "${_vk}/libs/kimath/include"
  "${_vk}/libs/core/include"
  "${_vk}/libs/sexpr/include"
  "${_vk}/libs/kiplatform/include"
  "${_vk}/thirdparty/thread-pool"
  "${_vk}/thirdparty/magic_enum/magic_enum"
  "${_vk}/thirdparty/fast_float/include"
  "${_vk}/thirdparty/dynamic_bitset"
  "${_vk}/thirdparty/rtree"
  "${_vk}/thirdparty/nlohmann_json"
  "${_vk}/thirdparty/fmt/include"
  "${_vk}/thirdparty/clipper2/Clipper2Lib/include"
  "${_vk}/thirdparty/pegtl"
  "${_vk}/thirdparty/expected/include"
  "${_vk}/thirdparty/picosha2"
  "${_vk}/thirdparty/glm/include"
  "${CICADA_BOOST_ROOT}"
)

if(WIN32 AND EXISTS "${CICADA_WX_ROOT}/include")
  # wx 顺序：项目内 build（setup.h）优先，再是源码头
  target_include_directories(cicada_kicad_model PUBLIC
    "${CICADA_WX_ROOT}/build-cicada-msvc/lib/vc_x64_lib/mswu"
    "${CICADA_WX_ROOT}/lib/vc_x64_lib/mswu"
    "${CICADA_WX_ROOT}/include")
endif()

if(WIN32)
  target_link_libraries(cicada_kicad_model PRIVATE
    ${CICADA_WX_LIBS} ws2_32 winhttp crypt32 wintrust)
endif()
