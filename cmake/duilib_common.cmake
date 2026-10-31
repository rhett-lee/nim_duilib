# 操作系统：Windows,Linux,MacOS
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(DUILIB_OS_WINDOWS 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DUILIB_OS_LINUX 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DUILIB_OS_MACOS 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(DUILIB_OS_FREEBSD 1)
    message(STATUS "Building for FreeBSD")
else()
    message(WARNING "Unknown OS: ${CMAKE_SYSTEM_NAME}")
endif()

# 定义开关变量（修改后，需要清除camke生成目录才能生效，如果未清除则会从缓存中读取旧值）
option(DUILIB_LOG "Print duilib debug log" OFF)

# MingGW-w64编译时，使用静态链接（仅在 MINGW 开发环境下生效）
option(DUILIB_MINGW_STATIC "MINGW Static Link" ON)

# Skia的lib子目录名：默认值在下方按平台/工具链自动推导，不写缓存；可用 -DDUILIB_SKIA_LIB_SUBPATH 覆盖

# 判断 MinGW 工具链是否来自 MSYS2，并返回环境名（决定 Skia 子目录前缀）：
#   MSYS2 的编译器位于 <root>/<env>/bin（env = mingw64/ucrt64/clang64/clangarm64），
#   安装根目录下存在 usr/bin/pacman.exe；独立 MinGW-w64 不满足该特征。
# 不能用 MSYSTEM 环境变量判断：Git Bash 同样会设置 MSYSTEM=MINGW64。
# 输出为空表示不是 MSYS2；否则输出 mingw64 / ucrt64 / clang64 / clangarm64。
function(_duilib_detect_msys2 out_var)
    set(_duilib_env_name "")
    set(_duilib_cc "${CMAKE_CXX_COMPILER}")
    if(_duilib_cc STREQUAL "")
        set(_duilib_cc "${CMAKE_C_COMPILER}")
    endif()
    if(NOT _duilib_cc STREQUAL "")
        get_filename_component(_duilib_cc_dir "${_duilib_cc}" DIRECTORY)
        get_filename_component(_duilib_env_dir "${_duilib_cc_dir}" DIRECTORY)
        get_filename_component(_duilib_msys_root "${_duilib_env_dir}" DIRECTORY)
        if(EXISTS "${_duilib_msys_root}/usr/bin/pacman.exe")
            get_filename_component(_duilib_env_name "${_duilib_env_dir}" NAME)
        endif()
    endif()
    set(${out_var} "${_duilib_env_name}" PARENT_SCOPE)
endfunction()

# MINGW 开发环境
set(DUILIB_MSYS2_ENV "")
if(MINGW)
    set(DUILIB_MINGW 1)
    # Skia 目录前缀：MSYS2 无前缀（gcc./llvm.），独立 MinGW-w64 为 mingw64-
    _duilib_detect_msys2(_duilib_msys2_env)
    set(DUILIB_MSYS2_ENV "${_duilib_msys2_env}")
    if(_duilib_msys2_env)
        set(DUILIB_MINGW_NAME "")
    else()
        set(DUILIB_MINGW_NAME "mingw64-")
    endif()
    unset(_duilib_msys2_env)
endif()

# SDL功能：Windows平台默认不开启，其他平台默认开启
if(DUILIB_OS_WINDOWS)
    option(DUILIB_ENABLE_SDL "Enable SDL" OFF)
else()
    option(DUILIB_ENABLE_SDL "Enable SDL" ON)
endif()

# CEF功能：默认不开启，只有特定的项目开启
option(DUILIB_ENABLE_CEF "Enable CEF" OFF)

# CEF 109版本是否开启，默认不开启（CEF109版本兼容Win7系统，其他CEF版本只能运行在Win10及高版本操作系统）
option(DUILIB_CEF_109 "Enable CEF 109" OFF)

# WebView2控件的二进制（仅Windows平台有效，其他平台忽略该定义）
option(DUILIB_WEBVIEW2_EXE "Is Windows WebView2 exe" OFF)

#编译器类型: msvc,gcc,llvm
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(DUILIB_COMPILER_MSVC 1)
    set(DUILIB_COMPILER_NAME "msvc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(DUILIB_COMPILER_GCC 1)
    set(DUILIB_COMPILER_NAME "${DUILIB_MINGW_NAME}gcc")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(DUILIB_COMPILER_LLVM 1)
    set(DUILIB_COMPILER_NAME "${DUILIB_MINGW_NAME}llvm")
else()
    set(DUILIB_COMPILER_NAME "unknown")
    message(WARNING "Unknown CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
endif() 

# CPU类型
# Linux, MacOS: 只支持64位，没必要支持32位
# Windows：需要支持64位和32位，因32位系统仍有部分用户
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" lower_processor)
# 根工程已解析出目标架构时直接采用，避免 Ninja 交叉编译时被宿主处理器误导
if(NOT DUILIB_SYSTEM_PROCESSOR)
    if(lower_processor MATCHES "armv7|arm")
        set(DUILIB_SYSTEM_PROCESSOR "arm32")
    elseif(lower_processor MATCHES "aarch64|arm64")
        set(DUILIB_SYSTEM_PROCESSOR "arm64")
    elseif(lower_processor MATCHES "i[3-6]86")
        set(DUILIB_SYSTEM_PROCESSOR "x86")
    elseif(lower_processor MATCHES "x86_64|amd64")
        set(DUILIB_SYSTEM_PROCESSOR "x64")
    else()
        set(DUILIB_SYSTEM_PROCESSOR "unknown")
        message(WARNING "Unknown CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

#64位还是32位
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DUILIB_BITS_64 1)
endif()

#编译类型：Debug还是Release
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DUILIB_BUILD_TYPE "debug")
else()
    set(DUILIB_BUILD_TYPE "release")
endif()

#Skia的lib子目录名：默认由自动规则推导（P2 / I-1），不再声明为 CACHE 变量，
# 避免缓存值在切换工具链/架构后静默沿用旧路径。
# 仍可用 -DDUILIB_SKIA_LIB_SUBPATH=... 覆盖（固定路径，不随配置变化）。
# 自动规则：
#   Windows MSVC / clang-cl -> llvm.<arch>.debug|release
#     Debug 必须匹配 debug 变体（Debug 版本无法链接 release Skia），缺失直接报错。
#   Windows MinGW gcc       -> mingw64-gcc.<arch>.debug|release（MSYS2 为 gcc.<arch>.xxx）
#     Debug 变体缺失时回退同编译器的 release 变体（MinGW 的 Debug 可以链接 release Skia）；
#     gcc 与 llvm 的 Skia 不能混用：libstdc++ 与 libc++ 会重复定义 std 符号，链接失败
#   Windows MinGW llvm      -> mingw64-llvm.<arch>.debug|release（MSYS2 为 llvm.<arch>.xxx）
#     Debug 变体缺失时回退 release 变体
#   其他平台                -> <编译器名>.<arch>.release（Skia 只有 release 变体，见 D-3）
# MSYS2 与独立 MinGW-w64 由编译器安装位置判断（见 _duilib_detect_msys2），
# 本工具链的前缀优先，另一组前缀作为回退。
# 解析结果必须真实存在，且 MinGW 构建要求目录内存在 *.a 静态库；
# 否则在配置阶段直接报错，避免进入编译后才出现难以定位的链接错误。
if(NOT DUILIB_SKIA_CPU_ARCH)
    if(DUILIB_OS_WINDOWS)
        if(CMAKE_GENERATOR_PLATFORM)
            if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
                set(DUILIB_SKIA_CPU_ARCH "x86")
            else()
                string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" DUILIB_SKIA_CPU_ARCH)
            endif()
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
            # Ninja 等生成器没有 generator platform，ARM64 目标仅按指针宽度会被
            # 误判为 x64，这里先看目标处理器。
            set(DUILIB_SKIA_CPU_ARCH "arm64")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            # MinGW 32 位没有 generator platform，按指针宽度判断
            set(DUILIB_SKIA_CPU_ARCH "x86")
        else()
            set(DUILIB_SKIA_CPU_ARCH "x64")
        endif()
    endif()
endif()

#dulib源码根目录，lib文件目录，bin文件目录
get_filename_component(DUILIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
# 库输出目录规则（D-2：<root>/lib/<arch>/<Config> 或 <root>/lib/<Config>）
include("${DUILIB_ROOT}/cmake/duilib_lib_output.cmake")
set(DUILIB_LIB_PATH "${DUILIB_LIB_BASE}/$<CONFIG>")
set(DUILIB_BIN_PATH "${DUILIB_ROOT}/bin")
set(DUILIB_LIBS duilib duilib-cximage duilib-webp duilib-png duilib-zlib)

#CEF模块的源码根目录（CEF模块为可选项）
# CEF/SDL 的库名列表先给出默认空值，开启对应开关后再填充，
# 避免在链接处引用未定义的变量（开关关闭时为空展开）
set(DUILIB_CEF_LIBS "")
set(DUILIB_SDL_LIBS "")
if(DUILIB_ENABLE_CEF)
    if(DUILIB_OS_WINDOWS)
        # Windows 平台：运行时目录按架构区分（x86 -> Win32）
        if(DUILIB_TARGET_ARCH STREQUAL "arm64")
            set(_duilib_cef_arch "arm64")
        elseif(DUILIB_BITS_64)
            set(_duilib_cef_arch "x64")
        else()
            set(_duilib_cef_arch "Win32")
        endif()
        if(DUILIB_CEF_VERSION STREQUAL "109")
            #使用CEF 109版本
            set(DUILIB_CEF_SRC_ROOT_DIR "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_win_109")
            set(DUILIB_CEF_LIB_PATH "${DUILIB_CEF_SRC_ROOT_DIR}/lib/${_duilib_cef_arch}")
            set(DUILIB_CEF_WRAPPER_LIB_NAME libcef_dll_wrapper_109)
        else()
            #使用CEF最新版本
            set(DUILIB_CEF_SRC_ROOT_DIR "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_win")
            set(DUILIB_CEF_LIB_PATH "${DUILIB_CEF_SRC_ROOT_DIR}/lib/${_duilib_cef_arch}")
            set(DUILIB_CEF_WRAPPER_LIB_NAME libcef_dll_wrapper)
        endif()
    elseif(DUILIB_OS_LINUX)
        # Linux平台
        set(DUILIB_CEF_SRC_ROOT_DIR "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_linux")
        set(DUILIB_CEF_LIB_PATH "${DUILIB_ROOT}/bin/libcef_linux")
        set(DUILIB_CEF_WRAPPER_LIB_NAME cef_dll_wrapper)
    elseif(DUILIB_OS_MACOS)
        # MacOS平台
        set(DUILIB_CEF_SRC_ROOT_DIR "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_macos")
        set(DUILIB_CEF_LIB_PATH "")
        set(DUILIB_CEF_WRAPPER_LIB_NAME cef_dll_wrapper)
    endif()
    if(DUILIB_OS_WINDOWS)
        set(DUILIB_CEF_LIBS libcef ${DUILIB_CEF_WRAPPER_LIB_NAME})
    elseif(DUILIB_OS_MACOS)
        set(DUILIB_CEF_LIBS ${DUILIB_CEF_WRAPPER_LIB_NAME})
    else()
        # 与上游 156 的 CEF_STANDARD_LIBS（X11 + ${CMAKE_DL_LIBS}）保持一致
        set(DUILIB_CEF_LIBS libcef.so ${DUILIB_CEF_WRAPPER_LIB_NAME} X11 ${CMAKE_DL_LIBS})
    endif()
endif()

#Skia源码根目录，lib文件目录（Skia必选项）
if(DUILIB_SKIA_ROOT)
    get_filename_component(DUILIB_SKIA_SRC_ROOT_DIR "${DUILIB_SKIA_ROOT}" ABSOLUTE)
else()
    get_filename_component(DUILIB_SKIA_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../skia/" ABSOLUTE)
endif()

# 当前工具链的 CRT 标识（与 cmake/duilib_root_options.cmake 的规则保持一致）
function(_duilib_expected_crt out_var)
    if(MSVC)
        if(DUILIB_MD)
            set(_crt "msvc-md")
        else()
            set(_crt "msvc-mt")
        endif()
    elseif(MINGW)
        if(DEFINED ENV{MSYSTEM} AND "$ENV{MSYSTEM}" STREQUAL "MINGW64")
            set(_crt "mingw64-msvcrt")
        elseif(DEFINED ENV{MSYSTEM} AND NOT "$ENV{MSYSTEM}" STREQUAL "")
            string(TOLOWER "$ENV{MSYSTEM}" _msys)
            set(_crt "${_msys}")
        else()
            set(_crt "mingw-unknown")
        endif()
    else()
        set(_crt "system")
    endif()
    set(${out_var} "${_crt}" PARENT_SCOPE)
endfunction()

# 依赖目录的 CRT 标记比对：没有标记的旧产物允许迁移，有标记则必须一致
function(_duilib_check_deps_crt what dir)
    if("${dir}" STREQUAL "" OR "${dir}" MATCHES "\\$<")
        return()
    endif()
    if(NOT EXISTS "${dir}/.duilib-crt")
        return()
    endif()
    file(READ "${dir}/.duilib-crt" _stamp_crt)
    string(STRIP "${_stamp_crt}" _stamp_crt)
    _duilib_expected_crt(_expected_crt)
    if(NOT _stamp_crt STREQUAL _expected_crt)
        message(FATAL_ERROR
            "[ERROR] CRT mismatch (${what}):\n"
            "  ${dir}\n"
            "  已构建: ${_stamp_crt}\n"
            "  当前:   ${_expected_crt}\n"
            "Hint: 用相同的 DUILIB_MD 重新执行 'python3 build/build.py deps'，"
            "或改用与依赖匹配的构建目录。")
    endif()
endfunction()

include("${CMAKE_CURRENT_LIST_DIR}/duilib_skia_toolchain.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/duilib_skia_variants.cmake")

# 生成指定配置下的 Skia 子目录候选（按优先级排列）
# 规则实现在 duilib_skia_variants.cmake，与 deps 的 off 模式检查和依赖准备共用；
# 这里只负责把本工程作用域的变量名映射成共享函数的参数。
function(_duilib_skia_candidate_subdirs config out_var)
    if(DUILIB_OS_WINDOWS)
        set(_duilib_skia_platform "windows")
        set(_duilib_skia_arch "${DUILIB_SKIA_CPU_ARCH}")
    else()
        set(_duilib_skia_platform "other")
        set(_duilib_skia_arch "${DUILIB_SYSTEM_PROCESSOR}")
    endif()
    duilib_skia_variant_candidates("${config}" "${_duilib_skia_platform}"
                                   "${_duilib_skia_arch}" "${DUILIB_MINGW}" _candidates)
    set(${out_var} "${_candidates}" PARENT_SCOPE)
endfunction()

# MinGW 构建要求 Skia 目录中存在 .a 静态库：
# llvm.<arch>.release 这类目录名与 MSVC 产物相同，若只检查目录存在，
# 会把 MSVC 的 .lib 误判为可用库，直到链接阶段才报错。
function(_duilib_check_mingw_skia_lib dir)
    if(NOT DUILIB_MINGW)
        return()
    endif()
    file(GLOB _duilib_skia_a_files "${dir}/*.a")
    if(NOT _duilib_skia_a_files)
        message(FATAL_ERROR
            "Skia 库目录中没有 .a 静态库，无法用于 MinGW 构建：\n"
            "  ${dir}\n"
            "该目录可能存放的是 MSVC 构建的 .lib 文件。\n"
            "请用与当前工具链相同的编译器重新编译 Skia（MSYS2 用 gcc 或 CLANG64 的 clang，"
            "独立 MinGW 用 gcc 或 llvm-mingw），"
            "或用 -DDUILIB_SKIA_LIB_SUBPATH=<子目录名> 指定正确的目录。")
    endif()
endfunction()

# 外部显式指定的子目录名（固定路径，不随配置变化）
set(_duilib_skia_user_subpath "")
if(DEFINED DUILIB_SKIA_LIB_SUBPATH AND
   NOT DUILIB_SKIA_LIB_SUBPATH STREQUAL "" AND
   NOT DUILIB_SKIA_LIB_SUBPATH STREQUAL "OFF")
    set(_duilib_skia_user_subpath "${DUILIB_SKIA_LIB_SUBPATH}")
endif()

# 需要校验的配置（小写，去重）：多配置生成器逐个配置校验，单配置只看 CMAKE_BUILD_TYPE
set(_duilib_skia_configs "")
if(CMAKE_CONFIGURATION_TYPES)
    foreach(_duilib_cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOLOWER "${_duilib_cfg}" _duilib_cfg_lower)
        if(_duilib_cfg_lower STREQUAL "debug")
            list(APPEND _duilib_skia_configs "debug")
        else()
            list(APPEND _duilib_skia_configs "release")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES _duilib_skia_configs)
else()
    set(_duilib_skia_configs "${DUILIB_BUILD_TYPE}")
endif()

set(DUILIB_SKIA_LIB_SUBPATH "")
set(DUILIB_SKIA_LIB_PATH "")
if(DUILIB_SKIA_AUTOBUILD AND NOT _duilib_skia_user_subpath)
    # 依赖自动准备模式：Skia 由 duilib_deps 在编译前构建，这里不校验存在性，
    # 只按下述规则确定生成目录名（与 duilib_deps_run.cmake 的选择保持一致）。
    if(CMAKE_CONFIGURATION_TYPES)
        _duilib_skia_candidate_subdirs("debug" _duilib_skia_debug_candidates)
        _duilib_skia_candidate_subdirs("release" _duilib_skia_release_candidates)
        list(GET _duilib_skia_debug_candidates 0 _duilib_skia_debug_sub)
        list(GET _duilib_skia_release_candidates 0 _duilib_skia_release_sub)
        if(_duilib_skia_debug_sub STREQUAL _duilib_skia_release_sub)
            set(DUILIB_SKIA_LIB_SUBPATH "${_duilib_skia_release_sub}")
        else()
            set(DUILIB_SKIA_LIB_SUBPATH
                "$<IF:$<CONFIG:Debug>,${_duilib_skia_debug_sub},${_duilib_skia_release_sub}>")
        endif()
        set(DUILIB_SKIA_LIB_PATH "${DUILIB_SKIA_SRC_ROOT_DIR}/out/${DUILIB_SKIA_LIB_SUBPATH}")
    else()
        set(_duilib_skia_auto_config "${DUILIB_BUILD_TYPE}")
        if(NOT _duilib_skia_auto_config MATCHES "^(debug|release)$")
            set(_duilib_skia_auto_config "release")
        endif()
        _duilib_skia_candidate_subdirs("${_duilib_skia_auto_config}" _duilib_skia_auto_candidates)
        list(GET _duilib_skia_auto_candidates 0 DUILIB_SKIA_LIB_SUBPATH)
        set(DUILIB_SKIA_LIB_PATH "${DUILIB_SKIA_SRC_ROOT_DIR}/out/${DUILIB_SKIA_LIB_SUBPATH}")
    endif()
elseif(_duilib_skia_user_subpath)
    set(DUILIB_SKIA_LIB_SUBPATH "${_duilib_skia_user_subpath}")
    set(DUILIB_SKIA_LIB_PATH "${DUILIB_SKIA_SRC_ROOT_DIR}/out/${_duilib_skia_user_subpath}")
    if(DUILIB_SKIA_LIB_PATH MATCHES "\\$<")
        # 含生成器表达式时无法在配置阶段确定最终路径，交由生成器展开
        message(STATUS "DUILIB_SKIA_LIB_SUBPATH 含生成器表达式，跳过配置期存在性校验：${DUILIB_SKIA_LIB_SUBPATH}")
    elseif(NOT EXISTS "${DUILIB_SKIA_LIB_PATH}")
        message(FATAL_ERROR
            "指定的 Skia 库目录不存在：\n"
            "  ${DUILIB_SKIA_LIB_PATH}\n"
            "请检查 -DDUILIB_SKIA_LIB_SUBPATH=${DUILIB_SKIA_LIB_SUBPATH} 是否正确，"
            "或先编译 Skia。")
    else()
        _duilib_check_mingw_skia_lib("${DUILIB_SKIA_LIB_PATH}")
    endif()
else()
    # 按配置解析候选目录，取第一个真实存在的候选
    set(_duilib_skia_debug_sub "")
    set(_duilib_skia_release_sub "")
    set(_duilib_skia_error "")
    foreach(_cfg IN LISTS _duilib_skia_configs)
        _duilib_skia_candidate_subdirs("${_cfg}" _candidates)
        set(_resolved "")
        foreach(_candidate IN LISTS _candidates)
            set(_candidate_path "${DUILIB_SKIA_SRC_ROOT_DIR}/out/${_candidate}")
            if(EXISTS "${_candidate_path}")
                # MinGW 的目录名可能与 MSVC 产物重名（如 llvm.x64.debug 存放的是 .lib），
                # 只有包含 .a 静态库的目录才能用于 MinGW，否则继续尝试后续候选
                # （例如 MSYS2 CLANG64 回退到 mingw64-llvm.<arch>.release）
                set(_candidate_ok ON)
                if(DUILIB_MINGW)
                    file(GLOB _candidate_a_files "${_candidate_path}/*.a")
                    if(NOT _candidate_a_files)
                        set(_candidate_ok OFF)
                    endif()
                endif()
                if(_candidate_ok)
                    set(_resolved "${_candidate}")
                    break()
                endif()
            endif()
        endforeach()
        if(_resolved STREQUAL "")
            string(REPLACE ";" "\n    " _candidates_text "${_candidates}")
            string(APPEND _duilib_skia_error
                "  ${_cfg} 配置：未找到可用的 Skia 库目录，已尝试：\n    ${_candidates_text}\n")
        elseif(_cfg STREQUAL "debug")
            set(_duilib_skia_debug_sub "${_resolved}")
        else()
            set(_duilib_skia_release_sub "${_resolved}")
        endif()
    endforeach()

    if(NOT _duilib_skia_error STREQUAL "")
        message(FATAL_ERROR
            "未找到可用的 Skia 库目录（${DUILIB_SKIA_SRC_ROOT_DIR}/out 下）：\n"
            "${_duilib_skia_error}"
            "请先编译 Skia，或用 -DDUILIB_SKIA_LIB_SUBPATH=<子目录名> 指定。")
    endif()

    # MinGW 构建逐个配置校验目录内的 .a 静态库（多配置下两种配置可能指向不同目录）
    if(DUILIB_MINGW)
        if(NOT _duilib_skia_debug_sub STREQUAL "")
            _duilib_check_mingw_skia_lib("${DUILIB_SKIA_SRC_ROOT_DIR}/out/${_duilib_skia_debug_sub}")
        endif()
        if(NOT _duilib_skia_release_sub STREQUAL "")
            _duilib_check_mingw_skia_lib("${DUILIB_SKIA_SRC_ROOT_DIR}/out/${_duilib_skia_release_sub}")
        endif()
    endif()

    if(CMAKE_CONFIGURATION_TYPES)
        # 多配置生成器：不同配置可能解析到不同目录，用生成器表达式按 $<CONFIG> 选择
        if(NOT _duilib_skia_debug_sub STREQUAL "" AND
           NOT _duilib_skia_release_sub STREQUAL "" AND
           NOT _duilib_skia_debug_sub STREQUAL _duilib_skia_release_sub)
            set(DUILIB_SKIA_LIB_SUBPATH
                "$<IF:$<CONFIG:Debug>,${_duilib_skia_debug_sub},${_duilib_skia_release_sub}>")
        elseif(NOT _duilib_skia_debug_sub STREQUAL "")
            set(DUILIB_SKIA_LIB_SUBPATH "${_duilib_skia_debug_sub}")
        else()
            set(DUILIB_SKIA_LIB_SUBPATH "${_duilib_skia_release_sub}")
        endif()
    else()
        if(DUILIB_BUILD_TYPE STREQUAL "debug")
            set(DUILIB_SKIA_LIB_SUBPATH "${_duilib_skia_debug_sub}")
        else()
            set(DUILIB_SKIA_LIB_SUBPATH "${_duilib_skia_release_sub}")
        endif()
    endif()
    set(DUILIB_SKIA_LIB_PATH "${DUILIB_SKIA_SRC_ROOT_DIR}/out/${DUILIB_SKIA_LIB_SUBPATH}")
endif()
unset(_duilib_skia_user_subpath)
unset(_duilib_skia_configs)
unset(_duilib_cfg_lower)
unset(_duilib_skia_debug_sub)
unset(_duilib_skia_release_sub)
unset(_duilib_skia_error)
unset(_candidates)
unset(_candidates_text)
unset(_resolved)
unset(_cfg)
# 把库名解析成绝对路径（用于各平台的静态库）。
# Ninja 只会把“链接命令行里出现的文件路径”记为链接依赖；若只写库名（配合
# link_directories），库文件更新后 Ninja 不知道，示例程序不会重新链接，仍留着
# 旧版本静态库生成的二进制。Skia 由 deps 重新编译后必须能触发重新链接。
# 绝对路径只用于静态库（.a/.lib 会被完整吞进链接产物，不写进 install_name/rpath）；
# 共享库（SDL3 .dylib/.so）仍保留库名由 -l 解析，避免把构建机绝对路径写进
# dylib 的 install_name/rpath，影响打包后的可迁移性。
function(_duilib_resolve_static_libs out_var lib_dir)
    set(_result "")
    foreach(_lib IN LISTS ARGN)
        set(_found "")
        foreach(_candidate
                "${lib_dir}/${_lib}.lib"
                "${lib_dir}/${_lib}d.lib"
                "${lib_dir}/lib${_lib}.a"
                "${lib_dir}/lib${_lib}.dll.a"
                "${lib_dir}/${_lib}.a"
                "${lib_dir}/${_lib}.dll.a")
            if(EXISTS "${_candidate}")
                set(_found "${_candidate}")
                break()
            endif()
        endforeach()
        if(_found)
            list(APPEND _result "${_found}")
        else()
            list(APPEND _result "${_lib}")
        endif()
    endforeach()
    set(${out_var} "${_result}" PARENT_SCOPE)
endfunction()

set(DUILIB_SKIA_LIBS svg skshaper skottie sksg jsonreader skia)
_duilib_check_deps_crt("Skia" "${DUILIB_SKIA_LIB_PATH}")
if(NOT DUILIB_SKIA_LIB_PATH MATCHES "\\$<")
    # 多配置生成器下库目录可能含生成器表达式，此时保持库名，交给链接器解析。
    # 其余情况（含 Unix 单配置）把 Skia 静态库解析为绝对路径，让 Ninja 把库文件
    # 记为链接依赖：deps 重新编译 Skia 后，示例程序才会重新链接。
    _duilib_resolve_static_libs(_duilib_skia_abs_libs
                                "${DUILIB_SKIA_LIB_PATH}" ${DUILIB_SKIA_LIBS})
    set(DUILIB_SKIA_LIBS "${_duilib_skia_abs_libs}")
endif()

#SDL库根目录，lib文件目录（Windows系统可选，其他平台必选）
if(DUILIB_ENABLE_SDL)
    if(DUILIB_SDL_ROOT)
        get_filename_component(DUILIB_SDL_SRC_ROOT_DIR "${DUILIB_SDL_ROOT}" ABSOLUTE)
    else()
        get_filename_component(DUILIB_SDL_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../SDL3/" ABSOLUTE)
    endif()
    _duilib_check_deps_crt("SDL3" "${DUILIB_SDL_SRC_ROOT_DIR}")
    if(EXISTS "${DUILIB_SDL_SRC_ROOT_DIR}/lib64/")
        set(DUILIB_SDL_LIB_PATH "${DUILIB_SDL_SRC_ROOT_DIR}/lib64")
    else()
        set(DUILIB_SDL_LIB_PATH "${DUILIB_SDL_SRC_ROOT_DIR}/lib")
    endif()
    if(DUILIB_OS_WINDOWS AND NOT MINGW)
        # MSVC：Debug/Release 使用各自的静态库；只准备了一份时退回同一份。
        # 这里写绝对路径，Ninja 才能把库文件记为链接依赖（SDL3 重新编译后会重新链接）。
        if(EXISTS "${DUILIB_SDL_LIB_PATH}/SDL3-staticd.lib" AND
           EXISTS "${DUILIB_SDL_LIB_PATH}/SDL3-static.lib")
            set(DUILIB_SDL_LIBS
                "$<$<CONFIG:Debug>:${DUILIB_SDL_LIB_PATH}/SDL3-staticd.lib>$<$<NOT:$<CONFIG:Debug>>:${DUILIB_SDL_LIB_PATH}/SDL3-static.lib>")
        elseif(EXISTS "${DUILIB_SDL_LIB_PATH}/SDL3-staticd.lib")
            set(DUILIB_SDL_LIBS "${DUILIB_SDL_LIB_PATH}/SDL3-staticd.lib")
        elseif(EXISTS "${DUILIB_SDL_LIB_PATH}/SDL3-static.lib")
            set(DUILIB_SDL_LIBS "${DUILIB_SDL_LIB_PATH}/SDL3-static.lib")
        else()
            message(FATAL_ERROR
                "SDL3 静态库不存在：${DUILIB_SDL_LIB_PATH}\n"
                "请执行 'build.py deps'（Windows 会按 Debug/Release 分别编译），"
                "或传入 -DDUILIB_SDL_ROOT=<已安装的 SDL3 前缀>。")
        endif()
    elseif(DUILIB_OS_WINDOWS AND EXISTS "${DUILIB_SDL_LIB_PATH}/libSDL3.a")
        # MinGW：静态库用绝对路径，理由同上。
        set(DUILIB_SDL_LIBS "${DUILIB_SDL_LIB_PATH}/libSDL3.a")
    else()
        set(DUILIB_SDL_LIBS SDL3)
    endif()
endif()

# 记录目标的构建配置：写 bin/.build-config/<目标>（见 duilib_write_build_stamp.cmake）。
# pack.py 据此判断 bin/ 中是否混有其他配置的同名产物。
function(duilib_add_build_config_stamp target)
    if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        return()
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
                "-DSTAMP_FILE=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/.build-config/${target}"
                "-DSTAMP_CONFIG=$<CONFIG>"
                -P "${CMAKE_CURRENT_LIST_DIR}/duilib_write_build_stamp.cmake"
        COMMENT "Recording build configuration for ${target}"
        VERBATIM)
endfunction()

#输出日志：打印变量数据（实现在 duilib_log_variables.cmake 中）
include("${CMAKE_CURRENT_LIST_DIR}/duilib_log_variables.cmake")
