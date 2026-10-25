cmake_minimum_required(VERSION 3.18)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

# duilib
# MSVC runtime library flags are selected by an abstraction.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
if(DUILIB_MD)
    # MD/MDd
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
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
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")  # Linux
    set(CEF_LIB_DIR "libcef_linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows
    if(DUILIB_CEF_VERSION STREQUAL "109")
        set(CEF_LIB_DIR "libcef_win_109")
    else()
        set(CEF_LIB_DIR "libcef_win")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")  # FreeBSD
    message(WARNING "CEF is not supported on FreeBSD - building without CEF support")
    set(CEF_LIB_DIR "")  # Empty or set to a dummy value
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# CEF功能开关：默认开启（与加入本开关前的行为一致）
# FreeBSD平台不支持CEF，默认关闭；若外部强制打开，则在此关闭并给出提示
if(CEF_LIB_DIR)
    option(DUILIB_ENABLE_CEF "Enable CEF" ON)
else()
    option(DUILIB_ENABLE_CEF "Enable CEF" OFF)
endif()
if(DUILIB_ENABLE_CEF AND NOT CEF_LIB_DIR)
    message(WARNING "DUILIB_ENABLE_CEF is ON, but CEF is not supported on ${CMAKE_SYSTEM_NAME}: CEF is disabled")
    set(DUILIB_ENABLE_CEF OFF)
endif()

# Set include directories
get_filename_component(DUILIB_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
if(DUILIB_SKIA_ROOT)
    get_filename_component(SKIA_SRC_ROOT_DIR "${DUILIB_SKIA_ROOT}" ABSOLUTE)
else()
    get_filename_component(SKIA_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../skia/" ABSOLUTE)
endif()
if(DUILIB_SDL_ROOT)
    get_filename_component(SDL_SRC_ROOT_DIR "${DUILIB_SDL_ROOT}" ABSOLUTE)
else()
    get_filename_component(SDL_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../SDL3/" ABSOLUTE)
endif()

# Always include these directories（收集到列表，等库目标创建后应用到目标上，见 CM-17）
list(APPEND DUILIB_TARGET_INCLUDE_DIRS
    "${DUILIB_SRC_ROOT_DIR}"
    "${SKIA_SRC_ROOT_DIR}"
    "${SDL_SRC_ROOT_DIR}/include"
)

# Conditionally include CEF directory（仅在开启CEF功能时添加）
if(DUILIB_ENABLE_CEF)
    get_filename_component(CEF_INCLUDE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/libcef/${CEF_LIB_DIR}/" ABSOLUTE)
    list(APPEND DUILIB_TARGET_INCLUDE_DIRS "${CEF_INCLUDE_ROOT_DIR}")
    message(STATUS "CEF include path: ${CEF_INCLUDE_ROOT_DIR}")
else()
    message(STATUS "CEF includes disabled (DUILIB_ENABLE_CEF=OFF)")
endif()

#设置库文件的输出目录（D-2：按平台/配置分目录，规则见 cmake/duilib_lib_output.cmake）
include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_lib_output.cmake")

#添加源代码文件
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} DIR_LIB_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Animation ANIMATION_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Box BOX_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Layout LAYOUT_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Control CONTROL_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Core CORE_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Image IMAGE_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Render RENDER_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/RenderSkia RENDER_SKIA_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Utils UTILS_SRC)

# CEF源文件仅支持Windows、Linux、macOS平台，其他平台不支持
if(DUILIB_ENABLE_CEF)
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/CEFControl CEF_CONTROL_SRC)
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/CEFControl/internal CEF_CONTROL_INTERNAL_SRC)
    message(STATUS "CEF sources enabled (${CMAKE_SYSTEM_NAME})")

    # Windows平台专有的CEF源文件
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/CEFControl/internal/Windows CEF_CONTROL_INTERNAL_WINDOWS_SRC)
    endif()

    # 非macOS平台，删除macOS专有的CEF源文件(*.mm)，避免出现编译错误
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        list(REMOVE_ITEM CEF_CONTROL_SRC ${CMAKE_CURRENT_LIST_DIR}/CEFControl/CefWindowUtils_MacOS.mm)
        message(STATUS "CEF macOS-only sources removed (${CMAKE_SYSTEM_NAME} is not Darwin)")
    endif()
else()
    message(STATUS "CEF sources disabled (DUILIB_ENABLE_CEF=OFF)")
endif()

# 非macOS平台，删除macOS专有的源文件(*.mm/*.h)，避免出现编译错误
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(REMOVE_ITEM CORE_SRC ${CMAKE_CURRENT_LIST_DIR}/Core/SDL_MacOS.mm)
    list(REMOVE_ITEM CORE_SRC ${CMAKE_CURRENT_LIST_DIR}/Core/SDL_MacOS.h)
    list(REMOVE_ITEM CORE_SRC ${CMAKE_CURRENT_LIST_DIR}/Core/ClickThrough_MacOS.mm)
    list(REMOVE_ITEM UTILS_SRC ${CMAKE_CURRENT_LIST_DIR}/Utils/ScreenCapture_MacOS.mm)
    message(STATUS "macOS-only sources removed (${CMAKE_SYSTEM_NAME} is not Darwin)")
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

