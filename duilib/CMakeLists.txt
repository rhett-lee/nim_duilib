cmake_minimum_required(VERSION 3.16)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(duilib CXX)

if(MSVC)
    add_compile_options("/utf-8")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_CXX_STANDARD 20) # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++20

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

add_definitions(-DSK_GANESH -DSK_GL -DSK_RELEASE)

get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
get_filename_component(SKIA_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../skia/" ABSOLUTE)
get_filename_component(SDL_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../SDL3/include/" ABSOLUTE)

include_directories(${DUILIB_SRC_ROOT_DIR})
include_directories(${SKIA_SRC_ROOT_DIR})
include_directories(${SDL_SRC_ROOT_DIR})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} DIR_LIB_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Animation ANIMATION_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Box BOX_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Control CONTROL_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Core CORE_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Image IMAGE_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Render RENDER_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/RenderSkia RENDER_SKIA_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Utils UTILS_SRC)

add_library(duilib
	${DIR_LIB_SRC}
	${ANIMATION_SRC}
	${BOX_SRC}
	${CONTROL_SRC}
	${CORE_SRC}
    ${IMAGE_SRC}
	${RENDER_SRC}
	${RENDER_SKIA_SRC}
	${UTILS_SRC}
)
