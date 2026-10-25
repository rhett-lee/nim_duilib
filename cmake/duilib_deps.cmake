# duilib 根工程：依赖准备（Skia / SDL3 / CEF）
#
# 三种模式（DUILIB_DEPS_MODE）：
#   off     —— 不自动准备；配置期检查依赖是否就绪，缺失直接报错
#   prepare —— 只创建 duilib_deps 目标（build.py deps 使用），构建该目标即完成准备
#   auto    —— 创建 duilib_deps 目标，并让 duilib/示例在编译前先完成依赖准备
#
# 真正的准备逻辑在 duilib_deps_run.cmake（脚本模式），保证 CMake 与 build.py
# 走同一条路径。

include("${CMAKE_CURRENT_LIST_DIR}/duilib_i18n.cmake")
# 函数体内不能依赖 CMAKE_CURRENT_LIST_DIR（那是调用者的目录），先固化本文件所在目录
set(DUILIB_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")

# 目标配置（多配置生成器在 P2 支持；当前按 Release 处理依赖）
function(_duilib_deps_config out_var)
    if(CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
        set(${out_var} "${CMAKE_BUILD_TYPE}" PARENT_SCOPE)
    else()
        set(${out_var} "Release" PARENT_SCOPE)
    endif()
endfunction()

# 构造传给脚本模式的 -D 参数列表
function(_duilib_deps_script_args out_var)
    _duilib_deps_config(_duilib_deps_cfg)
    set(_args
        "-DDUILIB_LANG=${DUILIB_I18N_LANG}"
        "-DDUILIB_DEPS_MODE=${DUILIB_DEPS_MODE}"
        "-DDUILIB_DEPS_ROOT=${DUILIB_DEPS_ROOT}"
        "-DDUILIB_DEPS_FORCE=${DUILIB_DEPS_FORCE}"
        "-DDUILIB_DEPS_PLATFORM=${DUILIB_DEPS_PLATFORM}"
        "-DDUILIB_ARCH=${DUILIB_TARGET_ARCH_RESOLVED}"
        "-DDUILIB_CONFIG=${_duilib_deps_cfg}"
        "-DDUILIB_BUILD_TYPE=${DUILIB_BUILD_TYPE}"
        "-DDUILIB_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}"
        "-DDUILIB_COMPILER_NAME=${DUILIB_COMPILER_NAME}"
        "-DDUILIB_COMPILER_GCC=${DUILIB_COMPILER_GCC}"
        "-DDUILIB_MINGW=${MINGW}"
        "-DDUILIB_MSYS2_ENV=${DUILIB_MSYS2_ENV}"
        "-DDUILIB_MINGW_NAME=${DUILIB_MINGW_NAME}"
        "-DDUILIB_MD=${DUILIB_MD}"
        "-DDUILIB_CRT_ID=${DUILIB_CRT_ID}"
        "-DDUILIB_ENABLE_SDL=${DUILIB_ENABLE_SDL}"
        "-DDUILIB_ENABLE_CEF=${DUILIB_ENABLE_CEF}"
        "-DDUILIB_CEF_VERSION=${DUILIB_CEF_VERSION}"
        "-DDUILIB_CEF_REQUIRED=${DUILIB_CEF_REQUIRED}"
        "-DDUILIB_VERSIONS_FILE=${DUILIB_VERSIONS_FILE}"
        "-DDUILIB_SKIA_REF=${DUILIB_SKIA_REF}"
        "-DDUILIB_SKIA_REF_TYPE=${DUILIB_SKIA_REF_TYPE}"
        "-DDUILIB_SKIA_REF_VALUE=${DUILIB_SKIA_REF_VALUE}"
        "-DDUILIB_SKIA_PATCH_ZIP=${DUILIB_SKIA_PATCH_ZIP}"
        "-DDUILIB_SKIA_REQUIRE_PATCH=${DUILIB_SKIA_REQUIRE_PATCH}"
        "-DDUILIB_SKIA_Root=${DUILIB_SKIA_ROOT}"
        "-DDUILIB_SKIA_ROOT=${DUILIB_SKIA_ROOT}"
        "-DDUILIB_SKIA_COMPILE_ROOT=${DUILIB_SKIA_COMPILE_ROOT}"
        "-DDUILIB_SDL_REF=${DUILIB_SDL_REF}"
        "-DDUILIB_SDL_REF_VALUE=${DUILIB_SDL_REF_VALUE}"
        "-DDUILIB_SDL_SRC=${DUILIB_SDL_SRC}"
        "-DDUILIB_SDL_ROOT=${DUILIB_SDL_ROOT}"
        "-DDUILIB_SDL_BUILD_ROOT=${DUILIB_SDL_BUILD_ROOT}"
        "-DDUILIB_CEF_ROOT=${DUILIB_CEF_ROOT}"
        "-DDUILIB_CEF_CACHE=${DUILIB_CEF_CACHE}"
        "-DDUILIB_CEF_URL_BASE=${DUILIB_CEF_URL_BASE}"
        "-DDUILIB_CEF_ARCHIVE=${DUILIB_CEF_ARCHIVE}"
        "-DDUILIB_CEF_109=${DUILIB_CEF_VERSION}" # compatibility marker
        "-DDUILIB_OFFLINE=${DUILIB_OFFLINE}"
        "-DDUILIB_DEPS_AUTO_CLONE=${DUILIB_DEPS_AUTO_CLONE}"
        "-DDUILIB_DEPS_JOBS=${DUILIB_DEPS_JOBS}"
        "-DDUILIB_DOWNLOAD_RETRIES=${DUILIB_DOWNLOAD_RETRIES}"
        "-DDUILIB_DOWNLOAD_RETRY_DELAY=${DUILIB_DOWNLOAD_RETRY_DELAY}"
        "-DDUILIB_DOWNLOAD_CONNECT_TIMEOUT=${DUILIB_DOWNLOAD_CONNECT_TIMEOUT}"
        "-DDUILIB_DEPS_LOCK_TIMEOUT=${DUILIB_DEPS_LOCK_TIMEOUT}"
        "-DDUILIB_LLVM_ROOT=${DUILIB_LLVM_ROOT}"
        "-DDUILIB_GN_EXECUTABLE=${DUILIB_GN_EXECUTABLE}"
        "-DDUILIB_NINJA_EXECUTABLE=${DUILIB_NINJA_EXECUTABLE}"
        "-DDUILIB_ROOT=${DUILIB_ROOT}"
        "-DDUILIB_GENERATOR=${CMAKE_GENERATOR}"
        "-DDUILIB_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
        "-DDUILIB_C_COMPILER=${CMAKE_C_COMPILER}"
        "-DDUILIB_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
        "-DDUILIB_CMAKE_COMMAND=${CMAKE_COMMAND}"
    )
    set(${out_var} "${_args}" PARENT_SCOPE)
endfunction()

function(duilib_add_dependency_targets)
    _duilib_deps_script_args(_duilib_deps_args)
    add_custom_target(duilib_deps
        COMMAND "${CMAKE_COMMAND}" ${_duilib_deps_args}
                -P "${DUILIB_CMAKE_DIR}/duilib_deps_run.cmake"
        USES_TERMINAL
        COMMENT "Preparing duilib dependencies (Skia/SDL3/CEF)")
endfunction()

# 配置期依赖检查（off 模式）：只检查，不准备
function(_duilib_check_dependency_exists what path hint)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "[ERROR] ${what} not found:\n  ${path}\n  Hint: ${hint}")
    endif()
endfunction()

function(duilib_configure_dependencies)
    if(DUILIB_DEPS_MODE STREQUAL "prepare")
        # 由根 CMakeLists 调用 duilib_add_dependency_targets 后直接返回
        return()
    endif()

    if(DUILIB_DEPS_MODE STREQUAL "auto")
        set(DUILIB_SKIA_AUTOBUILD ON CACHE BOOL "Skip the Skia existence check (deps builds it first)" FORCE)
        duilib_msg("deps.done" " (auto: prepared before compiling)")
        return()
    endif()

    # off：逐个检查必需依赖
    if(NOT EXISTS "${DUILIB_SKIA_ROOT}")
        duilib_error("deps.skia_source_missing" "deps.skia_source_missing_hint")
    endif()

    # Skia 库目录按配置检查（与 duilib_common.cmake 的规则一致）
    set(_duilib_skia_candidates "")
    if(DUILIB_OS_WINDOWS)
        if(NOT MINGW)
            if(DUILIB_BUILD_TYPE STREQUAL "debug")
                list(APPEND _duilib_skia_candidates "llvm.${DUILIB_TARGET_ARCH_RESOLVED}.debug")
            else()
                list(APPEND _duilib_skia_candidates "llvm.${DUILIB_TARGET_ARCH_RESOLVED}.release")
            endif()
        else()
            if(DUILIB_COMPILER_GCC)
                set(_duilib_skia_compilers "gcc")
                if(DUILIB_MSYS2_ENV STREQUAL "mingw64")
                    list(APPEND _duilib_skia_candidates
                        "gcc.msvcrt.${DUILIB_TARGET_ARCH_RESOLVED}.release")
                endif()
            else()
                set(_duilib_skia_compilers "llvm")
            endif()
            if(DUILIB_MINGW_NAME STREQUAL "")
                set(_duilib_skia_prefixes "" "mingw64-")
            else()
                set(_duilib_skia_prefixes "mingw64-" "")
            endif()
            foreach(_prefix IN LISTS _duilib_skia_prefixes)
                foreach(_compiler IN LISTS _duilib_skia_compilers)
                    if(DUILIB_BUILD_TYPE STREQUAL "debug")
                        list(APPEND _duilib_skia_candidates
                            "${_prefix}${_compiler}.${DUILIB_TARGET_ARCH_RESOLVED}.debug"
                            "${_prefix}${_compiler}.${DUILIB_TARGET_ARCH_RESOLVED}.release")
                    else()
                        list(APPEND _duilib_skia_candidates
                            "${_prefix}${_compiler}.${DUILIB_TARGET_ARCH_RESOLVED}.release")
                    endif()
                endforeach()
            endforeach()
        endif()
    else()
        list(APPEND _duilib_skia_candidates
            "${DUILIB_COMPILER_NAME}.${DUILIB_TARGET_ARCH_RESOLVED}.release")
    endif()

    set(_duilib_skia_found FALSE)
    foreach(_candidate IN LISTS _duilib_skia_candidates)
        if(EXISTS "${DUILIB_SKIA_ROOT}/out/${_candidate}")
            set(_duilib_skia_found TRUE)
            break()
        endif()
    endforeach()
    if(NOT _duilib_skia_found)
        message(FATAL_ERROR
            "[ERROR] no usable Skia library directory in ${DUILIB_SKIA_ROOT}/out\n"
            "  Tried: ${_duilib_skia_candidates}\n"
            "  Hint: run 'build.py deps', or pass -DDUILIB_DEPS_MODE=auto")
    endif()

    if(DUILIB_ENABLE_SDL AND NOT EXISTS "${DUILIB_SDL_LIB_DIR}")
        duilib_error("deps.sdl_missing" "deps.sdl_missing_hint")
    endif()

    if(DUILIB_ENABLE_CEF)
        if(DUILIB_OS_WINDOWS)
            if(DUILIB_TARGET_ARCH_RESOLVED STREQUAL "arm64")
                set(_duilib_cef_arch "arm64")
            elseif(DUILIB_TARGET_ARCH_RESOLVED STREQUAL "x86")
                set(_duilib_cef_arch "Win32")
            else()
                set(_duilib_cef_arch "x64")
            endif()
            set(_duilib_cef_suffix "")
            if(DUILIB_CEF_VERSION STREQUAL "109")
                set(_duilib_cef_suffix "_109")
            endif()
            set(_duilib_cef_runtime
                "${DUILIB_ROOT}/bin/libcef_win${_duilib_cef_suffix}/${_duilib_cef_arch}/libcef.dll")
        elseif(DUILIB_OS_LINUX)
            set(_duilib_cef_runtime "${DUILIB_ROOT}/bin/libcef_linux/libcef.so")
        else()
            set(_duilib_cef_runtime "")
        endif()
        if(NOT _duilib_cef_runtime STREQUAL "" AND NOT EXISTS "${_duilib_cef_runtime}")
            message(FATAL_ERROR
                "[ERROR] CEF runtime not found:\n  ${_duilib_cef_runtime}\n"
                "  Hint: run 'build.py deps', or pass -DDUILIB_DEPS_MODE=auto")
        endif()
    endif()
endfunction()

# 在 all targets 创建后调用：auto 模式下把依赖准备放到编译之前
function(duilib_link_dependency_targets)
    if(NOT TARGET duilib_deps)
        return()
    endif()
    if(TARGET duilib)
        add_dependencies(duilib duilib_deps)
    endif()
    foreach(_duilib_example IN LISTS DUILIB_EXAMPLES)
        if(TARGET ${_duilib_example})
            add_dependencies(${_duilib_example} duilib_deps)
        endif()
    endforeach()
    if(TARGET libcef_dll_wrapper)
        add_dependencies(libcef_dll_wrapper duilib_deps)
    endif()
endfunction()
