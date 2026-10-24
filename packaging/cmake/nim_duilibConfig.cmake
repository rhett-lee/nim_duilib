# nim_duilib SDK 的 CMake 包配置（发布包内使用）
#
# 用法：
#   find_package(nim_duilib CONFIG REQUIRED)
#   target_link_libraries(my_app PRIVATE nim_duilib::duilib)
#
# 可选开关（在 find_package 之前设置）：
#   NIM_DUILIB_SET_MSVC_RUNTIME=OFF        不按 SDK 元数据设置 MSVC 运行库
#   NIM_DUILIB_ALLOW_ARCH_MISMATCH=ON      架构不匹配时只警告
#   NIM_DUILIB_ALLOW_CONFIG_MISMATCH=ON    配置不匹配时只警告
#
# 运行期复制（可选）：
#   include(${nim_duilib_DIR}/nim_duilibHelpers.cmake)   # 本文件已自动 include
#   nim_duilib_copy_runtime(my_app [RESOURCES] [DESTINATION <dir>])

cmake_minimum_required(VERSION 3.18)

if(TARGET nim_duilib::duilib)
    set(nim_duilib_FOUND TRUE)
    return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/nim_duilibPackageInfo.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/nim_duilibHelpers.cmake")

# 包根目录：<root>/lib/cmake/nim_duilib/ -> <root>
get_filename_component(nim_duilib_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(nim_duilib_INCLUDE_DIR "${nim_duilib_ROOT}/include")
set(nim_duilib_LIB_DIR "${nim_duilib_ROOT}/lib")
set(nim_duilib_BIN_DIR "${nim_duilib_ROOT}/bin")
set(nim_duilib_VERSION "${NIM_DUILIB_PACKAGE_VERSION}")
set(NIM_DUILIB_VERSION "${NIM_DUILIB_PACKAGE_VERSION}")
set(NIM_DUILIB_MANIFEST "${nim_duilib_ROOT}/MANIFEST.txt")

if(NOT EXISTS "${nim_duilib_INCLUDE_DIR}/duilib/duilib.h")
    message(FATAL_ERROR
        "nim_duilib SDK 不完整：找不到 ${nim_duilib_INCLUDE_DIR}/duilib/duilib.h。\n"
        "请确认 CMAKE_PREFIX_PATH / nim_duilib_DIR 指向发布包根目录。")
endif()

# --------------------------------------------------------------------------
# 平台 / 工具链 / 架构 / 配置校验
# --------------------------------------------------------------------------

if(NIM_DUILIB_PACKAGE_SYSTEM STREQUAL "win")
    set(_nim_expected_system "Windows")
elseif(NIM_DUILIB_PACKAGE_SYSTEM STREQUAL "linux")
    set(_nim_expected_system "Linux")
elseif(NIM_DUILIB_PACKAGE_SYSTEM STREQUAL "macos")
    set(_nim_expected_system "Darwin")
elseif(NIM_DUILIB_PACKAGE_SYSTEM STREQUAL "freebsd")
    set(_nim_expected_system "FreeBSD")
else()
    set(_nim_expected_system "${NIM_DUILIB_PACKAGE_SYSTEM}")
endif()
if(NOT CMAKE_SYSTEM_NAME STREQUAL _nim_expected_system)
    message(FATAL_ERROR
        "nim_duilib SDK 平台为 ${NIM_DUILIB_PACKAGE_SYSTEM}，当前工程目标平台为 "
        "${CMAKE_SYSTEM_NAME}，不能混用。")
endif()

if(NIM_DUILIB_PACKAGE_TOOLCHAIN STREQUAL "msvc")
    if(NOT MSVC)
        message(FATAL_ERROR
            "该 nim_duilib SDK 是 MSVC 产物（.lib），当前编译器为 "
            "CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}。请使用 MSVC 构建，"
            "或换用 MinGW 发布包。")
    endif()
elseif(NIM_DUILIB_PACKAGE_TOOLCHAIN STREQUAL "mingw")
    if(NOT MINGW)
        message(FATAL_ERROR
            "该 nim_duilib SDK 是 MinGW 产物（.a/.dll.a），当前编译器不是 MinGW。"
            "请使用 MinGW 构建，或换用 MSVC 发布包。")
    endif()
endif()

if(NIM_DUILIB_PACKAGE_SYSTEM STREQUAL "win")
    if(CMAKE_GENERATOR_PLATFORM)
        string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" _nim_arch)
        if(_nim_arch STREQUAL "win32")
            set(_nim_arch "x86")
        endif()
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(_nim_arch "x86")
    else()
        set(_nim_arch "x64")
    endif()
else()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _nim_processor)
    if(_nim_processor MATCHES "aarch64|arm64")
        set(_nim_arch "arm64")
    elseif(_nim_processor MATCHES "i[3-6]86")
        set(_nim_arch "x86")
    else()
        set(_nim_arch "x64")
    endif()
