cmake_minimum_required(VERSION 3.18)

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

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows
    add_definitions(-DUNICODE -D_UNICODE)
endif()

add_definitions(-DSK_GANESH -DSK_GL -DSK_RELEASE)

#设置libcef目录路径
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")  # macOS
    set(CEF_LIB_DIR "libcef_macos")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")  # Linux
    set(CEF_LIB_DIR "libcef_linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows
    set(CEF_LIB_DIR "libcef_win")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

#设置include目录
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
get_filename_component(CEF_INCLUDE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/${CEF_LIB_DIR}/" ABSOLUTE)
get_filename_component(SKIA_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../skia/" ABSOLUTE)
get_filename_component(SDL_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../SDL3/" ABSOLUTE)

include_directories(${DUILIB_SRC_ROOT_DIR})
include_directories(${CEF_INCLUDE_ROOT_DIR})
include_directories(${SKIA_SRC_ROOT_DIR})
include_directories("${SDL_SRC_ROOT_DIR}/include")

#设置库文件的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DUILIB_SRC_ROOT_DIR}/lib/")

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

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/CEFControl CEF_CONTROL_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/CEFControl/internal CEF_CONTROL_INTERNAL_SRC)

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

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows平台，单独依赖的几个Skia源码文件
    set(SKIA_WGL_SRCS
        ${SKIA_SRC_ROOT_DIR}/tools/gpu/gl/win/SkWGL.h
        ${SKIA_SRC_ROOT_DIR}/tools/gpu/gl/win/SkWGL_win.cpp
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows平台，单独依赖的源码文件
    set(UDIS_HDRS 
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/decode.h
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/extern.h
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/itab.h
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/syn.h
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/types.h
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/udint.h
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/udis86.h
    )
    set(UDIS_SRCS 
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/udis86.c
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/syn-att.c
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/syn-intel.c
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/syn.c
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/itab.c
        ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/decode.c
    )
endif()

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
    	    ${CEF_CONTROL_SRC}
    	    ${CEF_CONTROL_INTERNAL_SRC}
    	    ${APNG_SRC}
    	    ${STB_SRC}
    	    ${SVG_SRC}
    	    ${UTF_SRC}
    	    ${XML_SRC}
    	    ${UNZIP_HDRS}
    	    ${UNZIP_SRCS}
    	    ${SKIA_HDRS}
    	    ${SKIA_SRCS}
    	    ${SKIA_WGL_SRCS}
    	    ${UDIS_HDRS}
    	    ${UDIS_SRCS}
           )

# macOS 特有链接库
if(APPLE)
    find_package(Freetype REQUIRED)
    find_library(CORETEXT_LIBRARY CoreText)
    target_link_libraries(duilib
        PRIVATE
        Freetype::Freetype
        ${CORETEXT_LIBRARY}
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
    )
endif()
