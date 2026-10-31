# Skia 变体目录名的唯一规则（配置期解析 / off 模式检查 / 依赖准备共用）
#
# 目录名规则（out/<variant> 下）：
#   Windows MSVC / clang-cl：
#     llvm.<arch>.<debug|release>，/MD（DUILIB_MD）时追加 .mdd / .md
#   Windows MinGW（独立 MinGW-w64 带 mingw64- 前缀；MSYS2 无前缀）：
#     <prefix><gcc|llvm>.<arch>.<debug|release>
#     Debug 优先 debug 变体、缺失时回退同编译器的 release 变体；
#     MSYS2 MINGW64 环境额外提供 gcc.msvcrt.<arch>.release（msvcrt 与 UCRT64 不兼容）
#   非 Windows：
#     <llvm|gcc>.<arch>.release（Skia 只有 release 变体），clang 版优先
#
# 本文件曾被三处各自实现（duilib_common.cmake、duilib_deps.cmake、
# duilib_deps_run.cmake），并且已经漂移过一次（/MD 后缀）。这里是唯一实现，
# 另外 build/test_cmake_scripts.py 会比对 pack.py 的打包侧候选目录名。

include("${CMAKE_CURRENT_LIST_DIR}/duilib_skia_toolchain.cmake")

# duilib_skia_variant_candidates(<config> <platform> <arch> <mingw> <out_var>)
#   config   : Debug / Release（大小写不敏感；其他值按 Release）
#   platform : windows / other
#   arch     : Skia 的 CPU 架构名（x86 / x64 / arm64）
#   mingw    : 是否 MinGW 工具链（TRUE/FALSE 或 ON/OFF）
# 读取调用方作用域内的 DUILIB_MD、DUILIB_COMPILER_GCC、DUILIB_MSYS2_ENV、
# DUILIB_MINGW_NAME（三处调用点的变量名一致，只有 mingw 判定变量名不同，
# 因此作为显式参数传入）。
function(duilib_skia_variant_candidates config platform arch mingw out_var)
    string(TOLOWER "${config}" _cfg)
    if(NOT _cfg MATCHES "^(debug|release)$")
        set(_cfg "release")
    endif()
    set(_candidates "")
    if("${platform}" STREQUAL "windows")
        if(NOT mingw)
            set(_suffix "")
            if(DUILIB_MD)
                if(_cfg STREQUAL "debug")
                    set(_suffix ".mdd")
                else()
                    set(_suffix ".md")
                endif()
            endif()
            list(APPEND _candidates "llvm.${arch}.${_cfg}${_suffix}")
        else()
            # 优先本工具链的前缀，另一组作为回退，避免用户只准备了一份 Skia
            # 时无法构建。gcc(libstdc++) 与 llvm-mingw(libc++) 的 C++ 运行时不
            # 兼容，混用会在链接阶段出现 std 符号重复定义，因此不做跨编译器回退。
            if("${DUILIB_MINGW_NAME}" STREQUAL "")
                set(_prefixes "" "mingw64-")
            else()
                set(_prefixes "mingw64-" "")
            endif()
            if(DUILIB_COMPILER_GCC)
                set(_compilers "gcc")
                # MSYS2 MINGW64 使用 msvcrt，与 UCRT64 的 gcc 静态库不能混用：
                # 该环境的 Skia 放在独立的 gcc.msvcrt.<arch>.release 目录，优先匹配。
                if("${DUILIB_MSYS2_ENV}" STREQUAL "mingw64")
                    list(APPEND _candidates "gcc.msvcrt.${arch}.release")
                endif()
            else()
                set(_compilers "llvm")
            endif()
            foreach(_prefix IN LISTS _prefixes)
                foreach(_compiler IN LISTS _compilers)
                    if(_cfg STREQUAL "debug")
                        # Debug 变体优先；缺失时回退同编译器的 release 变体
                        list(APPEND _candidates
                            "${_prefix}${_compiler}.${arch}.debug"
                            "${_prefix}${_compiler}.${arch}.release")
                    else()
                        list(APPEND _candidates
                            "${_prefix}${_compiler}.${arch}.release")
                    endif()
                endforeach()
            endforeach()
        endif()
    else()
        # 非 Windows：Skia 只提供 release 变体；优先 clang 版，未准备时允许
        # 复用已有的 gcc 版
        duilib_skia_toolchain(_variant _cc _cxx)
        list(APPEND _candidates "${_variant}.${arch}.release")
        if(_variant STREQUAL "llvm")
            list(APPEND _candidates "gcc.${arch}.release")
        endif()
    endif()
    set(${out_var} "${_candidates}" PARENT_SCOPE)
endfunction()