endif()

if(NOT _nim_arch STREQUAL NIM_DUILIB_PACKAGE_ARCH)
    if(NIM_DUILIB_ALLOW_ARCH_MISMATCH)
        message(WARNING
            "nim_duilib SDK 架构为 ${NIM_DUILIB_PACKAGE_ARCH}，当前工程为 ${_nim_arch}；"
            "已按 NIM_DUILIB_ALLOW_ARCH_MISMATCH 放行，链接很可能失败。")
    else()
        message(FATAL_ERROR
            "nim_duilib SDK 架构为 ${NIM_DUILIB_PACKAGE_ARCH}，当前工程为 ${_nim_arch}。"
            "请换用对应架构的发布包，或设置 -DNIM_DUILIB_ALLOW_ARCH_MISMATCH=ON 强行继续。")
    endif()
endif()

if(CMAKE_CONFIGURATION_TYPES)
    message(WARNING
        "当前是多配置生成器，而 nim_duilib 发布包是单配置（${NIM_DUILIB_PACKAGE_CONFIG}）。"
        "构建非 ${NIM_DUILIB_PACKAGE_CONFIG} 配置时会链接到不匹配的运行库；"
        "请为每个配置分别使用对应的发布包。")
elseif(CMAKE_BUILD_TYPE)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" _nim_build_type)
    string(TOLOWER "${NIM_DUILIB_PACKAGE_CONFIG}" _nim_package_config)
    if(NOT _nim_build_type STREQUAL _nim_package_config)
        if(NIM_DUILIB_ALLOW_CONFIG_MISMATCH)
            message(WARNING
                "nim_duilib SDK 配置为 ${NIM_DUILIB_PACKAGE_CONFIG}，工程为 "
                "${CMAKE_BUILD_TYPE}；已按 NIM_DUILIB_ALLOW_CONFIG_MISMATCH 放行。")
        else()
            message(FATAL_ERROR
                "nim_duilib SDK 配置为 ${NIM_DUILIB_PACKAGE_CONFIG}，工程为 "
                "${CMAKE_BUILD_TYPE}。发布包是单配置的，请换用对应配置的包，"
                "或设置 -DNIM_DUILIB_ALLOW_CONFIG_MISMATCH=ON 强行继续。")
        endif()
    endif()
else()
    message(STATUS
        "未设置 CMAKE_BUILD_TYPE；nim_duilib 包为 ${NIM_DUILIB_PACKAGE_CONFIG}，"
        "请自行确认配置一致。")
endif()

