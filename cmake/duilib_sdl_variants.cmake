# SDL3 变体前缀目录名的唯一规则（配置期解析 / 依赖检查 / 依赖准备 / 打包共用）
#
# Windows 上的 SDL3 静态库必须按“架构 × 配置 × MSVC 运行库”分开存放，否则：
#   * Debug/Release 同名覆盖（/MTd 与 /MT、/MDd 与 /MD 混链）；
#   * MT/MTd/MD/MDd 四种 RuntimeLibrary 互斥（链接期 LNK2038）；
#   * x86 / x64 / arm64 的目标文件不能混用。
# 每个变体目录都是一个完整安装前缀（include/、lib/、share/licenses），库文件名
# 在各变体内保持 SDL3-static.lib（MinGW 为 libSDL3.a）不变，由目录区分配置与运行库。
#
# 目录名规则（<deps-root>/SDL3/<variant>）：
#   MSVC：msvc.<arch>.<debug|release>，/MD（DUILIB_MD）时追加 .mdd / .md
#   MinGW：直接复用 Skia 的 release 变体名（MinGW 的 CRT 由工具链决定，
#          Debug/Release 可共用同一份静态库）：
#     独立 MinGW-w64 : mingw64-<gcc|llvm>.<arch>.release
#     MSYS2 MINGW64  : gcc.msvcrt.<arch>.release（msvcrt 与 UCRT64 的产物不兼容）
#     MSYS2          : <gcc|llvm>.<arch>.release
#   非 Windows：返回空串，表示沿用 <deps-root>/SDL3 单前缀（共享库只有 Release）
#
# 读取调用方作用域内的 DUILIB_MD、DUILIB_COMPILER_GCC、DUILIB_MSYS2_ENV、
# DUILIB_MINGW_NAME（与 duilib_skia_variants.cmake 相同的变量）。
# build/test_cmake_scripts.py 会用同一组用例比对 build/pack.py 的打包侧实现
# （_sdl_variant_name），防止两侧再次漂移。

include("${CMAKE_CURRENT_LIST_DIR}/duilib_skia_variants.cmake")

# duilib_sdl_variant_name(<config> <platform> <arch> <mingw> <out_var>)
#   config   : Debug / Release（大小写不敏感；其他值按 Release）
#   platform : windows / other
#   arch     : 目标架构名（x86 / x64 / arm64）
#   mingw    : 是否 MinGW 工具链（TRUE/FALSE 或 ON/OFF/1/0）
function(duilib_sdl_variant_name config platform arch mingw out_var)
    string(TOLOWER "${config}" _cfg)
    if(NOT _cfg MATCHES "^(debug|release)$")
        set(_cfg "release")
    endif()
    set(_variant "")
    if("${platform}" STREQUAL "windows")
        if(NOT mingw)
            set(_variant "msvc.${arch}.${_cfg}")
            if(DUILIB_MD)
                if(_cfg STREQUAL "debug")
                    string(APPEND _variant ".mdd")
                else()
                    string(APPEND _variant ".md")
                endif()
            endif()
        else()
            # MinGW 没有 MT/MD 之分，Debug/Release 的静态库 ABI 相同：统一使用
            # release 变体（与 Skia 允许 MinGW Debug 回退 release 的规则一致）。
            duilib_skia_variant_candidates("Release" "windows" "${arch}" TRUE _candidates)
            list(GET _candidates 0 _variant)
        endif()
    endif()
    set(${out_var} "${_variant}" PARENT_SCOPE)
endfunction()

# SDL3 静态库文件名（Windows）：规范名排在首位，其余为兼容探测的旧名字。
# SDL3 源码不设置 DEBUG_POSTFIX，Debug/Release 的静态库同名，因此不能靠文件名
# 区分配置；早期脚本按 MSVC 习惯猜测过 SDL3-staticd.lib，保留兼容探测。
function(duilib_sdl_static_lib_names mingw out_var)
    if(mingw)
        set(${out_var} "libSDL3.a" "libSDL3d.a" PARENT_SCOPE)
    else()
        set(${out_var} "SDL3-static.lib" "SDL3-staticd.lib" PARENT_SCOPE)
    endif()
