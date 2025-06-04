cmake_minimum_required(VERSION 3.18)

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

# 定义项目名称和开发语言
project(cef CXX)

# duilib 的源码根目录
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE) 

# 项目源码目录
get_filename_component(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/" ABSOLUTE) 

# 定义资源（*.rc）名，默认名称是 "项目名称.rc"，文件编码需要改为UTF8编码
# 资源文件需要放在源码所在目录中，只有Windows平台有效，其他平台忽略该定义
set(DUILIB_WINRES_FILE_NAME "${PROJECT_NAME}.rc")

# 使用CEF模块
set(DUILIB_ENABLE_CEF ON)

# 定义资源的子目录名称（位于resources/themes/default的子目录名称，MacOS复制资源时使用）
set(DUILIB_THEME_DIR_NAME "cef")

# 包含公共实现代码
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_common.cmake")
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_bin.cmake")
