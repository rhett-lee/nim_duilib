# CRT 标识的唯一推导点
#
# 该标识写入依赖产物目录（.duilib-crt，由 duilib_deps_run.cmake 按
# -DDUILIB_CRT_ID 写入），配置期再比对当前工具链的 CRT 是否一致。
# duilib_root_options.cmake（DUILIB_CRT_ID）与 duilib_common.cmake（依赖目录
# 比对）必须得到同一结果，此前两处各有一份 if/else 实现，现统一到本文件。

# duilib_crt_id(<out_var>)
#   MSVC          -> msvc-md / msvc-mt（按 DUILIB_MD）
#   MinGW / MSYS2 -> mingw64-msvcrt（MSYSTEM=MINGW64）/
#                    <msystem 小写>（UCRT64、CLANG64 等）/ mingw-unknown
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
