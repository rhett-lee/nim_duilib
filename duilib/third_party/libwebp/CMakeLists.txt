cmake_minimum_required(VERSION 3.16)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(webp C)

if(MSVC)
    add_compile_options("/utf-8")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

#设置库文件的输出目录
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/libs/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/libs/")

include_directories(${CMAKE_CURRENT_LIST_DIR})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/dec DEC_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/demux DEMUX_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/dsp DSP_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/enc ENC_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/utils UTILS_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/webp WEBP_SRC)

add_library(duilib-webp STATIC ${DEC_SRC} ${DEMUX_SRC} ${DSP_SRC} ${ENC_SRC} ${UTILS_SRC} ${WEBP_SRC})
