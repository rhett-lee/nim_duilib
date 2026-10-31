# Skia / SDL3 变体规则、off 模式检查与依赖准备的 CMake 侧测试夹具
#
# 由 build/test_cmake_scripts.py 以 `cmake -D... -P` 调用，结果从 stdout 的
# "RESULT:" 行读取。模式：
#
#   DUILIB_TEST_MODE=candidates
#     验证 cmake/duilib_skia_variants.cmake 的候选目录名（与 pack.py 的打包侧比对）
#
#   DUILIB_TEST_MODE=sdl_variant
#     验证 cmake/duilib_sdl_variants.cmake 的变体前缀目录名（与 pack.py 比对）
#
#   DUILIB_TEST_MODE=sdl_include
#     验证 cmake/duilib_sdl_variants.cmake 的 SDL3 头文件目录：前缀不同（多配置
#     生成器的 Debug/Release）时按配置切换，前缀相同/只有一侧时退化为普通目录
#
#   DUILIB_TEST_MODE=deps_check
#     验证 cmake/duilib_deps.cmake 的 off 模式检查（逐配置、MinGW 只认 .a）
#
#   DUILIB_TEST_MODE=prepare_skia
#     验证 cmake/duilib_deps_run.cmake 的准备流程：多配置生成器下 Debug/Release
#     两份都要准备；单配置只准备当前配置。用假产物目录让流程直接“采纳”而不编译。
#
#   DUILIB_TEST_MODE=sdl_prefix
#     验证 SDL3 安装前缀解析：变体目录优先、<root> 作为单前缀回退，
#     以及各平台/工具链的库目录与规范库文件名。
#
#   DUILIB_TEST_MODE=deps_check_sdl
#     验证 cmake/duilib_deps.cmake 的 off 模式 SDL3 检查（逐配置、按变体前缀）。
#
#   DUILIB_TEST_MODE=deps_auto
#     验证 auto 模式下 DUILIB_SDL_AUTOBUILD 能传出函数作用域（子目录里的
#     duilib_common.cmake 依赖它跳过硬校验）。
#
#   DUILIB_TEST_MODE=sdl_ready
#     验证 cmake/duilib_deps_run.cmake 的 _duilib_sdl_ready：stamp 必须与
#     变体匹配（Release 产物不能被视为 Debug 就绪）。
#
#   DUILIB_TEST_MODE=prepare_sdl
#     验证 SDL3 多配置安装：Debug/Release 各装到自己的变体前缀，互不覆盖；
#     MinGW 的 Debug/Release 共用 release 变体。用假安装前缀让流程直接“采纳”。
#
#   DUILIB_TEST_MODE=prepare_sdl_build
#     同上，但不预置假前缀：真实构建与 git 对齐用桩替代，走“安装 → 写 stamp →
#     再次校验就绪”的路径，防止构建成功路径漏传变体参数而误报失败。
#
#   DUILIB_TEST_MODE=deps_plan
#     验证 cmake/duilib_deps_run.cmake 的计划模式（build.py deps --dry-run /
#     duilib_deps_plan）：只打印 Skia/SDL3 的命令、不写文件，并确认依赖根目录
#     没有被创建。
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

elseif("${DUILIB_TEST_MODE}" STREQUAL "sdl_variant")
    # SDL3 变体前缀目录名的唯一规则（Windows 按架构/配置/运行库区分）
    set(DUILIB_MD "${DUILIB_TEST_MD}")
    set(DUILIB_COMPILER_GCC "${DUILIB_TEST_GCC}")
    set(DUILIB_MSYS2_ENV "${DUILIB_TEST_MSYS2_ENV}")
    set(DUILIB_MINGW_NAME "${DUILIB_TEST_MINGW_NAME}")
    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_sdl_variants.cmake")
    duilib_sdl_variant_name("${DUILIB_TEST_CONFIG}" "${DUILIB_TEST_PLATFORM}"
                            "${DUILIB_TEST_ARCH}" "${DUILIB_TEST_MINGW}" _sdl_variant)
    message(STATUS "RESULT:${_sdl_variant}")