# MSVC 运行库：默认对齐 SDK，避免 /MT 与 /MD 混用导致 LNK2038。
if(MSVC AND NIM_DUILIB_PACKAGE_MSVC_RUNTIME)
    if(NOT DEFINED NIM_DUILIB_SET_MSVC_RUNTIME)
        set(NIM_DUILIB_SET_MSVC_RUNTIME ON)
    endif()
    if(NIM_DUILIB_SET_MSVC_RUNTIME)
        if(NIM_DUILIB_PACKAGE_MSVC_RUNTIME STREQUAL "MD")
            if(NIM_DUILIB_PACKAGE_CONFIG STREQUAL "Debug")
                set(_nim_msvc_runtime "MultiThreadedDebugDLL")
            else()
                set(_nim_msvc_runtime "MultiThreadedDLL")
            endif()
        else()
            if(NIM_DUILIB_PACKAGE_CONFIG STREQUAL "Debug")
                set(_nim_msvc_runtime "MultiThreadedDebug")
            else()
                set(_nim_msvc_runtime "MultiThreaded")
            endif()
        endif()

        if(CMAKE_MSVC_RUNTIME_LIBRARY)
            string(FIND "${CMAKE_MSVC_RUNTIME_LIBRARY}" "DLL" _nim_existing_dll_pos)
            if(NOT _nim_existing_dll_pos EQUAL -1)
                set(_nim_existing_md TRUE)
            else()
                set(_nim_existing_md FALSE)
            endif()
            if(NIM_DUILIB_PACKAGE_MSVC_RUNTIME STREQUAL "MD")
                set(_nim_package_md TRUE)
            else()
                set(_nim_package_md FALSE)
            endif()
            if(NOT _nim_existing_md STREQUAL _nim_package_md)
                message(FATAL_ERROR
                    "CMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY} 与 "
                    "nim_duilib SDK 的运行库（${NIM_DUILIB_PACKAGE_MSVC_RUNTIME}）不一致，"
                    "会触发 LNK2038。请移除该设置，或设置 "
                    "-DNIM_DUILIB_SET_MSVC_RUNTIME=OFF 自行保证一致。")
            endif()
            message(STATUS
                "nim_duilib: 沿用已设置的 CMAKE_MSVC_RUNTIME_LIBRARY="
                "${CMAKE_MSVC_RUNTIME_LIBRARY}")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "${_nim_msvc_runtime}")
            message(STATUS
                "nim_duilib: 设置 CMAKE_MSVC_RUNTIME_LIBRARY=${_nim_msvc_runtime}"
                "（与 SDK 的 ${NIM_DUILIB_PACKAGE_MSVC_RUNTIME} 对齐）")
        endif()
    endif()
endif()

# --------------------------------------------------------------------------
# include / 宏定义
# --------------------------------------------------------------------------

set(_nim_includes "${nim_duilib_INCLUDE_DIR}")
if(NIM_DUILIB_PACKAGE_HAS_CEF AND NIM_DUILIB_PACKAGE_CEF_SRC)
    set(_nim_cef_include
        "${nim_duilib_INCLUDE_DIR}/duilib/third_party/libcef/${NIM_DUILIB_PACKAGE_CEF_SRC}")
    if(EXISTS "${_nim_cef_include}/include/cef_version.h")
        list(APPEND _nim_includes "${_nim_cef_include}")
    else()
        message(WARNING
            "SDK 声明包含 CEF，但找不到 ${_nim_cef_include}/include/cef_version.h。")
    endif()
endif()

set(_nim_defines
    "DUILIB_SDL=${NIM_DUILIB_PACKAGE_HAS_SDL}"
    "DUILIB_CEF=${NIM_DUILIB_PACKAGE_HAS_CEF}"
    "DUILIB_WEBVIEW2=${NIM_DUILIB_PACKAGE_HAS_WEBVIEW2}"
    "DUILIB_JPEG_TURBO=${NIM_DUILIB_PACKAGE_HAS_JPEG}"
    "DUILIB_LIB_PAG=0")
if(NIM_DUILIB_PACKAGE_IS_DLL)
    list(APPEND _nim_defines "DUILIB_DLL")
endif()
if(WIN32)
    list(APPEND _nim_defines "UNICODE" "_UNICODE")
endif()

# --------------------------------------------------------------------------
# 链接库（全部使用绝对路径，规避 -l 搜索与顺序差异）
# --------------------------------------------------------------------------

