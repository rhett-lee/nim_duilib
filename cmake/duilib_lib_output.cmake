# duilib: 设置库产物的输出目录（CM-5 / D-2 方案 B：配置只由目录表达）
#
# 规则：
#   Windows : <root>/lib/<arch>/<Config>   (arch = x64 / x86 / arm64)
#   Unix    : <root>/lib/<Config>
#
# 用法：在 project() 之后、创建库目标之前 include 本文件。
#       包含后可用 DUILIB_LIB_BASE 作为库目录的基础路径（不含配置层）。
#
# 说明：
#   - 显式设置 4 个标准配置的输出变量，避免多配置生成器再追加一层配置目录；
#   - 单配置生成器下，自定义配置也会单独成目录，避免与标准配置互相覆盖；
#   - 与 VS 的 $(Platform)/$(Configuration) 目录对齐（Win32 对应 x86）。

get_filename_component(DUILIB_LIB_BASE "${CMAKE_CURRENT_LIST_DIR}/../lib" ABSOLUTE)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(_duilib_platform "")
    if(CMAKE_GENERATOR_PLATFORM)
        string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" _duilib_platform)
    endif()

    if(_duilib_platform MATCHES "arm64")
        set(_duilib_arch "arm64")
    elseif(_duilib_platform MATCHES "win32")
        set(_duilib_arch "x86")
    elseif(_duilib_platform MATCHES "x64")
        set(_duilib_arch "x64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_duilib_arch "x64")
    else()
        set(_duilib_arch "x86")
    endif()

    set(DUILIB_LIB_BASE "${DUILIB_LIB_BASE}/${_duilib_arch}")
endif()

foreach(_duilib_cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${_duilib_cfg}" _duilib_cfg_upper)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_duilib_cfg_upper} "${DUILIB_LIB_BASE}/${_duilib_cfg}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_duilib_cfg_upper} "${DUILIB_LIB_BASE}/${_duilib_cfg}")
endforeach()

# 兜底：多配置生成器会自动追加配置名；单配置的未知配置单独成目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DUILIB_LIB_BASE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DUILIB_LIB_BASE}")
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _duilib_bt_upper)
    if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_duilib_bt_upper})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_duilib_bt_upper} "${DUILIB_LIB_BASE}/${CMAKE_BUILD_TYPE}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_duilib_bt_upper} "${DUILIB_LIB_BASE}/${CMAKE_BUILD_TYPE}")
    endif()
endif()