elseif("${DUILIB_TEST_MODE}" STREQUAL "sdl_include")
    # 头文件目录必须与链接的变体一一对应（多配置生成器下用生成器表达式）
    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_sdl_variants.cmake")
    duilib_sdl_include_dir("${DUILIB_TEST_SDL_DEBUG_PREFIX}"
                           "${DUILIB_TEST_SDL_RELEASE_PREFIX}" _sdl_include)
    message(STATUS "RESULT:${_sdl_include}")

elseif("${DUILIB_TEST_MODE}" STREQUAL "sdl_prefix")
    # 前缀解析：变体目录优先，<root> 作为显式单前缀的兼容回退
    set(DUILIB_MD "${DUILIB_TEST_MD}")
    set(DUILIB_COMPILER_GCC "${DUILIB_TEST_GCC}")
    set(DUILIB_MSYS2_ENV "${DUILIB_TEST_MSYS2_ENV}")
    set(DUILIB_MINGW_NAME "${DUILIB_TEST_MINGW_NAME}")
    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_sdl_variants.cmake")
    duilib_sdl_prefix_candidates("${DUILIB_TEST_CONFIG}" "${DUILIB_TEST_PLATFORM}"
                                 "${DUILIB_TEST_ARCH}" "${DUILIB_TEST_MINGW}"
                                 "${DUILIB_TEST_SDL_ROOT}" _sdl_candidates)
    set(_sdl_resolved "")
    foreach(_sdl_candidate IN LISTS _sdl_candidates)
        duilib_sdl_prefix_ready("${_sdl_candidate}" "${DUILIB_TEST_PLATFORM}"
                                "${DUILIB_TEST_MINGW}" _sdl_candidate_ok)
        if(_sdl_candidate_ok)
            set(_sdl_resolved "${_sdl_candidate}")
            break()
        endif()
    endforeach()
    if(_sdl_resolved STREQUAL "")
        set(_sdl_resolved "<none>")
    endif()
    duilib_sdl_lib_dir("${_sdl_resolved}" "${DUILIB_TEST_PLATFORM}" _sdl_lib_dir)
    duilib_sdl_static_lib_names("${DUILIB_TEST_MINGW}" _sdl_lib_names)
    list(GET _sdl_lib_names 0 _sdl_canonical)
    message(STATUS "RESULT:${_sdl_resolved}|${_sdl_lib_dir}|${_sdl_canonical}")

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

elseif("${DUILIB_TEST_MODE}" STREQUAL "deps_check_sdl")
    # off 模式的 SDL3 检查：逐配置解析 <root>/<variant>，变体缺失时可回退 <root>
    set(DUILIB_SDL_ROOT "${DUILIB_TEST_SDL_ROOT}")
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
        set(CMAKE_BUILD_TYPE "")
        set(CMAKE_CONFIGURATION_TYPES "${DUILIB_TEST_CONFIGURATION_TYPES}")
    endif()
    set(MINGW "${DUILIB_TEST_MINGW}")
    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_deps.cmake")
    _duilib_check_sdl()
    message(STATUS "RESULT:OK")

elseif("${DUILIB_TEST_MODE}" STREQUAL "deps_auto")
    # auto 模式只预判路径：DUILIB_SDL_AUTOBUILD 必须从函数作用域传出去，
    # 否则首次配置（变体尚未安装）时子目录里的 duilib_common.cmake 会误报缺少 SDL3。
    set(DUILIB_DEPS_MODE "auto")
    set(DUILIB_DEPS_ROOT "${DUILIB_TEST_DEPS_ROOT}")
    set(DUILIB_SKIA_ROOT "${DUILIB_TEST_SKIA_ROOT}")
    set(DUILIB_SDL_ROOT "${DUILIB_TEST_SDL_ROOT}")
    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_deps.cmake")
    duilib_configure_dependencies()
    if(DEFINED DUILIB_SDL_AUTOBUILD AND NOT "${DUILIB_SDL_AUTOBUILD}" STREQUAL "")
        message(STATUS "RESULT:${DUILIB_SDL_AUTOBUILD}")
    else()
        message(STATUS "RESULT:<undefined>")
    endif()

