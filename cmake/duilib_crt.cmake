# CRT 标识的唯一推导点
#
# 该标识写入依赖产物目录（.duilib-crt，由 duilib_deps_run.cmake 按
# -DDUILIB_CRT_ID 写入），配置期再比对当前工具链的 CRT 是否一致。
# duilib_root_options.cmake（DUILIB_CRT_ID）与 duilib_common.cmake（依赖目录
# 比对）必须得到同一结果，此前两处各有一份 if/else 实现，现统一到本文件。
#
# MSYS2 判定同样只有一份实现（duilib_skia_toolchain.cmake::duilib_detect_msys2）：
# 依据编译器安装位置（<root>/<env>/bin 且 <root>/usr/bin/pacman.exe），
# 不能用 MSYSTEM —— Git Bash 也设置 MSYSTEM=MINGW64，会让同一个构建在不同
# shell（cmd / MSYS2 / Git Bash）里得到不同的 CRT 标识，进而误报 CRT mismatch。

include("${CMAKE_CURRENT_LIST_DIR}/duilib_skia_toolchain.cmake")

# duilib_crt_id(<out_var>)
#   MSVC          -> msvc-md / msvc-mt（按 DUILIB_MD）
#   MSYS2 MinGW   -> mingw64-msvcrt / mingw32-msvcrt（msvcrt）/
#                   <环境名小写>（UCRT64、CLANG64 等）
#   独立 MinGW-w64-> mingw-unknown
#   其他          -> system
function(duilib_crt_id out_var)
    set(_crt "")
    if(MSVC)
        if(DUILIB_MD)
            set(_crt "msvc-md")
        else()
            set(_crt "msvc-mt")
        endif()
    elseif(MINGW)
        set(_crt "mingw-unknown")
        # 有编译器路径时以安装位置为准；只有在脱离配置直接调用（测试夹具）时才
        # 回退到 MSYSTEM，避免同一个构建因 shell 不同而得到不同标识。
        set(_duilib_compiler "${CMAKE_CXX_COMPILER}")
        if(_duilib_compiler STREQUAL "")
            set(_duilib_compiler "${CMAKE_C_COMPILER}")
        endif()
        set(_duilib_env "")
        if(NOT _duilib_compiler STREQUAL "")
            duilib_detect_msys2(_duilib_env)
            string(TOLOWER "${_duilib_env}" _duilib_env)
        elseif(DEFINED ENV{MSYSTEM})
            string(TOLOWER "$ENV{MSYSTEM}" _duilib_env)
        endif()
        if(NOT _duilib_env STREQUAL "")
            # msvcrt 与 UCRT 的静态库符号不同：MINGW64/MINGW32 需要单独标识
            if(_duilib_env STREQUAL "mingw64" OR _duilib_env STREQUAL "mingw32")
                set(_crt "${_duilib_env}-msvcrt")
            else()
                set(_crt "${_duilib_env}")
            endif()
        endif()
    else()
        set(_crt "system")
    endif()
    set(${out_var} "${_crt}" PARENT_SCOPE)
endfunction()
