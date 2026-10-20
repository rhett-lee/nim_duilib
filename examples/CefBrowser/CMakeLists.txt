cmake_minimum_required(VERSION 3.18)

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

# 定义项目名称和开发语言
project(CefBrowser CXX)

# 使用CEF模块
set(DUILIB_ENABLE_CEF ON)

# 定义源代码文件需要枚举的子目录（Windows平台额外枚举Windows相关子目录）
set(DUILIB_SRC_SUB_DIRS "browser")
set(DUILIB_SRC_SUB_DIRS_WINDOWS "Windows" "Windows/taskbar")

# 定义资源的子目录名称（位于resources/themes/default的子目录名称，MacOS复制资源时使用）
set(DUILIB_THEME_DIR_NAME "cef_browser")

# 包含示例程序的公共实现（CM-11）
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/duilib_example.cmake")
