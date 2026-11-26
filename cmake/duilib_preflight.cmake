# 阶段 B：编译器已探测后的预检（include 模式，在 project() 之后）
#
# 只做确定性的判断，不下载、不改文件。全部错误文案来自 cmake/i18n/messages.json。

include("${CMAKE_CURRENT_LIST_DIR}/duilib_i18n.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/duilib_skia_toolchain.cmake")

# MSVC 版本：VS 2022 / MSVC 19.30+
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.30")
        duilib_tf(_duilib_pf_what "preflight.msvc_too_old"
                  "version=${CMAKE_CXX_COMPILER_VERSION}")
        duilib_t("preflight.msvc_too_old_hint" _duilib_pf_hint)
        message(FATAL_ERROR "${_duilib_pf_what}\n  Hint: ${_duilib_pf_hint}")
    endif()
endif()

# GNU / Clang：仅要求可用的 C++20 编译器
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0")
        duilib_tf(_duilib_pf_what "preflight.gcc_too_old"
                  "version=${CMAKE_CXX_COMPILER_VERSION}")
        duilib_t("preflight.gcc_too_old_hint" _duilib_pf_hint)
        message(FATAL_ERROR "${_duilib_pf_what}\n  Hint: ${_duilib_pf_hint}")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
        duilib_tf(_duilib_pf_what "preflight.clang_too_old"
                  "version=${CMAKE_CXX_COMPILER_VERSION}")
        duilib_t("preflight.clang_too_old_hint" _duilib_pf_hint)
        message(FATAL_ERROR "${_duilib_pf_what}\n  Hint: ${_duilib_pf_hint}")
    endif()
endif()

# Skia 工具链：ml3/ml4 目标（Linux/x64）固定使用 -march=x86-64-v3，
# 需要 clang 12+ 或 gcc 11+；两者都过旧时在配置阶段给出可读错误。
duilib_skia_toolchain_check()

# CEF + MinGW：ABI 不兼容
if(DUILIB_ENABLE_CEF AND MINGW)
    duilib_error("preflight.cef_mingw" "preflight.cef_mingw_hint")
endif()

# 目标架构与 Skia CPU 架构必须一致（子项目据此选择 Skia 子目录）
if(NOT DUILIB_TARGET_ARCH_RESOLVED STREQUAL DUILIB_SKIA_CPU_ARCH)
    duilib_tf(_duilib_pf_what "preflight.arch_mismatch"
              "target=${DUILIB_TARGET_ARCH_RESOLVED}"
              "skia=${DUILIB_SKIA_CPU_ARCH}")
    duilib_t("preflight.arch_mismatch_hint" _duilib_pf_hint)
    message(FATAL_ERROR "${_duilib_pf_what}\n  Hint: ${_duilib_pf_hint}")
endif()
