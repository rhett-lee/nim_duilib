# Skia 编译工具链选择（与主工程工具链解耦）
#
# 非 Windows：优先 clang/clang++。GCC 编译出的 Skia 运行性能明显较差，会直接影响
# 界面体验，因此只要 clang/clang++ 都可用就用 clang；都不可用时才回退 gcc/g++。
# Windows：Skia 必须与主工程 ABI 保持一致，继续跟随主工具链
#   （MSVC/clang-cl → llvm，MinGW + gcc → gcc，MinGW + clang → llvm）。
#
# 变体名对应 Skia 生成目录 out/<variant>.<arch>.<config>：
#   llvm —— clang/clang++
#   gcc  —— gcc/g++

include_guard(GLOBAL)

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
