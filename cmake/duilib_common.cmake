# 操作系统：Windows,Linux,MacOS
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(DUILIB_OS_WINDOWS 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DUILIB_OS_LINUX 1)
elseif(APPLE)
    set(DUILIB_OS_MACOS 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(DUILIB_OS_FREEBSD 1)
    message(STATUS "Building for FreeBSD")
else()
    message(WARNING "Unknown OS: ${CMAKE_SYSTEM_NAME}")
endif()

# MINGW 开发环境
if(MINGW)
    set(DUILIB_MINGW 1)
    set(DUILIB_MINGW_NAME "mingw64-")
    
    # MingGW-w64编译时，使用静态链接
    option(DUILIB_MINGW_STATIC "MINGW Static Link" ON)
    
endif()

# 定义开关变量（修改后，需要清除camke生成目录才能生效，如果未清除则会从缓存中读取旧值）
option(DUILIB_LOG "Print duilib debug log" OFF)

# Skia的lib子目录名开关（默认值在下方"编译类型"之后计算，此处不声明）

# SDL功能：Windows平台默认不开启，其他平台默认开启
if(DUILIB_OS_WINDOWS)
    option(DUILIB_ENABLE_SDL "Enable SDL" OFF)
else()
    option(DUILIB_ENABLE_SDL "Enable SDL" ON)
endif()

# CEF功能：默认不开启，只有特定的项目开启
option(DUILIB_ENABLE_CEF "Enable CEF" OFF)

# CEF 109版本是否开启，默认不开启（CEF109版本兼容Win7系统，其他CEF版本只能运行在Win10及高版本操作系统）
option(DUILIB_CEF_109 "Enable CEF 109" OFF)

# WebView2控件的二进制
if(DUILIB_OS_WINDOWS)
    option(DUILIB_WEBVIEW2_EXE "Is Windows WebView2 exe" OFF)
endif()

#编译器类型: msvc,gcc,llvm
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(DUILIB_COMPILER_MSVC 1)
    set(DUILIB_COMPILER_NAME "msvc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(DUILIB_COMPILER_GCC 1)
    set(DUILIB_COMPILER_NAME "${DUILIB_MINGW_NAME}gcc")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(DUILIB_COMPILER_LLVM 1)
    set(DUILIB_COMPILER_NAME "${DUILIB_MINGW_NAME}llvm")
else()
    set(DUILIB_COMPILER_NAME "unknown")
    message(WARNING "Unknown CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
endif() 

# CPU类型
# Linux, MacOS: 只支持64位，没必要支持32位
# Windows：需要支持64位和32位，因32位系统仍有部分用户
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" lower_processor)
if(lower_processor MATCHES "armv7|arm")
    set(DUILIB_SYSTEM_PROCESSOR "arm32")
elseif(lower_processor MATCHES "aarch64|arm64")
    set(DUILIB_SYSTEM_PROCESSOR "arm64")
elseif(lower_processor MATCHES "i[3-6]86")
    set(DUILIB_SYSTEM_PROCESSOR "x86")
elseif(lower_processor MATCHES "x86_64|amd64")
    set(DUILIB_SYSTEM_PROCESSOR "x64")
else()
    set(DUILIB_SYSTEM_PROCESSOR, "unknown")
    message(WARNING "Unknown CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

#64位还是32位
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DUILIB_BITS_64 1)
endif()

#编译类型：Debug还是Release
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DUILIB_BUILD_TYPE "debug")
else()
    set(DUILIB_BUILD_TYPE "release")
endif()

#Skia的lib子目录名（默认值：Windows平台按 llvm.<CPU_ARCH>.<构建类型> 拼接；其他平台默认空，走下方自动拼接规则）
# 规则与 build/msvc_build.bat 保持一致：llvm.<CPU_ARCH>.release / llvm.<CPU_ARCH>.debug，
# 其中 CPU_ARCH 取 VS 目标架构（cmake -A 参数），缺省 x64，-A Win32 时为 x86。
# 仅在外部未指定时才使用默认值：-DDUILIB_SKIA_LIB_SUBPATH=... 传入时以外部值为准
# （set(... CACHE ...) 不会覆盖已存在的缓存变量，因此外部指定值优先）
set(DUILIB_SKIA_LIB_SUBPATH_DEFAULT "")
if(DUILIB_OS_WINDOWS)
    set(DUILIB_SKIA_CPU_ARCH "x64")
    if(CMAKE_GENERATOR_PLATFORM)
        if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(DUILIB_SKIA_CPU_ARCH "x86")
        else()
            set(DUILIB_SKIA_CPU_ARCH "${CMAKE_GENERATOR_PLATFORM}")
        endif()
    endif()
    set(DUILIB_SKIA_LIB_SUBPATH_DEFAULT "llvm.${DUILIB_SKIA_CPU_ARCH}.${DUILIB_BUILD_TYPE}")
endif()
set(DUILIB_SKIA_LIB_SUBPATH "${DUILIB_SKIA_LIB_SUBPATH_DEFAULT}" CACHE STRING "Skia lib sub path")

#dulib源码根目录，lib文件目录，bin文件目录
get_filename_component(DUILIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
set(DUILIB_LIB_PATH "${DUILIB_ROOT}/lib")
set(DUILIB_BIN_PATH "${DUILIB_ROOT}/bin")
set(DUILIB_LIBS duilib duilib-cximage duilib-webp duilib-png duilib-zlib)

#CEF模块的源码根目录（CEF模块为可选项）
if(DUILIB_ENABLE_CEF)
    if(DUILIB_OS_WINDOWS)
        # Winows平台
        if (DUILIB_CEF_109)
            #使用CEF 109版本
            set(DUILIB_CEF_SRC_ROOT_DIR "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_win_109")
            if(DUILIB_BITS_64)
                set(DUILIB_CEF_LIB_PATH "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_win_109/lib/x64")
            else()
                set(DUILIB_CEF_LIB_PATH "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_win_109/lib/Win32")
            endif()
            set(DUILIB_CEF_WRAPPER_LIB_NAME libcef_dll_wrapper_109)
        else()
            #使用CEF最新版本
            set(DUILIB_CEF_SRC_ROOT_DIR "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_win")
            if(DUILIB_BITS_64)
                set(DUILIB_CEF_LIB_PATH "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_win/lib/x64")
            else()
                set(DUILIB_CEF_LIB_PATH "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_win/lib/Win32")
            endif()
            set(DUILIB_CEF_WRAPPER_LIB_NAME libcef_dll_wrapper)
        endif()
    elseif(DUILIB_OS_LINUX)
        # Linux平台
        set(DUILIB_CEF_SRC_ROOT_DIR "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_linux")
        set(DUILIB_CEF_LIB_PATH "${DUILIB_ROOT}/bin/libcef_linux")
        set(DUILIB_CEF_WRAPPER_LIB_NAME cef_dll_wrapper)
    elseif(DUILIB_OS_MACOS)
        # MacOS平台
        set(DUILIB_CEF_SRC_ROOT_DIR "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_macos")
        set(DUILIB_CEF_LIB_PATH "")
        set(DUILIB_CEF_WRAPPER_LIB_NAME cef_dll_wrapper)
    endif()
    if(DUILIB_OS_WINDOWS)
        set(DUILIB_CEF_LIBS libcef ${DUILIB_CEF_WRAPPER_LIB_NAME})
    elseif(DUILIB_OS_MACOS)
        set(DUILIB_CEF_LIBS ${DUILIB_CEF_WRAPPER_LIB_NAME})
    else()
        set(DUILIB_CEF_LIBS libcef.so ${DUILIB_CEF_WRAPPER_LIB_NAME} X11)
    endif()
endif()

#Skia源码根目录，lib文件目录（Skia必选项）
get_filename_component(DUILIB_SKIA_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../skia/" ABSOLUTE)
if(DUILIB_SKIA_LIB_SUBPATH STREQUAL "" OR DUILIB_SKIA_LIB_SUBPATH STREQUAL "OFF")
    # 根据规则拼接
    set(DUILIB_SKIA_LIB_PATH "${DUILIB_SKIA_SRC_ROOT_DIR}/out/${DUILIB_COMPILER_NAME}.${DUILIB_SYSTEM_PROCESSOR}.${DUILIB_BUILD_TYPE}")
else()
    # 外部指定的子目录名
    set(DUILIB_SKIA_LIB_PATH "${DUILIB_SKIA_SRC_ROOT_DIR}/out/${DUILIB_SKIA_LIB_SUBPATH}")
endif()
set(DUILIB_SKIA_LIBS svg skshaper skottie sksg jsonreader skia)

#SDL库根目录，lib文件目录（Windows系统可选，其他平台必选）
if(DUILIB_ENABLE_SDL)
    get_filename_component(DUILIB_SDL_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../SDL3/" ABSOLUTE)
    if(EXISTS "${DUILIB_SDL_SRC_ROOT_DIR}/lib64/")
        set(DUILIB_SDL_LIB_PATH "${DUILIB_SDL_SRC_ROOT_DIR}/lib64")
    else()
        set(DUILIB_SDL_LIB_PATH "${DUILIB_SDL_SRC_ROOT_DIR}/lib")
    endif()
    if(DUILIB_OS_WINDOWS AND NOT MINGW)
        set(DUILIB_SDL_LIBS SDL3-static.lib)
    else()
        set(DUILIB_SDL_LIBS SDL3)
    endif()
endif()

#输出日志：打印变量数据
if(DUILIB_LOG)
    message(STATUS "DUILIB_PROJECT_SRC_DIR: ${DUILIB_PROJECT_SRC_DIR}")
    message(STATUS "PROJECT_NAME: ${PROJECT_NAME}")
    if(DUILIB_OS_WINDOWS)
        message(STATUS "DUILIB_WINRES_FILE_NAME: ${DUILIB_WINRES_FILE_NAME}")
    endif()
    message(STATUS "") 

    if(DUILIB_OS_WINDOWS)
        message(STATUS "DUILIB_OS: Windows")
    endif()
    if(DUILIB_OS_LINUX)
        message(STATUS "DUILIB_OS: Linux")
    endif()
    if(DUILIB_OS_MACOS)
        message(STATUS "DUILIB_OS: MacOS")
    endif()
    if(DUILIB_OS_FREEBSD)
        message(STATUS "DUILIB_OS: FreeBSD")
    endif()

    message(STATUS "DUILIB_COMPILER_NAME: ${DUILIB_COMPILER_NAME}")
    message(STATUS "DUILIB_SYSTEM_PROCESSOR: ${DUILIB_SYSTEM_PROCESSOR}")

    if(DUILIB_MINGW)
        message(STATUS "DUILIB_MINGW: ON")
        message(STATUS "DUILIB_MINGW_STATIC: ${DUILIB_MINGW_STATIC}")
    else()
        message(STATUS "DUILIB_MINGW: OFF")
    endif()

    if(DUILIB_BITS_64)
        message(STATUS "DUILIB_BITS_64: ON")
    else()
        message(STATUS "DUILIB_BITS_64: OFF")
    endif()

    message(STATUS "DUILIB_BUILD_TYPE: ${DUILIB_BUILD_TYPE}")
    
    if(MSVC)
        # MSVC 编译器：打印代码的运行库
        message(STATUS "CMAKE_MSVC_RUNTIME_LIBRARY: ${CMAKE_MSVC_RUNTIME_LIBRARY}") 
    endif()
    
    message(STATUS "") 
    
    message(STATUS "DUILIB_ROOT: ${DUILIB_ROOT}")
    message(STATUS "DUILIB_BIN_PATH: ${DUILIB_BIN_PATH}")
    message(STATUS "DUILIB_LIB_PATH: ${DUILIB_LIB_PATH}")    
    message(STATUS "DUILIB_LIBS: ${DUILIB_LIBS}") 
    message(STATUS "") 
    
    message(STATUS "DUILIB_SKIA_SRC_ROOT_DIR: ${DUILIB_SKIA_SRC_ROOT_DIR}")
    message(STATUS "DUILIB_SKIA_LIB_SUBPATH: ${DUILIB_SKIA_LIB_SUBPATH}")
    message(STATUS "DUILIB_SKIA_LIB_PATH: ${DUILIB_SKIA_LIB_PATH}")
    message(STATUS "DUILIB_SKIA_LIBS: ${DUILIB_SKIA_LIBS}")
    message(STATUS "") 
    
    message(STATUS "DUILIB_ENABLE_CEF: ${DUILIB_ENABLE_CEF}") 
    if (DUILIB_ENABLE_CEF) 
        message(STATUS "DUILIB_CEF_SRC_ROOT_DIR: ${DUILIB_CEF_SRC_ROOT_DIR}") 
        message(STATUS "DUILIB_CEF_LIB_PATH: ${DUILIB_CEF_LIB_PATH}") 
        message(STATUS "DUILIB_CEF_109: ${DUILIB_CEF_109}") 
        message(STATUS "DUILIB_CEF_LIBS: ${DUILIB_CEF_LIBS}")
    endif()
    message(STATUS "") 
    
    if(DUILIB_OS_WINDOWS)
        message(STATUS "DUILIB_WEBVIEW2_EXE: ${DUILIB_WEBVIEW2_EXE}") 
        message(STATUS "") 
    endif()
    
    message(STATUS "DUILIB_ENABLE_SDL: ${DUILIB_ENABLE_SDL}")
    if(DUILIB_ENABLE_SDL)        
        message(STATUS "DUILIB_SDL_SRC_ROOT_DIR: ${DUILIB_SDL_SRC_ROOT_DIR}")
        message(STATUS "DUILIB_SDL_LIB_PATH: ${DUILIB_SDL_LIB_PATH}")
        message(STATUS "DUILIB_SDL_LIBS: ${DUILIB_SDL_LIBS}")
    endif()
    message(STATUS "") 
endif()