elseif("${DUILIB_TEST_MODE}" STREQUAL "sdl_ready")
    # stamp 与变体绑定：Release 前缀不能让 Debug 判为就绪。
    # 夹具自己造 <root>/<variant> 假前缀（stamp 由 _duilib_sdl_stamp_value 生成），
    # 再分别按“正确变体”“其他变体”“错误 stamp”查询就绪状态。
    set(DUILIB_DEPS_MODE "off")   # 主流程只打印提示，不进入准备逻辑
    set(DUILIB_DEPS_ROOT "${DUILIB_TEST_DEPS_ROOT}")
    set(DUILIB_SDL_ROOT "${DUILIB_TEST_SDL_ROOT}")
    set(DUILIB_SDL_SRC "${DUILIB_TEST_SDL_SRC}")
    set(DUILIB_SDL_BUILD_ROOT "${DUILIB_TEST_DEPS_ROOT}/SDL3_build")
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
    set(DUILIB_SDL_REF "test-ref")
    set(DUILIB_SDL_REF_VALUE "test-value")
    set(DUILIB_SDL_REF_TYPE "commit")
    set(DUILIB_VERSIONS_FILE "${DUILIB_TEST_REPO_ROOT}/cmake/third_party_versions.json")
    set(DUILIB_DEPS_JOBS 2)
    set(DUILIB_DEPS_LOCK_TIMEOUT 5)

    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_deps_run.cmake")

    set(_ready_prefix "${DUILIB_SDL_ROOT}/${DUILIB_TEST_READY_VARIANT}")
    file(MAKE_DIRECTORY "${_ready_prefix}/include/SDL3" "${_ready_prefix}/lib")
    if(NOT "${DUILIB_TEST_READY_HEADER}" STREQUAL "0")
        file(WRITE "${_ready_prefix}/include/SDL3/SDL.h" "")
    endif()
    if(NOT "${DUILIB_TEST_READY_LIB}" STREQUAL "0")
        duilib_sdl_static_lib_names("${DUILIB_MINGW}" _lib_names)
        list(GET _lib_names 0 _lib_name)
        file(WRITE "${_ready_prefix}/lib/${_lib_name}" "")
    endif()
    _duilib_sdl_stamp_value(_stamp "${DUILIB_TEST_READY_VARIANT}")
    if(NOT "${DUILIB_TEST_STAMP_OVERRIDE}" STREQUAL "")
        set(_stamp "${DUILIB_TEST_STAMP_OVERRIDE}")
    endif()
    file(WRITE "${_ready_prefix}/.duilib-sdl" "${_stamp}")

    _duilib_sdl_ready("${_ready_prefix}" "${DUILIB_TEST_READY_VARIANT}" _sdl_ready_self)
    _duilib_sdl_ready("${_ready_prefix}" "${DUILIB_TEST_OTHER_VARIANT}" _sdl_ready_other)
    message(STATUS "RESULT:${_sdl_ready_self};${_sdl_ready_other}")

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
    # 有编译器路径时以安装位置判定 MSYS2（与真实配置一致）；留空则回退 MSYSTEM
    set(CMAKE_CXX_COMPILER "${DUILIB_TEST_CXX_COMPILER}")
    set(CMAKE_C_COMPILER "${DUILIB_TEST_CXX_COMPILER}")
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

