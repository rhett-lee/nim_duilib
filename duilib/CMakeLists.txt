cmake_minimum_required(VERSION 3.21)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(duilib CXX C)

if(MSVC)
    add_compile_options("/utf-8")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_CXX_STANDARD 20) # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++20

set(CMAKE_C_STANDARD 17) # C17
set(CMAKE_C_STANDARD_REQUIRED ON) # C17

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

add_definitions(-DSK_GANESH -DSK_GL -DSK_RELEASE)

#设置include目录
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
get_filename_component(SKIA_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../skia/" ABSOLUTE)
get_filename_component(SDL_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../SDL3/" ABSOLUTE)

include_directories(${DUILIB_SRC_ROOT_DIR})
include_directories(${SKIA_SRC_ROOT_DIR})
include_directories("${SDL_SRC_ROOT_DIR}/include")

#设置库文件的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/libs/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/libs/")

#添加源代码文件
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} DIR_LIB_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Animation ANIMATION_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Box BOX_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Control CONTROL_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Core CORE_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Image IMAGE_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Render RENDER_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/RenderSkia RENDER_SKIA_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Utils UTILS_SRC)

aux_source_directory(${DUILIB_SRC_ROOT_DIR}/duilib/third_party/apng APNG_SRC)
aux_source_directory(${DUILIB_SRC_ROOT_DIR}/duilib/third_party/stb_image STB_SRC)
aux_source_directory(${DUILIB_SRC_ROOT_DIR}/duilib/third_party/svg SVG_SRC)
aux_source_directory(${DUILIB_SRC_ROOT_DIR}/duilib/third_party/convert_utf UTF_SRC)
aux_source_directory(${DUILIB_SRC_ROOT_DIR}/duilib/third_party/xml XML_SRC)

set(UNZIP_HDRS
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/zlib/contrib/minizip/ioapi.h
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/zlib/contrib/minizip/unzip.h
)

set(UNZIP_SRCS
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/zlib/contrib/minizip/ioapi.c
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/zlib/contrib/minizip/unzip.c
)

set(SKIA_HDRS
    ${SKIA_SRC_ROOT_DIR}/tools/window/DisplayParams.h
    ${SKIA_SRC_ROOT_DIR}/tools/window/RasterWindowContext.h
    ${SKIA_SRC_ROOT_DIR}/tools/window/WindowContext.h
    ${SKIA_SRC_ROOT_DIR}/tools/window/GLWindowContext.h
)

set(SKIA_SRCS
    ${SKIA_SRC_ROOT_DIR}/tools/window/WindowContext.cpp
    ${SKIA_SRC_ROOT_DIR}/tools/window/GLWindowContext.cpp
)

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
    	    ${APNG_SRC}
    	    ${STB_SRC}
    	    ${SVG_SRC}
    	    ${UTF_SRC}
    	    ${XML_SRC}
    	    ${UNZIP_HDRS}
    	    ${UNZIP_SRCS}
    	    ${SKIA_HDRS}
    	    ${SKIA_SRCS}
           )
