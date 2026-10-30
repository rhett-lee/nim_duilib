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
# 依赖归档位置（Skia 补丁 zip）：阶段 A 预检、Ninja 自举与依赖准备共用
# ---------------------------------------------------------------------------
if(NOT DUILIB_DEPS_ROOT)
    set(DUILIB_DEPS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
endif()
if(NOT DUILIB_SKIA_ROOT)
    set(DUILIB_SKIA_ROOT "${DUILIB_DEPS_ROOT}/skia")
endif()
# Skia 补丁归档随仓库分发；归档自带 bin/gn.exe、bin/ninja.exe。全新环境尚未
# 解压时，预检不应因缺少 gn/ninja 而中断，需把归档位置传给阶段 A 预检判断。
set(_duilib_patch_dir "")
set(_duilib_patch_zip "")
set(_duilib_versions_file "${CMAKE_CURRENT_SOURCE_DIR}/cmake/third_party_versions.json")
if(EXISTS "${_duilib_versions_file}")
    file(READ "${_duilib_versions_file}" _duilib_versions_json)
    string(JSON _duilib_patch_dir ERROR_VARIABLE _duilib_json_err
           GET "${_duilib_versions_json}" skia patchDir)
    if(_duilib_json_err OR _duilib_patch_dir STREQUAL "")
        set(_duilib_patch_dir "build/skia_patch")
    endif()
    get_filename_component(_duilib_patch_dir
        "${CMAKE_CURRENT_SOURCE_DIR}/${_duilib_patch_dir}" ABSOLUTE)
    string(JSON _duilib_skia_key ERROR_VARIABLE _duilib_json_err
           GET "${_duilib_versions_json}" skia default)
    if(NOT "${DUILIB_SKIA_REF}" STREQUAL "")
        set(_duilib_skia_key "${DUILIB_SKIA_REF}")
    endif()
    if(NOT _duilib_json_err AND NOT _duilib_skia_key STREQUAL "")
        string(JSON _duilib_patch_zip ERROR_VARIABLE _duilib_json_err
               GET "${_duilib_versions_json}" skia refs "${_duilib_skia_key}" patchZip)
        if(_duilib_json_err)
            set(_duilib_patch_zip "")
        endif()
    endif()
endif()
if(NOT "${DUILIB_SKIA_PATCH_DIR}" STREQUAL "")
    set(_duilib_patch_dir "${DUILIB_SKIA_PATCH_DIR}")
endif()
if(NOT "${DUILIB_SKIA_PATCH_ZIP}" STREQUAL "")
    set(_duilib_patch_zip "${DUILIB_SKIA_PATCH_ZIP}")
endif()

# Ninja 生成器自举：VS 未安装 ninja（可选组件）时使用归档自带的 bin/ninja.exe。
# 必须在 project() 之前执行，CMAKE_MAKE_PROGRAM 才会被 Ninja 生成器采用。
include("${CMAKE_CURRENT_LIST_DIR}/cmake/duilib_bootstrap.cmake")
duilib_bootstrap_ninja("${_duilib_patch_dir}" "${_duilib_patch_zip}")

# ---------------------------------------------------------------------------
# 阶段 A：无编译器环境预检（独立 cmake -P，失败时不生成构建树）
# ---------------------------------------------------------------------------
if(DUILIB_PREFLIGHT)
    execute_process(
        COMMAND "${CMAKE_COMMAND}"
                "-DDUILIB_LANG=${DUILIB_I18N_LANG}"
                "-DDUILIB_DEPS_MODE=${DUILIB_DEPS_MODE}"
                "-DDUILIB_GENERATOR=${CMAKE_GENERATOR}"
                "-DDUILIB_DEPS_ROOT=${DUILIB_DEPS_ROOT}"
                "-DDUILIB_SKIA_ROOT=${DUILIB_SKIA_ROOT}"
                "-DDUILIB_GN_EXECUTABLE=${DUILIB_GN_EXECUTABLE}"
                "-DDUILIB_NINJA_EXECUTABLE=${DUILIB_NINJA_EXECUTABLE}"
                "-DDUILIB_SKIA_PATCH_DIR=${_duilib_patch_dir}"
                "-DDUILIB_SKIA_PATCH_ZIP=${_duilib_patch_zip}"
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
        # libpng 以源码内嵌方式构建，不安装/导出（duilib-png 链接本工程的 duilib-zlib
        # 目标，开启导出时会因目标不在导出集中而生成失败）
        set(SKIP_INSTALL_ALL ON)
        add_subdirectory(duilib/third_party/libpng)
        unset(SKIP_INSTALL_ALL)
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duilib/third_party/cximage/CMakeLists.txt")
        add_subdirectory(duilib/third_party/cximage)
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duilib/third_party/libwebp/CMakeLists.txt")
        add_subdirectory(duilib/third_party/libwebp)
    endif()
endif()

# CEF wrapper：仓库只跟踪头文件/导入库，wrapper 需要随工程编译
# 目录必须跟随 DUILIB_CEF_VERSION（109 -> libcef_win_109），否则会用最新版
# 的 wrapper 源码去编译 109 的头文件，且产物名与链接方期望的不一致。
if(DUILIB_ENABLE_CEF)
    if(DUILIB_OS_WINDOWS)
        # 本项目不使用CEF沙箱（不链接 cef_sandbox.lib，打包也不随包），直接关闭：
        # - CEF 109：USE_SANDBOX=ON 时会为 Debug 的 wrapper 追加
        #   _HAS_ITERATOR_DEBUGGING=0（等价 _ITERATOR_DEBUG_LEVEL=0），与 Debug
        #   主工程的默认值 2 冲突，链接时报 LNK2038。
        # - CEF 156：USE_SANDBOX=ON 时会为 wrapper 追加
        #   CEF_API_VERSION=CEF_API_VERSION_LAST，而主工程未定义该宏（使用实验性
        #   API），wrapper 与主工程必须使用同一 API 版本，否则存在 ABI 不一致风险。
        set(USE_SANDBOX OFF CACHE BOOL "Enable or disable use of the sandbox." FORCE)
    endif()
    set(_duilib_cef_subdir "")
    if(DUILIB_OS_WINDOWS)
        if(DUILIB_CEF_VERSION STREQUAL "109")
            set(_duilib_cef_subdir "libcef_win_109")
        else()
            set(_duilib_cef_subdir "libcef_win")
        endif()
    elseif(DUILIB_OS_LINUX)
        set(_duilib_cef_subdir "libcef_linux")
    elseif(DUILIB_OS_MACOS)
        # macOS 的 wrapper 同样需要随工程编译（产物输出到 lib/<Config>）
        set(_duilib_cef_subdir "libcef_macos")
    endif()
    if(_duilib_cef_subdir AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duilib/third_party/libcef/${_duilib_cef_subdir}/CMakeLists.txt")
        add_subdirectory("duilib/third_party/libcef/${_duilib_cef_subdir}")

        # CEF 156 的 CMakeLists 用 `set(CMAKE_CONFIGURATION_TYPES ... CACHE ... FORCE)`
        # 写缓存。单配置生成器（Ninja/Make）上这会污染根缓存，使后续逻辑把本工程
        # 误判为多配置（Skia 路径退化成 $<IF:$<CONFIG:Debug>,...> 生成器表达式）。
        # 用生成器属性判断（不依赖缓存值），配置完 libcef 后复原。
        get_property(_duilib_generator_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
        if(NOT _duilib_generator_multi_config)
            unset(CMAKE_CONFIGURATION_TYPES CACHE)
            unset(CMAKE_CONFIGURATION_TYPES)
        endif()

        # CEF 109 的 wrapper 按上游固定使用 C++17，而本工程是 C++20。若不做目标级
        # 降级，编译命令会同时带 /std:c++20（MSVC）或 -std=c++20（clang）与 CEF
        # 自带的 /std:c++17，触发 D9025（或 clang 的选项覆盖警告）。wrapper 是独立
        # 静态库，单独使用 C++17 不影响 duilib 自身的 C++20。
        # 最新版（156 起）的 wrapper 已改用 C++20，无需降级。
        if(TARGET libcef_dll_wrapper_109)
            set_target_properties(libcef_dll_wrapper_109 PROPERTIES
                CXX_STANDARD 17
                CXX_STANDARD_REQUIRED ON)
        endif()
    endif()
endif()

# ---------------------------------------------------------------------------
# duilib 与示例
# ---------------------------------------------------------------------------
add_subdirectory(duilib)

if(DUILIB_BUILD_EXAMPLES)
    foreach(_duilib_example IN LISTS DUILIB_EXAMPLES)
        # CEF 示例依赖 duilib 中的 CefManager/CefControl 实现：关闭 DUILIB_ENABLE_CEF
        # 时构建它们只会在链接阶段报“无法解析的外部符号”。这里直接跳过，避免
        # 缓存里残留的旧示例列表（例如上次 CEF 开启时推导出的 cef/CefBrowser）
        # 造成构建失败。
        if(NOT DUILIB_ENABLE_CEF AND (_duilib_example STREQUAL "cef"
                                      OR _duilib_example STREQUAL "CefBrowser"))
            message(STATUS "[INFO] example '${_duilib_example}' skipped: DUILIB_ENABLE_CEF=OFF")
            continue()
        endif()
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
