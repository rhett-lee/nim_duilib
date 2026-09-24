cmake_minimum_required(VERSION 3.18)

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

# 定义项目名称和开发语言（<project_name> 替换为实际工程名，需与 examples/ 下的目录名一致）
project(<project_name> CXX)

# duilib 的源码根目录
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

# 项目源码目录
get_filename_component(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/" ABSOLUTE)

# Skia 库子目录名（仅 Windows 平台）
# 规则与 build/msvc_build.bat 保持一致：llvm.<CPU_ARCH>.release，
# 其中 CPU_ARCH 取 VS 目标架构（cmake -A 参数），缺省 x64，-A Win32 时为 x86。
# 仅当外部未指定时才设置：-DDUILIB_SKIA_LIB_SUBPATH=... 传入时以外部值为准。
# 必须放在 include(duilib_common.cmake) 之前：该脚本用 option() 声明此变量，
# 一旦 include，变量必然已定义（值为 OFF），届时再判断"是否未定义"永远为假。
if(WIN32 AND NOT DEFINED DUILIB_SKIA_LIB_SUBPATH)
    set(CPU_ARCH "x64")
    if(CMAKE_GENERATOR_PLATFORM)
        if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(CPU_ARCH "x86")
        else()
            set(CPU_ARCH "${CMAKE_GENERATOR_PLATFORM}")
        endif()
    endif()
    set(DUILIB_SKIA_LIB_SUBPATH "llvm.${CPU_ARCH}.release")
endif()

# 包含公共实现代码
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_common.cmake")
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_bin.cmake")