endfunction()

# 安装前缀内的库目录（Unix 的 CMake 安装可能用 lib64；Windows 固定 lib）
function(duilib_sdl_lib_dir prefix platform out_var)
    if("${platform}" STREQUAL "windows")
        set(${out_var} "${prefix}/lib" PARENT_SCOPE)
    elseif(EXISTS "${prefix}/lib64")
        set(${out_var} "${prefix}/lib64" PARENT_SCOPE)
    else()
        set(${out_var} "${prefix}/lib" PARENT_SCOPE)
    endif()
endfunction()

# 判断一个安装前缀是否完整可用（头文件 + 当前平台/工具链的库文件）。
function(duilib_sdl_prefix_ready prefix platform mingw out_var)
    set(_ok FALSE)
    if(EXISTS "${prefix}/include/SDL3/SDL.h")
        duilib_sdl_lib_dir("${prefix}" "${platform}" _lib_dir)
        if("${platform}" STREQUAL "windows")
            duilib_sdl_static_lib_names("${mingw}" _lib_names)
            foreach(_lib_name IN LISTS _lib_names)
                if(EXISTS "${_lib_dir}/${_lib_name}")
                    set(_ok TRUE)
                    break()
                endif()
            endforeach()
        elseif("${platform}" STREQUAL "macos")
            # macOS 的安装产物是 .dylib；只看 .so 会把已完成的构建误判为缺失
            if(EXISTS "${_lib_dir}/libSDL3.dylib")
                set(_ok TRUE)
            endif()
        else()
            if(EXISTS "${_lib_dir}/libSDL3.so")
                set(_ok TRUE)
            endif()
        endif()
    endif()
    set(${out_var} ${_ok} PARENT_SCOPE)
endfunction()

# SDL3 头文件目录（供根工程与独立示例子工程共用）
#
# 头文件在各变体中内容相同，但目录必须与各自链接的库变体一致：多配置生成器下
# 若统一取 Debug 前缀，IDE 会把 Debug 变体的 include 目录写进所有配置，
# Release 目标看到的头文件目录与它实际链接的库前缀不一致。这里返回生成器
# 表达式，让每个配置使用自己的变体前缀；前缀相同（MinGW 复用 release 变体、
# 单前缀回退）时退化为普通目录。
# duilib_sdl_include_dir(<debug_prefix> <release_prefix> <out_var>)
function(duilib_sdl_include_dir debug_prefix release_prefix out_var)
    if("${debug_prefix}" STREQUAL "" AND "${release_prefix}" STREQUAL "")
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()
    if(NOT "${debug_prefix}" STREQUAL "" AND NOT "${release_prefix}" STREQUAL ""
       AND NOT "${debug_prefix}" STREQUAL "${release_prefix}")
        set(${out_var}
            "$<$<CONFIG:Debug>:${debug_prefix}/include>$<$<NOT:$<CONFIG:Debug>>:${release_prefix}/include>"
            PARENT_SCOPE)
    elseif(NOT "${debug_prefix}" STREQUAL "")
        set(${out_var} "${debug_prefix}/include" PARENT_SCOPE)
    else()
        set(${out_var} "${release_prefix}/include" PARENT_SCOPE)
    endif()
endfunction()

# 候选安装前缀（顺序即优先级）：<root>/<variant> 优先；<root> 兼容显式指定的
# 单前缀（旧布局、自备产物或早期 deps 的安装结果）。
# duilib_sdl_prefix_candidates(<config> <platform> <arch> <mingw> <root> <out_var>)
function(duilib_sdl_prefix_candidates config platform arch mingw root out_var)
    duilib_sdl_variant_name("${config}" "${platform}" "${arch}" "${mingw}" _variant)
    set(_candidates "")
    if(NOT _variant STREQUAL "")
        list(APPEND _candidates "${root}/${_variant}")
    endif()
    list(APPEND _candidates "${root}")
    set(${out_var} "${_candidates}" PARENT_SCOPE)
endfunction()