set(GIFLIB_HDRS
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/gif_hash.h
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/gif_lib.h
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/gif_lib_private.h
)

set(GIFLIB_SRCS
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/dgif_lib.c
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/egif_lib.c
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/gif_err.c
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/gif_font.c
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/gif_hash.c
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/gifalloc.c
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/openbsd-reallocarray.c
    ${DUILIB_SRC_ROOT_DIR}/duilib/third_party/giflib/quantize.c
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
        ${SKIA_SRC_ROOT_DIR}/tools/ganesh/gl/win/SkWGL.h
        ${SKIA_SRC_ROOT_DIR}/tools/ganesh/gl/win/SkWGL_win.cpp
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
    ${LAYOUT_SRC}    
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
    ${GIFLIB_HDRS}
    ${GIFLIB_SRCS}
    ${SKIA_HDRS}
    ${SKIA_SRCS}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows, WebView2
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/WebView2 WEBVIEW2_CONTROL_SRC)
    list(APPEND LIBRARY_SOURCES
        ${WEBVIEW2_CONTROL_SRC}
    )
    # Windows平台专有的Skia源码文件
    list(APPEND LIBRARY_SOURCES
        ${SKIA_WGL_SRCS}
    )
    # Windows平台专有的源码文件
    list(APPEND LIBRARY_SOURCES
        ${UDIS_HDRS}
        ${UDIS_SRCS}
    )
endif()

# CEF源文件仅支持Windows、Linux、macOS平台，其他平台不支持（收集与过滤见上方同名判断）
if(DUILIB_ENABLE_CEF)
    list(APPEND LIBRARY_SOURCES
        ${CEF_CONTROL_SRC}
        ${CEF_CONTROL_INTERNAL_SRC}
    )
    # Windows平台专有的CEF源文件
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        list(APPEND LIBRARY_SOURCES
            ${CEF_CONTROL_INTERNAL_WINDOWS_SRC}
        )
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    # 字体相关依赖（与示例程序共用同一份定义）
    include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_unix_font_libs.cmake")
    add_library(${PROJECT_NAME} ${LIBRARY_SOURCES})
    target_link_libraries(${PROJECT_NAME} ${DUILIB_FONT_LIBS})
else()
    # Create the library with the filtered sources
    add_library(${PROJECT_NAME} ${LIBRARY_SOURCES})
endif()

# 把include目录应用到库目标上（CM-17：不再使用目录级的全局 include_directories）
target_include_directories(${PROJECT_NAME} PRIVATE ${DUILIB_TARGET_INCLUDE_DIRS})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(DUILIB_ENABLE_SDL)
        # Enable SDL support for windows
        target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_SDL=1)
    endif()
endif()

if(NOT DUILIB_ENABLE_CEF)
    # 关闭CEF功能时，源码中不再包含CEF的头文件（见 duilib.cpp）
    target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_NO_CEF=1)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")  # Windows, WebView2
    target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_WEBVIEW2=1)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_USE_WEBVIEW2_LOADER_DLL=1)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # 由于Skia内部会自动开启AVX/AVX2等编译参数，duilib内部直接引用了部分源码，需要保持编译选项一致
    include(${DUILIB_SRC_ROOT_DIR}/cmake/duilib_compiles.cmake)

    # 编译器的参数（公共定义见 cmake/duilib_compiler_flags.cmake）
    include(${DUILIB_SRC_ROOT_DIR}/cmake/duilib_compiler_flags.cmake)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND DUILIB_CXX_COMPILER_FLAGS
            -Wno-cast-function-type
          )
          set_source_files_properties("${DUILIB_SRC_ROOT_DIR}/duilib/third_party/libudis86/decode.c" PROPERTIES COMPILE_OPTIONS "-Wno-implicit-fallthrough")
          if(DUILIB_HAVE_AVX2)
            set_source_files_properties("${SKIA_SRC_ROOT_DIR}/tools/window/GLWindowContext.cpp" PROPERTIES COMPILE_OPTIONS "-mavx2")
          elseif(DUILIB_HAVE_AVX)
            set_source_files_properties("${SKIA_SRC_ROOT_DIR}/tools/window/GLWindowContext.cpp" PROPERTIES COMPILE_OPTIONS "-mavx")
          endif()
    endif()

    target_compile_options(${PROJECT_NAME} PRIVATE ${DUILIB_COMPILER_FLAGS})
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>: ${DUILIB_CXX_COMPILER_FLAGS}>)
endif()
