cmake_minimum_required(VERSION 3.18)

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

# 定义项目名称和开发语言（<project_name> 替换为实际工程名，需与 examples/ 下的目录名一致）
project(<project_name> CXX)

# duilib 的源码根目录
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

# 项目源码目录
get_filename_component(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/" ABSOLUTE)

# 定义资源（*.rc）名，默认名称是 "项目名称.rc"，文件编码需要改为UTF8编码
# 资源文件需要放在源码所在目录中，只有Windows平台有效，其他平台忽略该定义
set(DUILIB_WINRES_FILE_NAME "${PROJECT_NAME}.rc")

# 可选开关：必须写在 include(duilib_common.cmake) 之前，写在之后不生效
# set(DUILIB_ENABLE_SDL ON)       # 启用SDL（Windows默认OFF，其他平台默认ON）
# set(DUILIB_ENABLE_CEF ON)       # 启用CEF模块（默认OFF）
# set(DUILIB_WEBVIEW2_EXE ON)     # 使用WebView2控件（仅Windows有效，默认OFF）

# 包含公共实现代码
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_common.cmake")

# 可选变量：必须写在 include(duilib_bin.cmake) 之前
# set(DUILIB_SRC_SUB_DIRS "sub_dir1" "sub_dir2")        # 源代码文件需要枚举的子目录（相对于项目源码目录）
# set(DUILIB_THEME_DIR_NAME "<theme_dir_name>")         # 资源子目录名（可选；显式值优先并校验存在，未设置时按项目名自动匹配 themes/default 下的目录，macOS 复制资源时使用）

# 包含公共实现代码
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_bin.cmake")
