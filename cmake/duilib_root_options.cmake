# duilib 根工程：平台/编译器/架构识别与统一缓存选项
#
# 本文件在 project() 之后 include；所有写入缓存的值都优先于子项目里的 option()
# 默认值，避免子项目覆盖根工程的决策。

include("${CMAKE_CURRENT_LIST_DIR}/duilib_i18n.cmake")

get_filename_component(DUILIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# ---------------------------------------------------------------------------
# 平台
# ---------------------------------------------------------------------------
set(DUILIB_OS_WINDOWS FALSE)
set(DUILIB_OS_LINUX FALSE)
set(DUILIB_OS_MACOS FALSE)
set(DUILIB_OS_FREEBSD FALSE)
if(WIN32)
    set(DUILIB_OS_WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DUILIB_OS_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DUILIB_OS_MACOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(DUILIB_OS_FREEBSD TRUE)
else()
    message(FATAL_ERROR "[ERROR] unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# 编译器
set(DUILIB_COMPILER_MSVC FALSE)
set(DUILIB_COMPILER_GCC FALSE)
set(DUILIB_COMPILER_LLVM FALSE)
if(MSVC)
    set(DUILIB_COMPILER_MSVC TRUE)
    set(DUILIB_COMPILER_NAME "msvc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(DUILIB_COMPILER_GCC TRUE)
    set(DUILIB_COMPILER_NAME "gcc")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(DUILIB_COMPILER_LLVM TRUE)
    set(DUILIB_COMPILER_NAME "llvm")
else()
    duilib_error("preflight.compiler_unsupported" "preflight.compiler_unsupported_hint")
endif()

# MinGW：区分 MSYS2 与独立 MinGW-w64（决定 Skia 子目录前缀）
set(DUILIB_MSYS2_ENV "")
set(DUILIB_MINGW_NAME "")
if(MINGW)
    set(_duilib_cc "${CMAKE_CXX_COMPILER}")
    if(_duilib_cc STREQUAL "")
        set(_duilib_cc "${CMAKE_C_COMPILER}")
    endif()
    if(NOT _duilib_cc STREQUAL "")
        get_filename_component(_duilib_cc_dir "${_duilib_cc}" DIRECTORY)
        get_filename_component(_duilib_env_dir "${_duilib_cc_dir}" DIRECTORY)
        get_filename_component(_duilib_msys_root "${_duilib_env_dir}" DIRECTORY)
        if(EXISTS "${_duilib_msys_root}/usr/bin/pacman.exe")
            get_filename_component(DUILIB_MSYS2_ENV "${_duilib_env_dir}" NAME)
        endif()
    endif()
    if(NOT DUILIB_MSYS2_ENV STREQUAL "")
        set(DUILIB_MINGW_NAME "")
    else()
        set(DUILIB_MINGW_NAME "mingw64-")
    endif()
endif()

# ---------------------------------------------------------------------------
# 依赖目录（与旧脚本一致：仓库同级目录）
# ---------------------------------------------------------------------------
set(DUILIB_DEPS_ROOT "${CMAKE_SOURCE_DIR}/.." CACHE PATH "Root directory of the sibling dependency checkouts")
get_filename_component(DUILIB_DEPS_ROOT "${DUILIB_DEPS_ROOT}" ABSOLUTE)

set(DUILIB_VERSIONS_FILE "${CMAKE_CURRENT_LIST_DIR}/third_party_versions.json"
    CACHE FILEPATH "Third-party version manifest")

set(DUILIB_SKIA_ROOT "${DUILIB_DEPS_ROOT}/skia" CACHE PATH "Skia source tree")
set(DUILIB_SKIA_PATCH_DIR "${CMAKE_CURRENT_LIST_DIR}/../build/skia_patch"
    CACHE PATH "Skia patch archives shipped with the repository")
set(DUILIB_SDL_SRC "${DUILIB_DEPS_ROOT}/SDL" CACHE PATH "SDL3 source tree")
set(DUILIB_SDL_ROOT "${DUILIB_DEPS_ROOT}/SDL3" CACHE PATH "SDL3 installation prefix")
set(DUILIB_SDL_BUILD_ROOT "${DUILIB_DEPS_ROOT}/SDL3_build" CACHE PATH "SDL3 build directory")
set(DUILIB_CEF_ROOT "${DUILIB_DEPS_ROOT}/cef_binary" CACHE PATH "CEF binary distribution root (macOS)")
set(DUILIB_CEF_CACHE "${DUILIB_DEPS_ROOT}/cef_binary" CACHE PATH "CEF download/extract cache")

# ---------------------------------------------------------------------------
# 版本清单
# ---------------------------------------------------------------------------
if(NOT EXISTS "${DUILIB_VERSIONS_FILE}")
    duilib_error_values("${DUILIB_VERSIONS_FILE} not found" "restore cmake/third_party_versions.json")
endif()
file(READ "${DUILIB_VERSIONS_FILE}" DUILIB_VERSIONS_JSON)

foreach(_duilib_section skia sdl cef)
    string(JSON _duilib_section_default ERROR_VARIABLE _duilib_json_err
           GET "${DUILIB_VERSIONS_JSON}" "${_duilib_section}" "default")
    if(_duilib_json_err OR _duilib_section_default STREQUAL "")
        duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
    endif()
    set(_duilib_default_${_duilib_section} "${_duilib_section_default}")
endforeach()

string(JSON DUILIB_SKIA_REPO ERROR_VARIABLE _duilib_json_err GET "${DUILIB_VERSIONS_JSON}" skia repo)
if(_duilib_json_err)
    duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
endif()
string(JSON DUILIB_SDL_REPO ERROR_VARIABLE _duilib_json_err GET "${DUILIB_VERSIONS_JSON}" sdl repo)
if(_duilib_json_err)
    duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
endif()
string(JSON DUILIB_CEF_URL_BASE ERROR_VARIABLE _duilib_json_err GET "${DUILIB_VERSIONS_JSON}" cef baseUrl)
if(_duilib_json_err)
    duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
endif()

set(DUILIB_SKIA_REF "${_duilib_default_skia}" CACHE STRING "Skia ref key from third_party_versions.json")
set(DUILIB_SDL_REF "${_duilib_default_sdl}" CACHE STRING "SDL3 ref key from third_party_versions.json")
# 校验所选 key 存在
string(JSON _duilib_unused ERROR_VARIABLE _duilib_json_err
       GET "${DUILIB_VERSIONS_JSON}" skia refs "${DUILIB_SKIA_REF}")
if(_duilib_json_err)
    duilib_t("deps.ref_unknown" _duilib_ref_msg)
    string(REPLACE "{section}" "skia" _duilib_ref_msg "${_duilib_ref_msg}")
    string(REPLACE "{key}" "${DUILIB_SKIA_REF}" _duilib_ref_msg "${_duilib_ref_msg}")
    duilib_t("deps.ref_unknown_hint" _duilib_ref_hint)
    string(REPLACE "{section}" "skia" _duilib_ref_hint "${_duilib_ref_hint}")
    duilib_error_values("${_duilib_ref_msg}" "${_duilib_ref_hint}")
endif()
string(JSON _duilib_unused ERROR_VARIABLE _duilib_json_err
       GET "${DUILIB_VERSIONS_JSON}" sdl refs "${DUILIB_SDL_REF}")
if(_duilib_json_err)
    duilib_t("deps.ref_unknown" _duilib_ref_msg)
    string(REPLACE "{section}" "sdl" _duilib_ref_msg "${_duilib_ref_msg}")
    string(REPLACE "{key}" "${DUILIB_SDL_REF}" _duilib_ref_msg "${_duilib_ref_msg}")
    duilib_t("deps.ref_unknown_hint" _duilib_ref_hint)
    string(REPLACE "{section}" "sdl" _duilib_ref_hint "${_duilib_ref_hint}")
    duilib_error_values("${_duilib_ref_msg}" "${_duilib_ref_hint}")
endif()
string(JSON _duilib_skia_ref_type ERROR_VARIABLE _duilib_json_err
       GET "${DUILIB_VERSIONS_JSON}" skia refs "${DUILIB_SKIA_REF}" type)
string(JSON _duilib_skia_ref_value ERROR_VARIABLE _duilib_json_err2
       GET "${DUILIB_VERSIONS_JSON}" skia refs "${DUILIB_SKIA_REF}" value)
if(_duilib_json_err OR _duilib_json_err2)
    duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
endif()
set(DUILIB_SKIA_REF_TYPE "${_duilib_skia_ref_type}")
set(DUILIB_SKIA_REF_VALUE "${_duilib_skia_ref_value}")
string(JSON DUILIB_SDL_REF_VALUE ERROR_VARIABLE _duilib_json_err
       GET "${DUILIB_VERSIONS_JSON}" sdl refs "${DUILIB_SDL_REF}" value)
string(JSON _duilib_sdl_ref_type ERROR_VARIABLE _duilib_json_err2
       GET "${DUILIB_VERSIONS_JSON}" sdl refs "${DUILIB_SDL_REF}" type)
if(_duilib_json_err OR _duilib_json_err2)
    duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
endif()
# 依赖脚本据此把本地源码切到清单指定的提交（tag / branch / commit）
set(DUILIB_SDL_REF_TYPE "${_duilib_sdl_ref_type}")

# Skia 补丁包（相对 DUILIB_SKIA_PATCH_DIR，缺省 build/skia_patch）
set(DUILIB_SKIA_PATCH_ZIP "")
string(JSON _duilib_skia_patch ERROR_VARIABLE _duilib_json_err
       GET "${DUILIB_VERSIONS_JSON}" skia refs "${DUILIB_SKIA_REF}" patchZip)
if(NOT _duilib_json_err)
    set(DUILIB_SKIA_PATCH_ZIP "${_duilib_skia_patch}")
endif()

# 补丁归档目录由清单 skia.patchDir 指定（相对仓库根），归档随仓库提交。
string(JSON _duilib_skia_patch_dir ERROR_VARIABLE _duilib_json_err
       GET "${DUILIB_VERSIONS_JSON}" skia patchDir)
if(NOT _duilib_json_err AND NOT _duilib_skia_patch_dir STREQUAL "")
    get_filename_component(_duilib_skia_patch_dir_abs
        "${CMAKE_CURRENT_LIST_DIR}/../${_duilib_skia_patch_dir}" ABSOLUTE)
    set(DUILIB_SKIA_PATCH_DIR "${_duilib_skia_patch_dir_abs}" CACHE PATH
        "Skia patch archives shipped with the repository" FORCE)
endif()

# 旧机制（仓库外 skia_compile）留下的缓存项已没有任何选项对应，复用的构建树里
# 会一直显示失效路径；在配置期清掉，避免用户误以为仍在使用它们。
foreach(_duilib_stale_cache DUILIB_SKIA_COMPILE_ROOT DUILIB_SKIA_COMPILE_REPO)
    unset(${_duilib_stale_cache} CACHE)
endforeach()

# 统一网络重试参数（0 = 无限重试）；单一维护点在清单的 retry 段
string(JSON _duilib_retry_max ERROR_VARIABLE _duilib_json_err
       GET "${DUILIB_VERSIONS_JSON}" retry maxAttempts)
if(_duilib_json_err)
    set(_duilib_retry_max 0)
endif()
string(JSON _duilib_retry_delay ERROR_VARIABLE _duilib_json_err
       GET "${DUILIB_VERSIONS_JSON}" retry delaySeconds)
if(_duilib_json_err)
    set(_duilib_retry_delay 20)
endif()

# ---------------------------------------------------------------------------
# CEF 版本
# ---------------------------------------------------------------------------
set(DUILIB_CEF_VERSION "${_duilib_default_cef}" CACHE STRING "CEF version key (109 / 156)")
option(DUILIB_CEF_109 "Legacy switch: use CEF 109" OFF)
if(DUILIB_CEF_109)
    duilib_warn("options.cef_109_deprecated")
    set(DUILIB_CEF_VERSION "109" CACHE STRING "CEF version key (109 / 156)" FORCE)
endif()
string(JSON _duilib_unused ERROR_VARIABLE _duilib_json_err
       GET "${DUILIB_VERSIONS_JSON}" cef versions "${DUILIB_CEF_VERSION}")
if(_duilib_json_err)
    duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
endif()

# ---------------------------------------------------------------------------
# 目标架构
# ---------------------------------------------------------------------------
set(DUILIB_TARGET_ARCH "auto" CACHE STRING "Target architecture: auto / x86 / x64 / arm64")
set(_duilib_arch "${DUILIB_TARGET_ARCH}")
if(_duilib_arch STREQUAL "auto" OR _duilib_arch STREQUAL "")
    if(DUILIB_OS_WINDOWS)
        if(CMAKE_GENERATOR_PLATFORM)
            string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" _duilib_arch)
            if(_duilib_arch STREQUAL "win32")
                set(_duilib_arch "x86")
            elseif(_duilib_arch MATCHES "arm64")
                set(_duilib_arch "arm64")
            else()
                set(_duilib_arch "x64")
            endif()
        elseif(DEFINED ENV{PROCESSOR_ARCHITEW6432} AND
               "$ENV{PROCESSOR_ARCHITEW6432}" MATCHES "ARM64")
            set(_duilib_arch "arm64")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(_duilib_arch "x86")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
            set(_duilib_arch "arm64")
        else()
            set(_duilib_arch "x64")
        endif()
    else()
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
            set(_duilib_arch "arm64")
        else()
            set(_duilib_arch "x64")
        endif()
    endif()
endif()

if(NOT _duilib_arch MATCHES "^(x86|x64|arm64)$")
    duilib_error("preflight.arch_unsupported" "preflight.arch_unsupported_hint")
endif()
if(NOT DUILIB_OS_WINDOWS AND _duilib_arch STREQUAL "x86")
    duilib_error_values("x86 is only supported on Windows (detected: ${CMAKE_SYSTEM_NAME}/${_duilib_arch})"
                        "use -DDUILIB_TARGET_ARCH=x64 or arm64")
endif()
if(_duilib_arch STREQUAL "x86" AND NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    duilib_error_values("DUILIB_TARGET_ARCH=x86 but the toolchain targets ${CMAKE_SIZEOF_VOID_P} bytes pointers"
                        "use a 32-bit toolchain/platform or -DDUILIB_TARGET_ARCH=x64")
endif()
if((_duilib_arch STREQUAL "x64" OR _duilib_arch STREQUAL "arm64") AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    duilib_error_values("DUILIB_TARGET_ARCH=${_duilib_arch} but the toolchain targets ${CMAKE_SIZEOF_VOID_P} bytes pointers"
                        "use a 64-bit toolchain/platform or -DDUILIB_TARGET_ARCH=x86")
endif()
if(DUILIB_OS_WINDOWS AND CMAKE_GENERATOR_PLATFORM)
    string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" _duilib_generator_arch)
    if(_duilib_generator_arch STREQUAL "win32")
        set(_duilib_generator_arch "x86")
    elseif(_duilib_generator_arch MATCHES "arm64")
        set(_duilib_generator_arch "arm64")
    else()
        set(_duilib_generator_arch "x64")
    endif()
    if(NOT _duilib_generator_arch STREQUAL _duilib_arch)
        duilib_error_values("DUILIB_TARGET_ARCH=${_duilib_arch} but CMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
                            "make the two match, or omit -DDUILIB_TARGET_ARCH")
    endif()
endif()

set(DUILIB_TARGET_ARCH_RESOLVED "${_duilib_arch}")
# 供 duilib_common.cmake 使用（子项目会读取这两个变量，不再重复推导）
set(DUILIB_SKIA_CPU_ARCH "${_duilib_arch}" CACHE STRING "Skia CPU architecture (derived)" FORCE)
set(DUILIB_SYSTEM_PROCESSOR "${_duilib_arch}" CACHE STRING "Target processor (derived)" FORCE)
set(DUILIB_BITS_64 FALSE)
if(_duilib_arch STREQUAL "x64" OR _duilib_arch STREQUAL "arm64")
    set(DUILIB_BITS_64 TRUE)
endif()

duilib_msg("options.arch_resolved" " ${_duilib_arch}")

# ---------------------------------------------------------------------------
# 功能开关
# ---------------------------------------------------------------------------
option(DUILIB_BUILD_THIRD_PARTY "Build zlib/libpng/cximage/libwebp" ON)
option(DUILIB_BUILD_EXAMPLES "Build example applications" ON)
# macOS App Bundle 示例列表（仅 Darwin 生效）：分号/逗号分隔的 CMake 目标名，
# all 或 * 表示全部示例。示例自己的 CMakeLists.txt 中设置的
# DUILIB_EXAMPLE_MACOS_BUNDLE 优先于本列表。
set(DUILIB_MACOS_BUNDLE_EXAMPLES "" CACHE STRING
    "macOS examples to build as .app bundles (semicolon/comma separated; all = every example)")
option(DUILIB_MD "Use the MSVC dynamic runtime (/MD, /MDd)" OFF)
# DLL 模式：duilib 编译为 bin/duilib*.dll，下游只链接导入库。
# 仅支持 Windows MSVC（/MD 是 DLL 的必然选择），并自动打开 DUILIB_MD。
option(DUILIB_DLL "Build duilib as a shared library (Windows MSVC, implies /MD)" OFF)
if(DUILIB_DLL)
    if(NOT DUILIB_OS_WINDOWS OR NOT DUILIB_COMPILER_MSVC)
        message(FATAL_ERROR
            "[ERROR] DUILIB_DLL=ON only supports Windows with MSVC "
            "(current: ${CMAKE_SYSTEM_NAME}, ${CMAKE_CXX_COMPILER_ID}).\n"
            "Use the static build, or a Windows MSVC toolchain.")
    endif()
    set(DUILIB_MD ON CACHE BOOL "Use the MSVC dynamic runtime (/MD, /MDd)" FORCE)
endif()
option(DUILIB_SDL_RPATH "Write rpath entries on Unix (relative first)" ON)
option(DUILIB_RPATH_ABSOLUTE "Include absolute fallback paths in the Unix rpath" ON)
option(DUILIB_OFFLINE "Use only cached dependency sources/archives" OFF)
option(DUILIB_DEPS_AUTO_CLONE "Allow git clone while preparing dependencies" ON)
option(DUILIB_DEPS_FORCE "Rebuild dependencies even when stamps match" OFF)
option(DUILIB_SKIA_REQUIRE_PATCH "Require the manifest patchZip for the selected Skia ref" ON)
option(DUILIB_SKIA_AUTOBUILD "Skip the Skia existence check (deps builds it first)" OFF)
set(DUILIB_DEPS_LOCK_TIMEOUT "5" CACHE STRING "Dependency preparation lock timeout in seconds")
# 值来自 third_party_versions.json 的 retry 段，修改清单即可统一调整
set(DUILIB_DOWNLOAD_RETRIES "${_duilib_retry_max}" CACHE STRING
    "Network retries: 0 = unlimited (retry.maxAttempts in third_party_versions.json)" FORCE)
set(DUILIB_DOWNLOAD_RETRY_DELAY "${_duilib_retry_delay}" CACHE STRING
    "Retry interval in seconds (retry.delaySeconds in third_party_versions.json)" FORCE)
set(DUILIB_DOWNLOAD_CONNECT_TIMEOUT "30" CACHE STRING "Download connect timeout in seconds")
set(DUILIB_DEPS_JOBS "6" CACHE STRING "Parallel jobs for Skia/SDL dependency builds")
# Skia 在 Windows 上需要 LLVM 根目录（gn 的 clang_win 参数）。
# 不写死 C:/LLVM：显式 -D 优先，其次按环境变量与常见安装位置探测。
if(NOT DEFINED DUILIB_LLVM_ROOT)
    set(DUILIB_LLVM_ROOT "")
    foreach(_duilib_llvm_candidate
            "$ENV{LLVM_ROOT}"
            "C:/LLVM"
            "$ENV{ProgramFiles}/LLVM"
            "$ENV{VCINSTALLDIR}/Tools/Llvm/x64"
            "$ENV{VSINSTALLDIR}/VC/Tools/Llvm/x64")
        if(_duilib_llvm_candidate AND EXISTS "${_duilib_llvm_candidate}/bin/clang.exe")
            file(TO_CMAKE_PATH "${_duilib_llvm_candidate}" DUILIB_LLVM_ROOT)
            break()
        endif()
    endforeach()
endif()
set(DUILIB_LLVM_ROOT "${DUILIB_LLVM_ROOT}" CACHE PATH "LLVM root used by Skia on Windows")
set(DUILIB_GN_EXECUTABLE "" CACHE FILEPATH "Override the gn executable")
set(DUILIB_NINJA_EXECUTABLE "" CACHE FILEPATH "Override the ninja executable")
set(DUILIB_CEF_ARCHIVE "" CACHE FILEPATH "Local CEF archive (skip download)")
option(DUILIB_CEF_REQUIRED "Fail when the CEF runtime cannot be prepared" ON)

set(DUILIB_DEPS_MODE "off" CACHE STRING "Dependency mode: off / prepare / auto")
set_property(CACHE DUILIB_DEPS_MODE PROPERTY STRINGS off prepare auto)
if(NOT DUILIB_DEPS_MODE MATCHES "^(off|prepare|auto)$")
    duilib_error_values("invalid DUILIB_DEPS_MODE=${DUILIB_DEPS_MODE}" "use off, prepare or auto")
endif()

# CEF 默认值：Windows MSVC / Linux / macOS 开启；MinGW、MSYS2、FreeBSD 关闭
set(_duilib_cef_default OFF)
if((DUILIB_OS_WINDOWS AND DUILIB_COMPILER_MSVC) OR DUILIB_OS_LINUX OR DUILIB_OS_MACOS)
    set(_duilib_cef_default ON)
endif()
if(NOT DEFINED DUILIB_ENABLE_CEF)
    set(DUILIB_ENABLE_CEF ${_duilib_cef_default} CACHE BOOL "Enable CEF")
endif()
if(DUILIB_ENABLE_CEF AND NOT _duilib_cef_default)
    duilib_warn("options.cef_disabled_platform")
    set(DUILIB_ENABLE_CEF OFF CACHE BOOL "Enable CEF" FORCE)
endif()

# SDL 默认值：Windows 可选（默认关闭），其他平台强制开启
if(DUILIB_OS_WINDOWS)
    if(NOT DEFINED DUILIB_ENABLE_SDL)
        set(DUILIB_ENABLE_SDL OFF CACHE BOOL "Enable SDL")
    endif()
else()
    if(DEFINED DUILIB_ENABLE_SDL AND NOT DUILIB_ENABLE_SDL)
        duilib_warn("options.sdl_forced_on")
    endif()
    set(DUILIB_ENABLE_SDL ON CACHE BOOL "Enable SDL" FORCE)
endif()

# CRT 标识：写入依赖 stamp，配置期比对
if(DUILIB_COMPILER_MSVC)
    if(DUILIB_MD)
        set(DUILIB_CRT_ID "msvc-md")
    else()
        set(DUILIB_CRT_ID "msvc-mt")
    endif()
elseif(MINGW)
    if(DEFINED ENV{MSYSTEM} AND "$ENV{MSYSTEM}" STREQUAL "MINGW64")
        set(DUILIB_CRT_ID "mingw64-msvcrt")
    elseif(DEFINED ENV{MSYSTEM} AND NOT "$ENV{MSYSTEM}" STREQUAL "")
        string(TOLOWER "$ENV{MSYSTEM}" _duilib_msystem)
        set(DUILIB_CRT_ID "${_duilib_msystem}")
    else()
        set(DUILIB_CRT_ID "mingw-unknown")
    endif()
else()
    set(DUILIB_CRT_ID "system")
endif()

# 构建类型：单配置生成器必须显式给出；多配置生成器保留为空
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug/Release)" FORCE)
endif()

# ---------------------------------------------------------------------------
# 示例列表
# ---------------------------------------------------------------------------
set(_duilib_examples_common
    basic controls ColorPicker DpiAware chat layout ListBox ListCtrl MoveControl
    MultiLang render RichEdit VirtualListBox threads TreeView ChildWindow
    XmlPreview ColorTheme panel)
# 平台基础示例（不含 CEF 示例）与 CEF 开启时的默认列表。
set(_duilib_examples_base ${_duilib_examples_common})
set(_duilib_examples_cef ${_duilib_examples_common})
if(DUILIB_OS_WINDOWS)
    list(APPEND _duilib_examples_base WebView2 WebView2Browser)
    list(APPEND _duilib_examples_cef WebView2 WebView2Browser)
endif()
if(DUILIB_OS_WINDOWS OR DUILIB_OS_LINUX OR DUILIB_OS_MACOS)
    list(APPEND _duilib_examples_cef cef CefBrowser)
endif()
# 本次的自动推导结果：CEF 开启时包含 cef/CefBrowser。
set(_duilib_examples_auto ${_duilib_examples_base})
if(DUILIB_ENABLE_CEF AND (DUILIB_OS_WINDOWS OR DUILIB_OS_LINUX OR DUILIB_OS_MACOS))
    list(APPEND _duilib_examples_auto cef CefBrowser)
endif()

# 用户显式列表优先。DUILIB_EXAMPLES 会写进缓存，但缓存里的值可能只是上一次
# 自动推导的结果（例如上次 DUILIB_ENABLE_CEF=ON 时含 cef/CefBrowser）。关闭
# CEF 后若把旧缓存值当成用户指定，仍会去构建 cef/CefBrowser，链接时报
# “CefManager::GetInstance 无法解析”。因此只有缓存值既不等于本次推导结果，也
# 不等于“CEF 开/关”的任一默认列表，也不等于上次记录的推导值时，才当作显式
# 覆盖（`-DDUILIB_EXAMPLES=...` 或 cmake-gui 修改）。
set(_duilib_examples_prev_auto "")
if(DEFINED DUILIB_EXAMPLES_AUTO)
    set(_duilib_examples_prev_auto "${DUILIB_EXAMPLES_AUTO}")
endif()
set(_duilib_examples ${_duilib_examples_auto})
if(DEFINED DUILIB_EXAMPLES AND NOT DUILIB_EXAMPLES STREQUAL ""
   AND NOT DUILIB_EXAMPLES STREQUAL _duilib_examples_prev_auto
   AND NOT DUILIB_EXAMPLES STREQUAL _duilib_examples_auto
   AND NOT DUILIB_EXAMPLES STREQUAL _duilib_examples_base
   AND NOT DUILIB_EXAMPLES STREQUAL _duilib_examples_cef)
    set(_duilib_examples ${DUILIB_EXAMPLES})
endif()
set(DUILIB_EXAMPLES "${_duilib_examples}" CACHE STRING
    "Example applications to build" FORCE)
set(DUILIB_EXAMPLES_AUTO "${_duilib_examples_auto}" CACHE INTERNAL
    "Platform default example list (derived, used to detect stale cache)")

# ---------------------------------------------------------------------------
# Unix rpath：优先加载包内的 SDL3 与 CEF 运行时，构建树用依赖前缀兜底
#
# 相对条目必须排在绝对条目之前：打包后的示例程序与库的相对位置是
#   bin/<exe>、lib/libSDL3.so*、bin/libcef_linux/libcef.so
# 这样解包即可运行，不会反过来加载构建机上的同名库。
#
# DUILIB_RPATH_ABSOLUTE=OFF 时只保留相对条目，产物完全可迁移；此时构建树中的
# 示例需要自行设置 LD_LIBRARY_PATH/DYLD_LIBRARY_PATH，打包后的程序不受影响。
#
# 构建树也使用 INSTALL_RPATH：CMake 默认会把 target_link_directories 里的
# 绝对路径（lib/<Config>、Skia 输出目录等）写进 BUILD_RPATH，而这些目录只
# 提供静态库，既不必要又会让产物带上构建机路径。统一使用 INSTALL_RPATH 后，
# 构建树与打包产物中的 rpath 完全一致。
# ---------------------------------------------------------------------------
if(NOT DUILIB_OS_WINDOWS AND DUILIB_SDL_RPATH)
    if(DUILIB_OS_MACOS)
        set(_duilib_origin "@loader_path")
    else()
        set(_duilib_origin "$ORIGIN")
    endif()
    set(_duilib_rpath "${_duilib_origin}")
    if(DUILIB_ENABLE_CEF AND DUILIB_OS_LINUX)
        # 构建树中即 <repo>/bin/libcef_linux；包内路径相同
        list(APPEND _duilib_rpath "${_duilib_origin}/libcef_linux")
    endif()
    if(DUILIB_ENABLE_SDL)
        # 包内 SDL3 在 lib/
        list(APPEND _duilib_rpath "${_duilib_origin}/../lib")
        if(DUILIB_RPATH_ABSOLUTE)
            # 构建树没有 lib/libSDL3*.dylib 时，用依赖前缀兜底
            if(EXISTS "${DUILIB_SDL_ROOT}/lib64")
                list(APPEND _duilib_rpath "${DUILIB_SDL_ROOT}/lib64")
            else()
                list(APPEND _duilib_rpath "${DUILIB_SDL_ROOT}/lib")
            endif()
        endif()
    endif()
    if(DUILIB_OS_FREEBSD AND DUILIB_RPATH_ABSOLUTE)
        # ports 安装的 fontconfig/freetype/X11 默认位置
        list(APPEND _duilib_rpath "/usr/local/lib")
    endif()
    if(_duilib_rpath)
        list(REMOVE_DUPLICATES _duilib_rpath)
        set(CMAKE_BUILD_RPATH "${_duilib_rpath}")
        set(CMAKE_INSTALL_RPATH "${_duilib_rpath}")
        set(CMAKE_SKIP_RPATH FALSE)
        set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    endif()
endif()

# SDL 库目录（供子项目链接使用）
if(EXISTS "${DUILIB_SDL_ROOT}/lib64")
    set(DUILIB_SDL_LIB_DIR "${DUILIB_SDL_ROOT}/lib64")
else()
    set(DUILIB_SDL_LIB_DIR "${DUILIB_SDL_ROOT}/lib")
endif()

# MSVC 运行库：在创建任何目标之前写入，子项目会继承
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
if(DUILIB_COMPILER_MSVC)
    if(DUILIB_MD)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# 依赖准备需要的目标平台标识（供 duilib_deps_run.cmake 使用）
if(DUILIB_OS_WINDOWS)
    set(DUILIB_DEPS_PLATFORM "windows")
elseif(DUILIB_OS_LINUX)
    set(DUILIB_DEPS_PLATFORM "linux")
elseif(DUILIB_OS_MACOS)
    set(DUILIB_DEPS_PLATFORM "macos")
else()
    set(DUILIB_DEPS_PLATFORM "freebsd")
endif()
