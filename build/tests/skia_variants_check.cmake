# 构建系统 CMake 脚本的自动化测试入口（由 build/test_cmake_scripts.py 调用）
#
# 用法：
#   cmake -DDUILIB_REPO_ROOT=<repo> -DDUILIB_TEST_MODE=candidates -P 本文件
#   cmake -DDUILIB_REPO_ROOT=<repo> -DDUILIB_TEST_MODE=deps_check \
#         -DDUILIB_TEST_CASE=<用例名> -DDUILIB_TEST_SKIA_ROOT=<假 skia 目录> -P 本文件
#   cmake -DDUILIB_REPO_ROOT=<repo> -DDUILIB_TEST_MODE=prepare_skia \
#         -DDUILIB_TEST_CASE=<用例名> -DDUILIB_TEST_SKIA_ROOT=<假 skia 目录> \
#         -DDUILIB_TEST_DEPS_ROOT=<临时依赖目录> -P 本文件
#
# 覆盖范围（此前只能在 Windows 上手工跑 build.py 验证，单配置生成器的 bug
# “Debug 被当成 Release”因此没被发现）：
#   1. cmake/duilib_skia_variants.cmake 的变体目录规则：
#      MSVC Debug/Release × /MT、/MD、MinGW 独立版/MSYS2 × gcc、llvm、非 Windows。
#   2. cmake/duilib_deps.cmake 的 DUILIB_DEPS_MODE=off 依赖检查：
#      单配置 / 多配置生成器、Debug 变体缺失、MinGW 拒绝 MSVC 产物等。
#   3. cmake/duilib_deps_run.cmake 的 duilib_prepare_skia 多配置循环：
#      多配置生成器必须同时准备 Debug/Release 两个变体（此前只准备 Release，
#      VS 的 Debug 配置阶段报 “no usable Skia library directory”）。

cmake_minimum_required(VERSION 3.24)