function(_nim_duilib_lib_path OUT_VAR BASE_NAME)
    set(_candidates "")
    if(NIM_DUILIB_PACKAGE_TOOLCHAIN STREQUAL "msvc")
        list(APPEND _candidates "${nim_duilib_LIB_DIR}/${BASE_NAME}.lib")
    else()
        if(NIM_DUILIB_PACKAGE_IS_DLL)
            list(APPEND _candidates "${nim_duilib_LIB_DIR}/lib${BASE_NAME}.dll.a")
        endif()
        list(APPEND _candidates "${nim_duilib_LIB_DIR}/lib${BASE_NAME}.a")
    endif()
    foreach(_candidate IN LISTS _candidates)
        if(EXISTS "${_candidate}")
            set(${OUT_VAR} "${_candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(${OUT_VAR} "" PARENT_SCOPE)
endfunction()

macro(_nim_duilib_add_lib BASE_NAME)
    _nim_duilib_lib_path(_nim_lib_tmp "${BASE_NAME}")
    if(_nim_lib_tmp)
        list(APPEND _nim_link "${_nim_lib_tmp}")
    endif()
endmacro()

macro(_nim_duilib_require_lib BASE_NAME)
    _nim_duilib_lib_path(_nim_lib_tmp "${BASE_NAME}")
    if(NOT _nim_lib_tmp)
        message(FATAL_ERROR
            "nim_duilib SDK 缺少库 ${BASE_NAME}（lib 目录：${nim_duilib_LIB_DIR}）。")
    endif()
    list(APPEND _nim_link "${_nim_lib_tmp}")
endmacro()

set(_nim_link "")

if(NIM_DUILIB_PACKAGE_IS_DLL)
    # DLL 模式：只链导入库，其余依赖已在 duilib.dll 内。
    _nim_duilib_require_lib("duilib")
else()
    _nim_duilib_require_lib("duilib")
    _nim_duilib_require_lib("duilib-cximage")
    _nim_duilib_require_lib("duilib-webp")
    _nim_duilib_require_lib("duilib-png")
    _nim_duilib_require_lib("duilib-zlib")

    if(NIM_DUILIB_PACKAGE_HAS_SDL)
        set(_nim_sdl_lib "")
        if(NIM_DUILIB_PACKAGE_TOOLCHAIN STREQUAL "msvc")
            if(NIM_DUILIB_PACKAGE_SDL_KIND STREQUAL "shared")
                set(_nim_sdl_lib "${nim_duilib_LIB_DIR}/SDL3.lib")
            else()
                set(_nim_sdl_lib "${nim_duilib_LIB_DIR}/SDL3-static.lib")
            endif()
        elseif(NIM_DUILIB_PACKAGE_TOOLCHAIN STREQUAL "mingw")
            if(NIM_DUILIB_PACKAGE_SDL_KIND STREQUAL "shared")
                set(_nim_sdl_lib "${nim_duilib_LIB_DIR}/libSDL3.dll.a")
            else()
                set(_nim_sdl_lib "${nim_duilib_LIB_DIR}/libSDL3.a")
            endif()
        else()
            file(GLOB _nim_sdl_candidates
                "${nim_duilib_LIB_DIR}/libSDL3.so*"
                "${nim_duilib_LIB_DIR}/libSDL3*.dylib")
            list(SORT _nim_sdl_candidates)
            if(_nim_sdl_candidates)
                list(GET _nim_sdl_candidates 0 _nim_sdl_lib)
            elseif(EXISTS "${nim_duilib_LIB_DIR}/libSDL3.a")
                set(_nim_sdl_lib "${nim_duilib_LIB_DIR}/libSDL3.a")
            endif()
        endif()
        if(NOT EXISTS "${_nim_sdl_lib}")
            message(FATAL_ERROR
                "SDK 声明包含 SDL3（${NIM_DUILIB_PACKAGE_SDL_KIND}），但找不到对应的库："
                "${nim_duilib_LIB_DIR}")
        endif()
        list(APPEND _nim_link "${_nim_sdl_lib}")
    endif()

    _nim_duilib_add_lib("svg")
    _nim_duilib_add_lib("skshaper")
    _nim_duilib_add_lib("skottie")
    _nim_duilib_add_lib("sksg")
    _nim_duilib_add_lib("jsonreader")
    _nim_duilib_add_lib("bentleyottmann")
    _nim_duilib_add_lib("expat")
    _nim_duilib_add_lib("skcms")
    _nim_duilib_add_lib("skresources")
    # libskia 必须放在其他 Skia 库之后：GNU ld 对静态库只做单遍扫描，
    # 放在前面会导致 skottie/skresources/jsonreader 引用的 Skia 符号无法解析
    _nim_duilib_require_lib("skia")

    if(NIM_DUILIB_PACKAGE_HAS_JPEG)
        _nim_duilib_add_lib("turbojpeg-static")
    endif()
    if(NIM_DUILIB_PACKAGE_HAS_WEBVIEW2)
        _nim_duilib_add_lib("WebView2LoaderStatic")
    endif()
    if(NIM_DUILIB_PACKAGE_HAS_CEF)
        _nim_duilib_add_lib("libcef")
        if(NIM_DUILIB_PACKAGE_CEF_WRAPPER)
            _nim_duilib_add_lib("${NIM_DUILIB_PACKAGE_CEF_WRAPPER}")
        endif()
    endif()
endif()

set(_nim_compile_options "")
if(MSVC)
    # 与 VS 属性表一致：头文件含 UTF-8 中文注释，显式指定源码编码
    list(APPEND _nim_compile_options "/utf-8")
endif()

set(_nim_link_options "")
if(MSVC AND NIM_DUILIB_PACKAGE_CONFIG STREQUAL "Debug")
    # 与 VS 属性表一致：Debug 下屏蔽 release CRT，避免第三方预编译库触发 LNK4098
    list(APPEND _nim_link_options "/NODEFAULTLIB:libcmt.lib")
endif()
if(WIN32)
    list(APPEND _nim_link
        Comctl32 Imm32 Opengl32 User32 shlwapi)
    if(NIM_DUILIB_PACKAGE_HAS_SDL)
        list(APPEND _nim_link Version Winmm Setupapi)
    endif()
    if(NIM_DUILIB_PACKAGE_HAS_WEBVIEW2)
        list(APPEND _nim_link WinInet)
    endif()
    if(MSVC AND NIM_DUILIB_PACKAGE_HAS_CEF AND NOT NIM_DUILIB_PACKAGE_IS_DLL)
        list(APPEND _nim_link_options "/DELAYLOAD:libcef.dll")
    endif()
elseif(APPLE)
    find_library(_nim_accelerate Accelerate REQUIRED)
    find_library(_nim_corefoundation CoreFoundation REQUIRED)
    find_library(_nim_coretext CoreText REQUIRED)
    find_library(_nim_coregraphics CoreGraphics REQUIRED)
    list(APPEND _nim_link
        "${_nim_accelerate}" "${_nim_corefoundation}"
        "${_nim_coretext}" "${_nim_coregraphics}" pthread dl
        "-framework AppKit" "-framework Foundation"
        "-framework Metal" "-framework Cocoa")
else()
    find_package(X11 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(Fontconfig REQUIRED)
    list(APPEND _nim_link
        X11::X11 Freetype::Freetype Fontconfig::Fontconfig pthread dl)
endif()

# --------------------------------------------------------------------------
# imported target
# --------------------------------------------------------------------------

add_library(nim_duilib::duilib INTERFACE IMPORTED GLOBAL)
set_target_properties(nim_duilib::duilib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_nim_includes}"
    INTERFACE_COMPILE_DEFINITIONS "${_nim_defines}"
    INTERFACE_COMPILE_OPTIONS "${_nim_compile_options}"
    INTERFACE_COMPILE_FEATURES "cxx_std_20"
    INTERFACE_LINK_LIBRARIES "${_nim_link}"
    INTERFACE_LINK_OPTIONS "${_nim_link_options}")

# 供 nim_duilibHelpers.cmake 使用
set_property(GLOBAL PROPERTY NIM_DUILIB_ROOT "${nim_duilib_ROOT}")
set_property(GLOBAL PROPERTY NIM_DUILIB_RUNTIME_DLLS "${NIM_DUILIB_PACKAGE_RUNTIME_DLLS}")

# 常用查询变量
set(NIM_DUILIB_HAS_SDL "${NIM_DUILIB_PACKAGE_HAS_SDL}")
set(NIM_DUILIB_HAS_WEBVIEW2 "${NIM_DUILIB_PACKAGE_HAS_WEBVIEW2}")
set(NIM_DUILIB_HAS_JPEG "${NIM_DUILIB_PACKAGE_HAS_JPEG}")
set(NIM_DUILIB_HAS_CEF "${NIM_DUILIB_PACKAGE_HAS_CEF}")
set(NIM_DUILIB_IS_DLL "${NIM_DUILIB_PACKAGE_IS_DLL}")
set(nim_duilib_FOUND TRUE)

message(STATUS
    "nim_duilib ${NIM_DUILIB_PACKAGE_VERSION} (${NIM_DUILIB_PACKAGE_SYSTEM}-"
    "${NIM_DUILIB_PACKAGE_ARCH}, ${NIM_DUILIB_PACKAGE_CONFIG}, "
    "${NIM_DUILIB_PACKAGE_TOOLCHAIN})")
