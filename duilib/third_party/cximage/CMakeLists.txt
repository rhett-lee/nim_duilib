cmake_minimum_required(VERSION 3.16)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(cximage CXX)

if(MSVC)
    add_compile_options("/utf-8")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_CXX_STANDARD 17) # C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++17

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

#设置库文件的输出目录
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/libs/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/libs/")

include_directories(${CMAKE_CURRENT_LIST_DIR}/../zlib)

set(CXIMAGE_HDRS
    xfile.h
    ximacfg.h
    ximadef.h
    ximage.h
    ximagif.h
    ximaico.h
    ximaiter.h
    ximapng.h
    ximath.h
    xiofile.h
    xmemfile.h
)

set(CXIMAGE_SRCS
    ximaenc.cpp
    ximage.cpp
    ximagif.cpp
    ximaico.cpp
    ximainfo.cpp
    ximalpha.cpp
    ximapal.cpp
    ximapng.cpp
    ximath.cpp
    ximatran.cpp
    xmemfile.cpp
)

add_library(duilib-cximage STATIC ${CXIMAGE_SRCS} ${CXIMAGE_HDRS})