elseif("${DUILIB_TEST_MODE}" STREQUAL "prepare_sdl")
    set(DUILIB_DEPS_MODE "off")   # 主流程只打印提示，随后调用 duilib_prepare_sdl
    set(DUILIB_DEPS_FORCE OFF)
    set(DUILIB_DEPS_ROOT "${DUILIB_TEST_DEPS_ROOT}")
    set(DUILIB_SDL_ROOT "${DUILIB_TEST_SDL_ROOT}")
    set(DUILIB_SDL_SRC "${DUILIB_TEST_SDL_SRC}")
    set(DUILIB_SDL_BUILD_ROOT "${DUILIB_TEST_DEPS_ROOT}/SDL3_build")
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
    set(DUILIB_SDL_REF "test-ref")
    set(DUILIB_SDL_REF_VALUE "test-value")
    set(DUILIB_SDL_REF_TYPE "commit")
    set(DUILIB_VERSIONS_FILE "${DUILIB_TEST_REPO_ROOT}/cmake/third_party_versions.json")
    set(DUILIB_DEPS_JOBS 2)
    set(DUILIB_DEPS_LOCK_TIMEOUT 5)

    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_deps_run.cmake")

    # 先造“已就绪”的假安装前缀（include + 静态库 + 与变体匹配的 stamp），
    # 让 duilib_prepare_sdl 直接采纳而不触发真实编译。Debug/Release 的变体
    # 目录由共享规则给出，测试本身不复制命名规则。
    set(_sdl_variants "")
    foreach(_cfg IN ITEMS "Debug" "Release")
        _duilib_sdl_variant_for("${_cfg}" _variant)
        _duilib_sdl_prefix_for("${_cfg}" _prefix)
        _duilib_sdl_stamp_value(_stamp "${_variant}")
        file(MAKE_DIRECTORY "${_prefix}/include/SDL3" "${_prefix}/lib")
        file(WRITE "${_prefix}/include/SDL3/SDL.h" "")
        duilib_sdl_static_lib_names("${DUILIB_MINGW}" _lib_names)
        list(GET _lib_names 0 _lib_name)
        file(WRITE "${_prefix}/lib/${_lib_name}" "")
        file(WRITE "${_prefix}/.duilib-sdl" "${_stamp}")
        list(APPEND _sdl_variants "${_variant}")
    endforeach()

    duilib_prepare_sdl()
    message(STATUS "RESULT:${_sdl_variants}")

elseif("${DUILIB_TEST_MODE}" STREQUAL "prepare_sdl_build")
    # 不预置假前缀：让流程真正走到“构建 → 安装 → 写 stamp → 再次校验”。
    # 真实构建与 git 对齐用桩替代，保证测试不联网、不编译。
    set(DUILIB_DEPS_MODE "off")
    set(DUILIB_DEPS_FORCE OFF)
    set(DUILIB_DEPS_ROOT "${DUILIB_TEST_DEPS_ROOT}")
    set(DUILIB_SDL_ROOT "${DUILIB_TEST_SDL_ROOT}")
    set(DUILIB_SDL_SRC "${DUILIB_TEST_SDL_SRC}")
    set(DUILIB_SDL_BUILD_ROOT "${DUILIB_TEST_DEPS_ROOT}/SDL3_build")
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
    set(DUILIB_SDL_REF "test-ref")
    set(DUILIB_SDL_REF_VALUE "test-value")
    set(DUILIB_SDL_REF_TYPE "commit")
    set(DUILIB_VERSIONS_FILE "${DUILIB_TEST_REPO_ROOT}/cmake/third_party_versions.json")
    set(DUILIB_DEPS_JOBS 2)
    set(DUILIB_DEPS_LOCK_TIMEOUT 5)
    # 假源码目录：带 .git 以跳过 clone 分支（git 对齐由下方桩替换）
    file(MAKE_DIRECTORY "${DUILIB_SDL_SRC}/.git")

    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_deps_run.cmake")

    # 桩替换真实构建：只在调用方给出的安装前缀里造头文件与库文件
    function(_duilib_sdl_build_one build_dir config install_prefix)
        duilib_sdl_static_lib_names("${DUILIB_MINGW}" _names)
        list(GET _names 0 _name)
        duilib_sdl_lib_dir("${install_prefix}" "${DUILIB_DEPS_PLATFORM}" _lib_dir)
        file(MAKE_DIRECTORY "${install_prefix}/include/SDL3" "${_lib_dir}")
        file(WRITE "${install_prefix}/include/SDL3/SDL.h" "")
        file(WRITE "${_lib_dir}/${_name}" "")
        message(STATUS "[test] fake SDL3 install (${config}) -> ${install_prefix}")
    endfunction()

    # 桩替换 git 对齐：测试不联网、不操作真实仓库
    function(_duilib_ensure_git_ref)
        message(STATUS "[test] skip git ref alignment")
    endfunction()

    duilib_prepare_sdl()

    set(_sdl_variants "")
    foreach(_cfg IN ITEMS "Debug" "Release")
        _duilib_sdl_variant_for("${_cfg}" _variant)
        list(APPEND _sdl_variants "${_variant}")
    endforeach()
    message(STATUS "RESULT:${_sdl_variants}")

