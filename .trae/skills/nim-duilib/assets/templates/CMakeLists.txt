cmake_minimum_required(VERSION 3.18)

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

# 定义项目名称和开发语言（<project_name> 替换为实际工程名，需与 examples/ 下的目录名一致）
project(<project_name> CXX)

# duilib 的源码根目录
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

# 项目源码目录
get_filename_component(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/" ABSOLUTE)

# 包含公共实现代码
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_common.cmake")
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_bin.cmake")
