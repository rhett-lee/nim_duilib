cmake_minimum_required(VERSION 3.18)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

# duilib
# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
if("${DUILIB_MD}" STREQUAL "ON")
    # MD/MDd
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")
else()
    # MT/MTd
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
# duilib end

project(duilib CXX C)

if(MSVC)
    # 源文件为UTF8编码
    add_compile_options("/utf-8")
    
    # 获取系统逻辑核心数
    cmake_host_system_information(RESULT CPU_CORES QUERY NUMBER_OF_LOGICAL_CORES)
    # 设置C/C++编译选项（开启多线程编译）
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/MP${CPU_CORES}>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/MP${CPU_CORES}>)
endif()

set(CMAKE_CXX_STANDARD 20) # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++20

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows
    add_definitions(-DUNICODE -D_UNICODE)
endif()

#Skia的编译选项
add_definitions(-DSK_GANESH -DSK_GL -DSK_RELEASE)

#设置libcef目录路径
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")  # macOS
    set(CEF_LIB_DIR "libcef_macos")
    set(DUILIB_SRC_MACOS 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")  # Linux
    set(CEF_LIB_DIR "libcef_linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows
    set(CEF_LIB_DIR "libcef_win")
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")  # FreeBSD
    message(WARNING "CEF is not supported on FreeBSD - building without CEF support")
    set(CEF_LIB_DIR "")  # Empty or set to a dummy value
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Set include directories
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
get_filename_component(SKIA_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../skia/" ABSOLUTE)
get_filename_component(SDL_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../SDL3/" ABSOLUTE)

# Always include these directories
include_directories(${DUILIB_SRC_ROOT_DIR})
include_directories(${SKIA_SRC_ROOT_DIR})
include_directories("${SDL_SRC_ROOT_DIR}/include")

# Conditionally include CEF directory
if(CEF_LIB_DIR AND NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    get_filename_component(CEF_INCLUDE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/${CEF_LIB_DIR}/" ABSOLUTE)
    include_directories(${CEF_INCLUDE_ROOT_DIR})
else()
    message(STATUS "CEF includes disabled - not supported on this platform")
endif()

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

# Only include CEF sources on supported platforms (Windows, Linux, macOS)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/CEFControl CEF_CONTROL_SRC)
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/CEFControl/internal CEF_CONTROL_INTERNAL_SRC)
    list(APPEND PROJECT_SOURCES ${CEF_CONTROL_SRC} ${CEF_CONTROL_INTERNAL_SRC})
    message(STATUS "CEF support enabled (${CMAKE_SYSTEM_NAME})")
else()
    message(STATUS "CEF support disabled (FreeBSD is not supported)")
endif()

# 非MacOS平台，删除*.mm文件，避免出现编译错误
if(NOT DUILIB_SRC_MACOS)
    list(REMOVE_ITEM CORE_SRC ${CMAKE_CURRENT_LIST_DIR}/Core/SDL_MacOS.mm)
    list(REMOVE_ITEM CORE_SRC ${CMAKE_CURRENT_LIST_DIR}/Core/SDL_MacOS.h)    
    list(REMOVE_ITEM CORE_SRC ${CMAKE_CURRENT_LIST_DIR}/Core/ClickThrough_MacOS.mm)    
    list(REMOVE_ITEM CEF_CONTROL_SRC ${CMAKE_CURRENT_LIST_DIR}/CEFControl/CefWindowUtils_MacOS.mm)    
    list(REMOVE_ITEM UTILS_SRC ${CMAKE_CURRENT_LIST_DIR}/Utils/ScreenCapture_MacOS.mm)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/CEFControl/internal/Windows CEF_CONTROL_INTERNAL_WINDOWS_SRC)
endif()

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

# Define the base sources (always included)
set(LIBRARY_SOURCES
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
    ${SKIA_WGL_SRCS}
    ${UDIS_HDRS}
    ${UDIS_SRCS}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows, WebView2
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/WebView2 WEBVIEW2_CONTROL_SRC)
    list(APPEND LIBRARY_SOURCES
        ${WEBVIEW2_CONTROL_SRC}
    )
endif()

# Only include CEF sources on supported platforms (Windows, Linux, macOS)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    list(APPEND LIBRARY_SOURCES
        ${CEF_CONTROL_SRC}
        ${CEF_CONTROL_INTERNAL_SRC}
        ${CEF_CONTROL_INTERNAL_WINDOWS_SRC}
    )
    message(STATUS "CEF support enabled (${CMAKE_SYSTEM_NAME})")
else()
    message(STATUS "CEF support disabled (FreeBSD is not supported)")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    find_package(Freetype REQUIRED)
    find_package(Fontconfig REQUIRED)
    add_library(${PROJECT_NAME} ${LIBRARY_SOURCES})
    target_link_libraries(${PROJECT_NAME} Freetype::Freetype Fontconfig::Fontconfig)
else()
    # Create the library with the filtered sources
    add_library(${PROJECT_NAME} ${LIBRARY_SOURCES})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(DUILIB_ENABLE_SDL)
        # Enable SDL support for windows
        target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_SDL=1)
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows, WebView2
    target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_WEBVIEW2=1)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_USE_WEBVIEW2_LOADER_DLL=1)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # 由于Skia内部会自动开启AVX/AVX2等编译参数，duilib内部直接引用了部分源码，需要保持编译选项一致
    include(${DUILIB_SRC_ROOT_DIR}/cmake/duilib_compiles.cmake)

    # 编译器的参数
    set(DUILIB_COMPILER_FLAGS
        -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
        -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
        -funwind-tables                 # Support stack unwinding for backtrace()
        -fvisibility=hidden             # Give hidden visibility to declarations that are not explicitly marked as visible
        -Wall                           # Enable all warnings
#        -Werror                         # Treat warnings as errors
        -Wno-unknown-pragmas
        -Wextra                         # Enable additional warnings
        -Wendif-labels                  # Warn whenever an #else or an #endif is followed by text
        -Wno-missing-field-initializers # Don't warn about missing field initializers
        -Wno-unused-parameter           # Don't warn about unused parameters
    )
    set(DUILIB_CXX_COMPILER_FLAGS
        -fno-threadsafe-statics         # Don't generate thread-safe statics
        -fvisibility-inlines-hidden     # Give hidden visibility to inlined class member functions
        -frtti
        -Wno-narrowing                  # Don't warn about type narrowing
        -Wsign-compare                  # Warn about mixed signed/unsigned type comparisons
        -Wno-unused-variable
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND DUILIB_CXX_COMPILER_FLAGS
            -Wno-reorder
            -Wno-cast-function-type
          )
          set_source_files_properties("${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/decode.c" PROPERTIES COMPILE_OPTIONS "-Wno-implicit-fallthrough")
          if(DUILIB_HAVE_AVX2)
            set_source_files_properties("${SKIA_SRC_ROOT_DIR}/tools/window/GLWindowContext.cpp" PROPERTIES COMPILE_OPTIONS "-mavx2")
          elseif(DUILIB_HAVE_AVX)
            set_source_files_properties("${SKIA_SRC_ROOT_DIR}/tools/window/GLWindowContext.cpp" PROPERTIES COMPILE_OPTIONS "-mavx")
          endif()
    endif()
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND DUILIB_COMPILER_FLAGS
            -Wnewline-eof                   # Warn about no newline at end of file
          )
        list(APPEND DUILIB_CXX_COMPILER_FLAGS
            -fobjc-call-cxx-cdtors          # Call the constructor/destructor of C++ instance variables in ObjC objects
            -Wno-reorder-ctor
            -Wno-unused-lambda-capture
            -Wno-unused-private-field
          )
    endif()
    
    target_compile_options(${PROJECT_NAME} PRIVATE ${DUILIB_COMPILER_FLAGS})
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>: ${DUILIB_CXX_COMPILER_FLAGS}>)
endif()
