# Skia 编译工具链选择（与主工程工具链解耦）
#
# 非 Windows：优先 clang/clang++。GCC 编译出的 Skia 运行性能明显较差，会直接影响
# 界面体验，因此只要 clang/clang++ 都可用就用 clang；都不可用时才回退 gcc/g++。
# 但 clang 主版本必须 >= 12：Skia 的 ml3 目标在 Linux/x64 上固定使用
# `-march=x86-64-v3`，该取值自 clang 12 起才存在，更旧的 clang（如中科方德自带的
# 11.x）会直接报 `unknown target CPU 'x86-64-v3'`，因此这类 clang 按“不可用”处理。
# 同理 gcc 需要 >= 11（`-march=x86-64-v3` / `x86-64-v4` 自 gcc 11 起支持），
# 两个编译器都过旧时由 duilib_skia_toolchain_check() 给出可读错误。
# Windows：Skia 必须与主工程 ABI 保持一致，继续跟随主工具链
#   （MSVC/clang-cl → llvm，MinGW + gcc → gcc，MinGW + clang → llvm）。
#
# 变体名对应 Skia 生成目录 out/<variant>.<arch>.<config>：
#   llvm —— clang/clang++
#   gcc  —— gcc/g++

include_guard(GLOBAL)

# 错误文案统一取自 cmake/i18n/messages.json（重复 include 安全）。
include("${CMAKE_CURRENT_LIST_DIR}/duilib_i18n.cmake")

# MSYS2 判定（决定 Skia/SDL3 的变体目录前缀）
#
# 判断 MinGW 工具链是否来自 MSYS2，并输出环境名（mingw64/ucrt64/clang64/clangarm64）：
#    MSYS2 的编译器位于 <root>/<env>/bin，安装根目录下存在 usr/bin/pacman.exe；
#    独立 MinGW-w64 不满足该特征。
# 不能用 MSYSTEM 环境变量判断：Git Bash 同样会设置 MSYSTEM=MINGW64。
# 输出为空表示不是 MSYS2。根工程（duilib_root_options.cmake）与子项目
# （duilib_common.cmake）共用本实现，避免两处口径漂移。
# duilib_detect_msys2(<out_var>)
function(duilib_detect_msys2 out_var)
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

