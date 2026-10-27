cmake_minimum_required(VERSION 3.18)

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

# 定义项目名称和开发语言
project(basic CXX)

# macOS 下编译为 App Bundle（bin/basic.app）；其他平台忽略。
# 参考文档：build/macos-app-bundle.md
set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)

# 包含示例程序的公共实现（CM-11）
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/duilib_example.cmake")
