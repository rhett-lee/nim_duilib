cmake_minimum_required(VERSION 3.18)

set(PROJECT_NAME move_control)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(${PROJECT_NAME} CXX)

if(MSVC)
    add_compile_options("/utf-8")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_CXX_STANDARD 20) # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++20

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)
get_filename_component(SKIA_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../skia/" ABSOLUTE)
get_filename_component(SDL_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../SDL3/" ABSOLUTE)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRC_FILES)

include_directories(${DUILIB_SRC_ROOT_DIR})
link_directories("${DUILIB_SRC_ROOT_DIR}/libs/")
link_directories("${SKIA_SRC_ROOT_DIR}/out/LLVM.x64.Release/")
link_directories("${SDL_SRC_ROOT_DIR}/lib64/")
link_directories("${SDL_SRC_ROOT_DIR}/lib/")

#设置可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/bin/")

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} duilib SDL3 skia duilib-cximage duilib-webp duilib-png duilib-zlib freetype fontconfig)
