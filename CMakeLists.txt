# nim_duilib 根工程：统一构建入口
#
# 所有构建逻辑下沉到根工程；命令行入口 build/build.py 只负责环境识别与编排。
# 方案见 docs/misc/Root-CMake-Build-Plan.md。

cmake_minimum_required(VERSION 3.24)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 先声明入口参数，保证 project() 之前的阶段 A 预检可以读取
set(DUILIB_LANG "auto" CACHE STRING "Interaction language: auto / zh_CN / en")
set(DUILIB_DEPS_MODE "off" CACHE STRING "Dependency mode: off / prepare / auto")
set_property(CACHE DUILIB_DEPS_MODE PROPERTY STRINGS off prepare auto)
option(DUILIB_PREFLIGHT "Run the two-stage environment preflight" ON)
option(DUILIB_PREFLIGHT_ONLY "Run preflight and exit without configuring targets" OFF)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/duilib_i18n.cmake")

# ---------------------------------------------------------------------------
# 阶段 A：无编译器环境预检（独立 cmake -P，失败时不生成构建树）
# ---------------------------------------------------------------------------
if(DUILIB_PREFLIGHT)
    if(NOT DUILIB_DEPS_ROOT)
        set(DUILIB_DEPS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
    endif()
    if(NOT DUILIB_SKIA_ROOT)
        set(DUILIB_SKIA_ROOT "${DUILIB_DEPS_ROOT}/skia")
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}"
                "-DDUILIB_LANG=${DUILIB_I18N_LANG}"
                "-DDUILIB_DEPS_MODE=${DUILIB_DEPS_MODE}"
                "-DDUILIB_GENERATOR=${CMAKE_GENERATOR}"
                "-DDUILIB_DEPS_ROOT=${DUILIB_DEPS_ROOT}"
                "-DDUILIB_SKIA_ROOT=${DUILIB_SKIA_ROOT}"
                "-DDUILIB_GN_EXECUTABLE=${DUILIB_GN_EXECUTABLE}"
                -P "${CMAKE_CURRENT_LIST_DIR}/cmake/duilib_preflight_tools.cmake"
        RESULT_VARIABLE _duilib_preflight_rc)
    if(NOT _duilib_preflight_rc EQUAL 0)
        message(FATAL_ERROR "[ERROR] environment preflight failed; see the messages above")
    endif()
endif()

project(nim_duilib LANGUAGES C CXX)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/duilib_root_options.cmake")
if(DUILIB_PREFLIGHT)
    include("${CMAKE_CURRENT_LIST_DIR}/cmake/duilib_preflight.cmake")
endif()

if(DUILIB_PREFLIGHT_ONLY)
    duilib_msg("preflight.preflight_only")
    return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/cmake/duilib_deps.cmake")

# prepare 模式只生成 duilib_deps 目标，供 `cmake --build --target duilib_deps` 使用
if(DUILIB_DEPS_MODE STREQUAL "prepare")
    duilib_add_dependency_targets()
    return()
endif()

duilib_configure_dependencies()
if(DUILIB_DEPS_MODE STREQUAL "auto")
    duilib_add_dependency_targets()
endif()

# MSVC：源文件为 UTF-8，并开启多进程编译
if(MSVC)
    add_compile_options("$<$<COMPILE_LANGUAGE:C>:/utf-8>" "$<$<COMPILE_LANGUAGE:CXX>:/utf-8>")
endif()

# ---------------------------------------------------------------------------
# 第三方库
# ---------------------------------------------------------------------------
if(DUILIB_BUILD_THIRD_PARTY)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duilib/third_party/zlib/CMakeLists.txt")
        add_subdirectory(duilib/third_party/zlib)
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duilib/third_party/libpng/CMakeLists.txt")
        add_subdirectory(duilib/third_party/libpng)
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duilib/third_party/cximage/CMakeLists.txt")
        add_subdirectory(duilib/third_party/cximage)
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duilib/third_party/libwebp/CMakeLists.txt")
        add_subdirectory(duilib/third_party/libwebp)
    endif()
endif()

# CEF wrapper：仓库只跟踪头文件/导入库，wrapper 需要随工程编译
if(DUILIB_ENABLE_CEF)
    if(DUILIB_OS_WINDOWS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duilib/third_party/libcef/libcef_win/CMakeLists.txt")
        add_subdirectory(duilib/third_party/libcef/libcef_win)
    elseif(DUILIB_OS_LINUX AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duilib/third_party/libcef/libcef_linux/CMakeLists.txt")
        add_subdirectory(duilib/third_party/libcef/libcef_linux)
    endif()
endif()

# ---------------------------------------------------------------------------
# duilib 与示例
# ---------------------------------------------------------------------------
add_subdirectory(duilib)

if(DUILIB_BUILD_EXAMPLES)
    foreach(_duilib_example IN LISTS DUILIB_EXAMPLES)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/${_duilib_example}/CMakeLists.txt")
            add_subdirectory("examples/${_duilib_example}")
        else()
            message(STATUS "[WARN] example '${_duilib_example}' not found; skipped")
        endif()
    endforeach()
endif()

# auto 模式：依赖准备先于所有编译
duilib_link_dependency_targets()

# CEF wrapper 只需在 CEF 示例（cef/CefBrowser）链接之前完成
if(TARGET libcef_dll_wrapper)
    foreach(_duilib_cef_example IN ITEMS cef CefBrowser)
        if(TARGET ${_duilib_cef_example})
            add_dependencies(${_duilib_cef_example} libcef_dll_wrapper)
        endif()
    endforeach()
endif()