elseif("${DUILIB_TEST_MODE}" STREQUAL "deps_plan")
    # 计划模式（build.py deps --dry-run / duilib_deps_plan）：只打印 Skia/SDL3 的
    # 实际命令行，不建目录、不加锁、不克隆、不编译。命令内容由 Python 侧检查 stdout；
    # 这里只负责把两条依赖的必要参数喂进去，并确认依赖根目录没有被写入。
    set(DUILIB_DEPS_MODE "off")   # 计划模式不看 deps 模式，这里故意用 off
    set(DUILIB_DEPS_PLAN ON)
    set(DUILIB_DEPS_FORCE OFF)
    set(DUILIB_DEPS_ROOT "${DUILIB_TEST_DEPS_ROOT}")
    set(DUILIB_SKIA_ROOT "${DUILIB_TEST_SKIA_ROOT}")
    set(DUILIB_SDL_ROOT "${DUILIB_TEST_SDL_ROOT}")
    set(DUILIB_SDL_SRC "${DUILIB_TEST_DEPS_ROOT}/SDL")
    set(DUILIB_SDL_BUILD_ROOT "${DUILIB_TEST_DEPS_ROOT}/SDL3_build")
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
    set(DUILIB_SDL_REF "test-ref")
    set(DUILIB_SDL_REF_VALUE "test-value")
    set(DUILIB_SDL_REF_TYPE "commit")
    set(DUILIB_ENABLE_SDL ON)
    set(DUILIB_ENABLE_CEF OFF)
    set(DUILIB_CMAKE_COMMAND "${CMAKE_COMMAND}")
    set(DUILIB_VERSIONS_FILE "${DUILIB_TEST_REPO_ROOT}/cmake/third_party_versions.json")
    set(DUILIB_DEPS_JOBS 2)
    set(DUILIB_DEPS_LOCK_TIMEOUT 5)

    # 假的 gn/ninja 占位文件：让打印的命令优先使用 <skia>/bin 下的路径
    if(WIN32)
        set(_plan_tool_suffix ".exe")
    else()
        set(_plan_tool_suffix "")
    endif()
    file(MAKE_DIRECTORY "${DUILIB_SKIA_ROOT}/bin")
    file(WRITE "${DUILIB_SKIA_ROOT}/bin/gn${_plan_tool_suffix}" "")
    file(WRITE "${DUILIB_SKIA_ROOT}/bin/ninja${_plan_tool_suffix}" "")

    include("${DUILIB_TEST_REPO_ROOT}/cmake/duilib_deps_run.cmake")

    # 计划模式必须保持只读：依赖根目录既不该被创建，也不该出现锁文件
    if(EXISTS "${DUILIB_DEPS_ROOT}")
        file(GLOB _plan_deps_root_entries "${DUILIB_DEPS_ROOT}/*")
    else()
        set(_plan_deps_root_entries "")
    endif()
    if(_plan_deps_root_entries)
        message(STATUS "RESULT:WRITTEN")
    else()
        message(STATUS "RESULT:OK")
    endif()

else()
    message(FATAL_ERROR "unknown DUILIB_TEST_MODE: '${DUILIB_TEST_MODE}'")
endif()
