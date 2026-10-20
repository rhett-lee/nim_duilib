cmake_minimum_required(VERSION 3.18)

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

# 定义项目名称和开发语言
project(WebView2Browser CXX)

# 定义使用WebView2的可执行程序
set(DUILIB_WEBVIEW2_EXE ON)

# 包含示例程序的公共实现（CM-11）
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/duilib_example.cmake")
