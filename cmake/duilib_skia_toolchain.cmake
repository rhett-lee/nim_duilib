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

# duilib_skia_toolchain(<out_variant> <out_cc> <out_cxx>)
function(duilib_skia_toolchain out_variant out_cc out_cxx)
    set(_variant "llvm")
    if(WIN32)
        if(DUILIB_COMPILER_GCC)
            set(_variant "gcc")
        endif()
    else()
        find_program(_duilib_skia_clang_hit clang PATHS ENV PATH NO_DEFAULT_PATH NO_CACHE)
        find_program(_duilib_skia_clangxx_hit clang++ PATHS ENV PATH NO_DEFAULT_PATH NO_CACHE)
        if(NOT _duilib_skia_clang_hit OR NOT _duilib_skia_clangxx_hit)
            set(_variant "gcc")
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
