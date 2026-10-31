# Skia 变体规则 / off 模式检查 / 依赖准备 的 CMake 侧测试夹具
#
# 由 build/test_cmake_scripts.py 以 `cmake -D... -P` 调用，结果从 stdout 的
# "RESULT:" 行读取。三种模式：
#
#   DUILIB_TEST_MODE=candidates
#     验证 cmake/duilib_skia_variants.cmake 的候选目录名（与 pack.py 的打包侧比对）
#
#   DUILIB_TEST_MODE=deps_check
#     验证 cmake/duilib_deps.cmake 的 off 模式检查（逐配置、MinGW 只认 .a）
#
#   DUILIB_TEST_MODE=prepare_skia
#     验证 cmake/duilib_deps_run.cmake 的准备流程：多配置生成器下 Debug/Release
#     两份都要准备；单配置只准备当前配置。用假产物目录让流程直接“采纳”而不编译。
#
# 注意：写 .duilib-ref 必须使用 file(WRITE)（无 BOM），否则 stamp 比较会失配。

cmake_minimum_required(VERSION 3.24)

get_filename_component(DUILIB_TEST_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(DUILIB_LANG "en")

if("${DUILIB_TEST_MODE}" STREQUAL "candidates")
    set(DUILIB_MD "${DUILIB_TEST_MD}")
    set(DUILIB_COMPILER_GCC "${DUILIB_TEST_GCC}")
    set(DUILIB_MSYS2_ENV "${DUILIB_TEST_MSYS2_ENV}")
    set(DUILIB_MINGW_NAME "${DUILIB_TEST_MINGW_NAME}")
    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_skia_variants.cmake")
    duilib_skia_variant_candidates("${DUILIB_TEST_CONFIG}" "${DUILIB_TEST_PLATFORM}"
                                   "${DUILIB_TEST_ARCH}" "${DUILIB_TEST_MINGW}" _result)
    message(STATUS "RESULT:${_result}")

elseif("${DUILIB_TEST_MODE}" STREQUAL "deps_check")
    set(DUILIB_SKIA_ROOT "${DUILIB_TEST_SKIA_ROOT}")
    set(DUILIB_OS_WINDOWS "${DUILIB_TEST_WINDOWS}")
    set(DUILIB_TARGET_ARCH_RESOLVED "${DUILIB_TEST_ARCH}")
    set(DUILIB_SYSTEM_PROCESSOR "${DUILIB_TEST_ARCH}")
    set(DUILIB_COMPILER_GCC "${DUILIB_TEST_GCC}")
    set(DUILIB_MSYS2_ENV "${DUILIB_TEST_MSYS2_ENV}")
    set(DUILIB_MINGW_NAME "${DUILIB_TEST_MINGW_NAME}")
    set(DUILIB_MD "${DUILIB_TEST_MD}")
    if("${DUILIB_TEST_CONFIGURATION_TYPES}" STREQUAL "")
        set(CMAKE_BUILD_TYPE "${DUILIB_TEST_CONFIG}")
        set(CMAKE_CONFIGURATION_TYPES "")
    else()
        # 多配置生成器：CMAKE_BUILD_TYPE 为空，按 CMAKE_CONFIGURATION_TYPES 逐配置检查
        set(CMAKE_BUILD_TYPE "")
        set(CMAKE_CONFIGURATION_TYPES "${DUILIB_TEST_CONFIGURATION_TYPES}")
    endif()
    set(MINGW "${DUILIB_TEST_MINGW}")
    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_deps.cmake")
    _duilib_check_skia()
    message(STATUS "RESULT:OK")

elseif("${DUILIB_TEST_MODE}" STREQUAL "common")
    # 直接 include cmake/duilib_common.cmake（它通常只在示例子工程里被包含），
    # 验证作用域映射、autobuild 路径解析与 CRT 比对都调用共享实现。
    set(MSVC "${DUILIB_TEST_MSVC}")
    set(MINGW "${DUILIB_TEST_MINGW}")
    if(DUILIB_TEST_WINDOWS)
        set(CMAKE_SYSTEM_NAME "Windows")
        set(CMAKE_SYSTEM_PROCESSOR "AMD64")
    else()
        set(CMAKE_SYSTEM_NAME "Linux")
        set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    endif()
    set(CMAKE_SIZEOF_VOID_P 8)
    set(CMAKE_CXX_COMPILER_ID "${DUILIB_TEST_COMPILER_ID}")
    set(CMAKE_CXX_COMPILER "${DUILIB_TEST_CXX_COMPILER}")
    set(CMAKE_C_COMPILER "${DUILIB_TEST_CXX_COMPILER}")
    set(CMAKE_BUILD_TYPE "${DUILIB_TEST_CONFIG}")
    set(CMAKE_CONFIGURATION_TYPES "${DUILIB_TEST_CONFIGURATION_TYPES}")
    set(DUILIB_SKIA_CPU_ARCH "${DUILIB_TEST_ARCH}")
    set(DUILIB_TARGET_ARCH_RESOLVED "${DUILIB_TEST_ARCH}")
    set(DUILIB_SYSTEM_PROCESSOR "${DUILIB_TEST_ARCH}")
    set(DUILIB_SKIA_ROOT "${DUILIB_TEST_SKIA_ROOT}")
    set(DUILIB_SKIA_AUTOBUILD "${DUILIB_TEST_AUTOBUILD}")
    set(DUILIB_MD "${DUILIB_TEST_MD}")
    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_common.cmake")
    if(NOT "${DUILIB_TEST_CRT_DIR}" STREQUAL "")
        _duilib_check_deps_crt("test" "${DUILIB_TEST_CRT_DIR}")
    endif()
    message(STATUS "RESULT:${DUILIB_SKIA_LIB_SUBPATH}")

elseif("${DUILIB_TEST_MODE}" STREQUAL "crt")
    # CRT 标识的唯一推导：与 duilib_root_options.cmake / duilib_common.cmake 共用
    set(MSVC "${DUILIB_TEST_MSVC}")
    set(MINGW "${DUILIB_TEST_MINGW}")
    set(DUILIB_MD "${DUILIB_TEST_MD}")
    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_crt.cmake")
    duilib_crt_id(_crt)
    message(STATUS "RESULT:${_crt}")

elseif("${DUILIB_TEST_MODE}" STREQUAL "prepare_skia")
    set(DUILIB_DEPS_MODE "off")   # 主流程只打印提示，随后手动调用 duilib_prepare_skia
    set(DUILIB_DEPS_FORCE OFF)
    set(DUILIB_DEPS_ROOT "${DUILIB_TEST_DEPS_ROOT}")
    set(DUILIB_SKIA_ROOT "${DUILIB_TEST_SKIA_ROOT}")
    set(DUILIB_ARCH "${DUILIB_TEST_ARCH}")
    set(DUILIB_CONFIG "${DUILIB_TEST_CONFIG}")
    set(DUILIB_MULTI_CONFIG "${DUILIB_TEST_MULTI_CONFIG}")
    set(DUILIB_DEPS_PLATFORM "windows")
    set(DUILIB_MINGW "${DUILIB_TEST_MINGW}")
    set(DUILIB_MD "${DUILIB_TEST_MD}")
    set(DUILIB_COMPILER_GCC "${DUILIB_TEST_GCC}")
    set(DUILIB_MSYS2_ENV "${DUILIB_TEST_MSYS2_ENV}")
    set(DUILIB_MINGW_NAME "${DUILIB_TEST_MINGW_NAME}")
    set(DUILIB_CRT_ID "${DUILIB_TEST_CRT_ID}")
    set(DUILIB_SKIA_REF "test-ref")
    set(DUILIB_SKIA_REF_VALUE "test-value")
    set(DUILIB_SKIA_PATCH_ZIP "")
    set(DUILIB_SKIA_REQUIRE_PATCH OFF)
    set(DUILIB_VERSIONS_FILE "${DUILIB_TEST_REPO_ROOT}/cmake/third_party_versions.json")
    set(DUILIB_DEPS_JOBS 2)
    set(DUILIB_DEPS_LOCK_TIMEOUT 5)

    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_deps_run.cmake")

    # 与 duilib_prepare_skia 相同的配置展开规则：造出两份/一份可被直接采纳的假产物
    set(_stamp "${DUILIB_SKIA_REF}|${DUILIB_SKIA_REF_VALUE}|${DUILIB_CRT_ID}|${DUILIB_ARCH}")
    set(_args_config "${DUILIB_CONFIG}")
    if(DUILIB_MULTI_CONFIG AND DUILIB_DEPS_PLATFORM STREQUAL "windows" AND NOT DUILIB_MINGW)
        set(_args_config "Debug" "Release")
    endif()
    foreach(_cfg IN LISTS _args_config)
        set(DUILIB_CONFIG "${_cfg}")
        _duilib_skia_candidates(_candidates)
        list(GET _candidates 0 _variant)
        _duilib_skia_lib_name(_lib_name)
        file(MAKE_DIRECTORY "${DUILIB_SKIA_ROOT}/out/${_variant}")
        file(WRITE "${DUILIB_SKIA_ROOT}/out/${_variant}/${_lib_name}" "")
        file(WRITE "${DUILIB_SKIA_ROOT}/out/${_variant}/.duilib-ref" "${_stamp}")
    endforeach()

    set(DUILIB_CONFIG "${DUILIB_TEST_CONFIG}")
    duilib_prepare_skia()
    message(STATUS "RESULT:OK")

else()
    message(FATAL_ERROR "unknown DUILIB_TEST_MODE: '${DUILIB_TEST_MODE}'")
endif()