if(NOT DUILIB_REPO_ROOT)
    get_filename_component(DUILIB_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()

# 输出一行供 Python 解析；Prefix 固定为 CASE|，避免与 cmake 自身的日志混淆。
function(_duilib_test_case name values)
    message("CASE|${name}|${values}")
endfunction()

if(DUILIB_TEST_MODE STREQUAL "candidates")
    include("${DUILIB_REPO_ROOT}/cmake/duilib_skia_variants.cmake")

    # 参数：用例名、配置、平台、架构、MinGW、/MD、GCC、MSYS2 环境、MinGW 前缀
    function(_duilib_test_variants name config platform arch mingw md gcc msys2 mingw_name)
        set(DUILIB_MD "${md}")
        set(DUILIB_COMPILER_GCC "${gcc}")
        set(DUILIB_MSYS2_ENV "${msys2}")
        set(DUILIB_MINGW_NAME "${mingw_name}")
        duilib_skia_variant_candidates("${config}" "${platform}" "${arch}" "${mingw}" _candidates)
        _duilib_test_case("${name}" "${_candidates}")
    endfunction()

    _duilib_test_variants("msvc-debug-mt"     debug   windows x64 FALSE OFF FALSE "" "")
    _duilib_test_variants("msvc-debug-md"     debug   windows x64 FALSE ON  FALSE "" "")
    _duilib_test_variants("msvc-release-mt"   release windows x64 FALSE OFF FALSE "" "")
    _duilib_test_variants("msvc-release-md"   release windows x64 FALSE ON  FALSE "" "")
    _duilib_test_variants("msvc-debug-x86"    debug   windows x86 FALSE OFF FALSE "" "")
    _duilib_test_variants("msvc-unknown-config" ""    windows x64 FALSE OFF FALSE "" "")
    # 独立 MinGW-w64：优先 mingw64- 前缀，另一组前缀作为回退
    _duilib_test_variants("mingw-gcc-debug"   debug   windows x64 TRUE  OFF TRUE  "" "mingw64-")
    _duilib_test_variants("mingw-llvm-release" release windows x64 TRUE  OFF FALSE "" "mingw64-")
    # MSYS2（无前缀）：UCRT64 等环境；MINGW64 环境优先 msvcrt 变体
    _duilib_test_variants("mingw-llvm-ucrt64-release" release windows x64 TRUE OFF FALSE "ucrt64" "")
    _duilib_test_variants("mingw-gcc-mingw64-debug"   debug   windows x64 TRUE OFF TRUE  "mingw64" "")
    # 非 Windows：只有 release 变体，允许 gcc 回退
    _duilib_test_variants("unix-release"      release other   x64 FALSE OFF FALSE "" "")
elseif(DUILIB_TEST_MODE STREQUAL "deps_check")
    include("${DUILIB_REPO_ROOT}/cmake/duilib_deps.cmake")

    # 模拟根工程作用域（duilib_root_options.cmake 已给出这些值），并显式清空
    # DUILIB_BUILD_TYPE：它在根工程里由 duilib_common.cmake 定义，此时尚未执行。
    unset(DUILIB_BUILD_TYPE)
    set(DUILIB_DEPS_MODE "off")
    set(DUILIB_ENABLE_SDL OFF)
    set(DUILIB_ENABLE_CEF OFF)
    set(DUILIB_SKIA_ROOT "${DUILIB_TEST_SKIA_ROOT}")
    set(DUILIB_TARGET_ARCH_RESOLVED "x64")
    set(DUILIB_OS_WINDOWS FALSE)
    set(MINGW FALSE)
    set(DUILIB_MD OFF)
    set(DUILIB_COMPILER_GCC FALSE)
    set(DUILIB_MSYS2_ENV "")
    set(DUILIB_MINGW_NAME "")
    set(CMAKE_BUILD_TYPE "")
    unset(CMAKE_CONFIGURATION_TYPES)

    if(DUILIB_TEST_CASE STREQUAL "msvc-debug-default-rt")
        # 回归用例：单配置 Debug 必须检查 debug 变体（此前误判为 Release）
        set(DUILIB_OS_WINDOWS TRUE)
        set(CMAKE_BUILD_TYPE "Debug")
    elseif(DUILIB_TEST_CASE STREQUAL "msvc-debug-md")
        set(DUILIB_OS_WINDOWS TRUE)
        set(CMAKE_BUILD_TYPE "Debug")
        set(DUILIB_MD ON)
    elseif(DUILIB_TEST_CASE STREQUAL "msvc-debug-missing")
        set(DUILIB_OS_WINDOWS TRUE)
        set(CMAKE_BUILD_TYPE "Debug")
    elseif(DUILIB_TEST_CASE STREQUAL "msvc-release")
        set(DUILIB_OS_WINDOWS TRUE)
        set(CMAKE_BUILD_TYPE "Release")
    elseif(DUILIB_TEST_CASE STREQUAL "mingw-debug-fallback")
        set(DUILIB_OS_WINDOWS TRUE)
        set(MINGW TRUE)
        set(DUILIB_COMPILER_GCC TRUE)
        set(DUILIB_MINGW_NAME "mingw64-")
        set(CMAKE_BUILD_TYPE "Debug")
    elseif(DUILIB_TEST_CASE STREQUAL "mingw-rejects-msvc-dir")
        set(DUILIB_OS_WINDOWS TRUE)
        set(MINGW TRUE)
        set(DUILIB_MINGW_NAME "mingw64-")
        set(CMAKE_BUILD_TYPE "Debug")
    elseif(DUILIB_TEST_CASE STREQUAL "multi-config-both")
        set(DUILIB_OS_WINDOWS TRUE)
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
    elseif(DUILIB_TEST_CASE STREQUAL "multi-config-missing-debug")
        set(DUILIB_OS_WINDOWS TRUE)
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
    elseif(DUILIB_TEST_CASE STREQUAL "unix-gcc-fallback")
        set(CMAKE_BUILD_TYPE "Release")
    else()
        message(FATAL_ERROR "unknown deps_check case: ${DUILIB_TEST_CASE}")
    endif()

    if(NOT EXISTS "${DUILIB_SKIA_ROOT}")
        message(FATAL_ERROR "fake Skia root not found: ${DUILIB_SKIA_ROOT}")
    endif()
    duilib_configure_dependencies()
    message("RESULT|ok")
elseif(DUILIB_TEST_MODE STREQUAL "prepare_skia")
    # 假 Skia 目录里已经有库文件与匹配的 .duilib-ref，让 _duilib_prepare_skia_config
    # 走“已就绪”的提前返回：不触发真正的 gn/ninja 编译，只验证
    # duilib_prepare_skia 到底准备了哪些变体。
    set(DUILIB_DEPS_MODE "off") # 脚本末尾的主流程不执行，由本文件直接调用函数
    set(DUILIB_DEPS_ROOT "${DUILIB_TEST_DEPS_ROOT}")
    set(DUILIB_VERSIONS_FILE "${DUILIB_REPO_ROOT}/cmake/third_party_versions.json")
    set(DUILIB_ROOT "${DUILIB_REPO_ROOT}")
    set(DUILIB_SKIA_ROOT "${DUILIB_TEST_SKIA_ROOT}")
    set(DUILIB_ARCH "x64")
    set(DUILIB_DEPS_PLATFORM "windows")
    set(DUILIB_MINGW FALSE)
    set(DUILIB_MD OFF)
    set(DUILIB_COMPILER_GCC FALSE)
    set(DUILIB_MSYS2_ENV "")
    set(DUILIB_MINGW_NAME "")
    set(DUILIB_SKIA_REF "test-ref")
    set(DUILIB_SKIA_REF_VALUE "test-value")
    set(DUILIB_CRT_ID "msvc-mt")
    set(DUILIB_SKIA_PATCH_ZIP "")
    set(DUILIB_SKIA_REQUIRE_PATCH OFF)
    set(DUILIB_DEPS_FORCE OFF)
    set(DUILIB_DEPS_JOBS 1)
    set(DUILIB_ENABLE_SDL OFF)
    set(DUILIB_ENABLE_CEF OFF)
    set(CMAKE_BUILD_TYPE "")

    if(DUILIB_TEST_CASE STREQUAL "multi-config")
        set(DUILIB_MULTI_CONFIG ON)
        set(DUILIB_CONFIG "Release")
    elseif(DUILIB_TEST_CASE STREQUAL "single-config-debug")
        set(DUILIB_MULTI_CONFIG OFF)
        set(DUILIB_CONFIG "Debug")
    elseif(DUILIB_TEST_CASE STREQUAL "single-config-release")
        set(DUILIB_MULTI_CONFIG OFF)
        set(DUILIB_CONFIG "Release")
    else()
        message(FATAL_ERROR "unknown prepare_skia case: ${DUILIB_TEST_CASE}")
    endif()

    include("${DUILIB_REPO_ROOT}/cmake/duilib_deps_run.cmake")
    duilib_prepare_skia()
    message("RESULT|ok")
else()
    message(FATAL_ERROR "usage: -DDUILIB_TEST_MODE=candidates|deps_check|prepare_skia")
endif()