# duilib_skia_clang_major(<out_var> <clangxx>)
# 取 clang/clang++ 的主版本号。输出 0 表示无法识别（调用方按“未知”处理，
# 保持原有选择，避免误判正常环境）：如 "Debian clang version 11.0.1-2+nfs5"、
# "clang version 18.1.8"。
function(duilib_skia_clang_major out_var clangxx)
    set(${out_var} 0 PARENT_SCOPE)
    execute_process(COMMAND "${clangxx}" --version
                    OUTPUT_VARIABLE _duilib_clang_version
                    ERROR_QUIET
                    RESULT_VARIABLE _duilib_clang_rc)
    if(NOT _duilib_clang_rc EQUAL 0)
        return()
    endif()
    if(_duilib_clang_version MATCHES "clang version ([0-9]+)")
        set(${out_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    endif()
endfunction()

# duilib_skia_gcc_major(<out_var>)
# 取 PATH 中 gcc 的主版本号（Skia 回退 gcc 时实际用的就是它）。输出 0 表示
# 无法识别（未找到或输出格式不认识），调用方按“未知”处理。
function(duilib_skia_gcc_major out_var)
    set(${out_var} 0 PARENT_SCOPE)
    find_program(_duilib_skia_gcc_hit gcc PATHS ENV PATH NO_DEFAULT_PATH NO_CACHE)
    if(NOT _duilib_skia_gcc_hit)
        return()
    endif()
    execute_process(COMMAND "${_duilib_skia_gcc_hit}" -dumpversion
                    OUTPUT_VARIABLE _duilib_gcc_version
                    ERROR_QUIET
                    RESULT_VARIABLE _duilib_gcc_rc)
    if(NOT _duilib_gcc_rc EQUAL 0)
        return()
    endif()
    string(STRIP "${_duilib_gcc_version}" _duilib_gcc_version)
    if(_duilib_gcc_version MATCHES "^([0-9]+)")
        set(${out_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    endif()
endfunction()

# duilib_skia_toolchain(<out_variant> <out_cc> <out_cxx>)
function(duilib_skia_toolchain out_variant out_cc out_cxx)
    set(_variant "llvm")
    if(WIN32)
        if(DUILIB_COMPILER_GCC)
            set(_variant "gcc")
        endif()
    else()
        # UOS（统信）上用 clang++ 链接会失败，旧脚本固定使用 gcc；这里保持同样的兜底。
        set(_duilib_uos FALSE)
        if(EXISTS "/etc/os-release")
            file(READ "/etc/os-release" _duilib_os_release)
            if(_duilib_os_release MATCHES "(^|\n)ID=\"?uos\"?(\n|$)")
                set(_duilib_uos TRUE)
            endif()
        endif()
        if(_duilib_uos)
            set(_variant "gcc")
        else()
            find_program(_duilib_skia_clang_hit clang PATHS ENV PATH NO_DEFAULT_PATH NO_CACHE)
            find_program(_duilib_skia_clangxx_hit clang++ PATHS ENV PATH NO_DEFAULT_PATH NO_CACHE)
            if(NOT _duilib_skia_clang_hit OR NOT _duilib_skia_clangxx_hit)
                set(_variant "gcc")
            else()
                # 旧版 clang（< 12）不支持 Skia ml3 目标的 `-march=x86-64-v3`，
                # 与其在编译 Skia 时失败，不如直接回退 gcc/g++。
                duilib_skia_clang_major(_duilib_skia_clang_major_version
                                        "${_duilib_skia_clangxx_hit}")
                if(_duilib_skia_clang_major_version GREATER 0 AND
                   _duilib_skia_clang_major_version LESS 12)
                    set(_variant "gcc")
                endif()
            endif()
        endif()
    endif()

    set(${out_variant} "${_variant}" PARENT_SCOPE)
    if(_variant STREQUAL "gcc")
        set(${out_cc} "gcc" PARENT_SCOPE)
        set(${out_cxx} "g++" PARENT_SCOPE)
    else()
        set(${out_cc} "clang" PARENT_SCOPE)
        set(${out_cxx} "clang++" PARENT_SCOPE)
    endif()
endfunction()

# duilib_skia_toolchain_check()
# 校验 duilib_skia_toolchain() 选出的工具链能不能编译 Skia：ml3/ml4 目标在
# Linux/x64 上固定使用 `-march=x86-64-v3` / `x86-64-v4`，只有 clang 12+ / gcc 11+
# 认识这组取值。clang 过旧时已在上面的选择逻辑里回退 gcc，这里再把“gcc 也过旧”
# 报成可读错误，避免只在编译 Skia 时才看到 ninja 的 `bad value ... for '-march='`。
function(duilib_skia_toolchain_check)
    if(WIN32)
        return()
    endif()
    duilib_skia_toolchain(_duilib_check_variant _duilib_check_cc _duilib_check_cxx)
    if(NOT _duilib_check_variant STREQUAL "gcc")
        return()
    endif()
    duilib_skia_gcc_major(_duilib_check_gcc_major)
    if(_duilib_check_gcc_major GREATER 0 AND _duilib_check_gcc_major LESS 11)
        duilib_tf(_duilib_check_what "preflight.skia_gcc_too_old"
                  "version=${_duilib_check_gcc_major}")
        duilib_t("preflight.skia_gcc_too_old_hint" _duilib_check_hint)
        message(FATAL_ERROR "${_duilib_check_what}\n  Hint: ${_duilib_check_hint}")
    endif()
endfunction()
