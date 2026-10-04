cmake_minimum_required(VERSION 3.18)

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

# 定义项目名称和开发语言
project(panel CXX)

# duilib 的源码根目录
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

# 项目源码目录
get_filename_component(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/" ABSOLUTE)

# 定义资源（*.rc）名，默认名称是 "项目名称.rc"，文件编码需要改为UTF8编码
# 资源文件需要放在源码所在目录中，只有Windows平台有效，其他平台忽略该定义
set(DUILIB_WINRES_FILE_NAME "${PROJECT_NAME}.rc")

# Windows 平台：定义 Skia 库的子目录（仅当未定义时才定义，与 build/msvc_build.bat 保持一致）
if(WIN32 AND NOT DEFINED DUILIB_SKIA_LIB_SUBPATH)
    # 取 CMAKE_GENERATOR_PLATFORM（cmake -A 参数）作为架构
    if(CMAKE_GENERATOR_PLATFORM)
        set(CPU_ARCH "${CMAKE_GENERATOR_PLATFORM}")
    else()
        set(CPU_ARCH "x64")
    endif()
    string(TOLOWER "${CPU_ARCH}" CPU_ARCH)
    if(CPU_ARCH STREQUAL "win32")
        set(CPU_ARCH "x86")
    endif()
    set(DUILIB_SKIA_LIB_SUBPATH "llvm.${CPU_ARCH}.release")
    message(STATUS "DUILIB_SKIA_LIB_SUBPATH: ${DUILIB_SKIA_LIB_SUBPATH}")
endif()

# 包含公共实现代码
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_common.cmake")
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_bin.cmake")
