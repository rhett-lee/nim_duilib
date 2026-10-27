# 示例程序的公共 CMake 实现（CM-11）
#
# 用法（examples/<name>/CMakeLists.txt）：
#   cmake_minimum_required(VERSION 3.18)
#   set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
#   project(<name> CXX)
#   include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/duilib_example.cmake")
#
# 可在 include 本文件之前设置的示例开关：
#   DUILIB_ENABLE_CEF             使用CEF模块（全局能力开关，见 duilib_root_options.cmake）
#   DUILIB_EXAMPLE_CEF            本示例使用CEF模块（仅 cef/CefBrowser 设置；默认 OFF）
#   DUILIB_WEBVIEW2_EXE           定义使用WebView2的可执行程序
#   DUILIB_THEME_DIR_NAME         资源子目录名称（位于resources/themes/default下，macOS复制资源时使用）
#   DUILIB_SRC_SUB_DIRS           需要枚举C++源码的子目录
#   DUILIB_SRC_SUB_DIRS_WINDOWS   仅Windows平台需要枚举的源码子目录
#   DUILIB_EXAMPLE_SDL_INCLUDE    为ON时把SDL的include目录加入本示例（仅少数直接使用SDL的示例需要）

# duilib 的源码根目录（本文件所在目录的上一级）
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

# 项目源码目录
get_filename_component(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE)

# 只有显式声明的示例才链接 CEF：全局 DUILIB_ENABLE_CEF 只表示“库支持 CEF”，
# 不能让所有示例都产生 libcef 依赖（否则 basic 等程序也会导入 libcef.so/dll）。
if(NOT DEFINED DUILIB_EXAMPLE_CEF)
    set(DUILIB_EXAMPLE_CEF OFF)
endif()

# 定义资源（*.rc）名，默认名称是 "项目名称.rc"，文件编码需要改为UTF8编码
# 资源文件需要放在源码所在目录中，只有Windows平台有效，其他平台忽略该定义
set(DUILIB_WINRES_FILE_NAME "${PROJECT_NAME}.rc")

# 包含公共实现代码
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_common.cmake")

# Windows平台专有的源码子目录
if(DUILIB_OS_WINDOWS AND DUILIB_SRC_SUB_DIRS_WINDOWS)
    list(APPEND DUILIB_SRC_SUB_DIRS ${DUILIB_SRC_SUB_DIRS_WINDOWS})
endif()

# SDL库的头文件所在路径（只对需要直接使用SDL头文件的示例开放）
if(DUILIB_EXAMPLE_SDL_INCLUDE AND DUILIB_ENABLE_SDL)
    include_directories("${DUILIB_SDL_SRC_ROOT_DIR}/include")
endif()

# macOS App Bundle 开关（仅 macOS 生效，其他平台强制关闭）：
#   1. 示例自己的 CMakeLists.txt 可设置 DUILIB_EXAMPLE_MACOS_BUNDLE=ON；
#   2. 未设置时，由根工程的 DUILIB_MACOS_BUNDLE_EXAMPLES 列表决定
#      （分号/逗号分隔的目标名，all 或 * 表示全部示例）。
# 示例内的显式设置优先，便于单独启用/禁用某个示例。
if(NOT DUILIB_OS_MACOS)
    set(DUILIB_EXAMPLE_MACOS_BUNDLE OFF)
elseif(NOT DEFINED DUILIB_EXAMPLE_MACOS_BUNDLE)
    set(DUILIB_EXAMPLE_MACOS_BUNDLE OFF)
    if(DEFINED DUILIB_MACOS_BUNDLE_EXAMPLES AND
       NOT "${DUILIB_MACOS_BUNDLE_EXAMPLES}" STREQUAL "")
        string(REPLACE "," ";" _duilib_bundle_targets
               "${DUILIB_MACOS_BUNDLE_EXAMPLES}")
        set(_duilib_bundle_targets_normalized "")
        foreach(_duilib_bundle_target IN LISTS _duilib_bundle_targets)
            string(STRIP "${_duilib_bundle_target}" _duilib_bundle_target)
            if(NOT "${_duilib_bundle_target}" STREQUAL "")
                list(APPEND _duilib_bundle_targets_normalized
                     "${_duilib_bundle_target}")
            endif()
        endforeach()
        if(_duilib_bundle_targets_normalized MATCHES "^(all|ALL|All|\\*)$")
            set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)
        elseif(PROJECT_NAME IN_LIST _duilib_bundle_targets_normalized)
            set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)
        endif()
    endif()
endif()

# 包含可执行程序的公共实现代码
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_bin.cmake")
