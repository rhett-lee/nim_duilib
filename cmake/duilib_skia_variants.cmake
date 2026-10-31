# Skia 变体目录名候选（唯一实现）
#
# 以下三个调用点必须使用同一套规则，否则会在“配置期解析 / off 模式检查 /
# 依赖准备”之间漂移（历史问题：/MD 后缀只改了其中一处，导致 Debug 找不到
# llvm.<arch>.debug.mdd）：
#   - cmake/duilib_common.cmake     配置期按配置解析 out/<variant>
#   - cmake/duilib_deps.cmake       DUILIB_DEPS_MODE=off 存在性检查
#   - cmake/duilib_deps_run.cmake   依赖准备时选择要编译的 out/<variant>
#
# 本函数读取以下调用方变量（三处调用点均已定义）：
#   DUILIB_MD            使用 /MD（MSVC 动态运行库）
#   DUILIB_COMPILER_GCC  MinGW 下使用 gcc（否则用 llvm/clang）
#   DUILIB_MSYS2_ENV     MSYS2 环境名（mingw64/ucrt64/clang64/...），空表示非 MSYS2
#   DUILIB_MINGW_NAME    独立 MinGW-w64 的前缀 "mingw64-"，MSYS2 为空
#
# 参数：
#   config    debug / release（其它值按 release 处理）
#   platform  windows / 其它（与 DUILIB_DEPS_PLATFORM 取值一致）
#   arch      x86 / x64 / arm64（目录名中的架构段）
#   mingw     是否 MinGW 工具链（三处调用点的变量名/来源不同，显式传入）
#   out_var   输出：按优先级排列的候选目录名

include("${CMAKE_CURRENT_LIST_DIR}/duilib_skia_toolchain.cmake")

function(duilib_skia_variant_candidates config platform arch mingw out_var)
    string(TOLOWER "${config}" _cfg)
    if(NOT _cfg MATCHES "^(debug|release)$")
        set(_cfg "release")
    endif()

    set(_candidates "")
    if("${platform}" STREQUAL "windows")
        if(NOT mingw)
            # MSVC / clang-cl：Debug 必须使用 debug 变体，不能回退 release。
            # 动态运行库（DUILIB_MD）必须使用 /MD 编译的变体：
            # Debug -> llvm.<arch>.debug.mdd，Release -> llvm.<arch>.release.md
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
            # MinGW：MSYS2 使用无前缀目录（gcc./llvm.），独立 MinGW-w64 使用 mingw64- 前缀。
            # 优先本工具链的前缀，另一组作为回退，避免用户只准备了一份 Skia 时无法构建。
            # 库与 Skia 必须同源：gcc(libstdc++) 与 llvm-mingw(libc++) 的 C++ 运行时不兼容，
            # 混用会在链接阶段出现 std 符号重复定义，因此不做跨编译器回退。
            if("${DUILIB_MINGW_NAME}" STREQUAL "")
                set(_prefixes "" "mingw64-")
            else()
                set(_prefixes "mingw64-" "")
            endif()
            if(DUILIB_COMPILER_GCC)
                set(_compilers "gcc")
                # MSYS2 的 MINGW64 环境使用 msvcrt，与 UCRT64 的 gcc 静态库不能混用：
                # 该环境的 Skia 放在独立的 gcc.msvcrt.<arch>.release 目录，优先匹配。
                if("${DUILIB_MSYS2_ENV}" STREQUAL "mingw64")
                    # MINGW64 暂无 debug 变体，Debug 构建同样复用 release Skia
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
        # 非Windows平台：Skia 只提供 release 变体（D-3）
        # 优先 clang 版（GCC 版运行性能较差）；未准备时允许复用已有 gcc 版
        duilib_skia_toolchain(_variant _cc _cxx)
        list(APPEND _candidates "${_variant}.${arch}.release")
        if(_variant STREQUAL "llvm")
            list(APPEND _candidates "gcc.${arch}.release")
        endif()
    endif()

    set(${out_var} "${_candidates}" PARENT_SCOPE)
endfunction()
