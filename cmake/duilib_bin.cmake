if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

#启用C++20
set(CMAKE_CXX_STANDARD 20) # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++20

if(DUILIB_OS_WINDOWS)  # Windows平台
    if(MSVC)
        # MSVC 编译器
        set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)        
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # 对应/MT或/MTd
        add_compile_options("/utf-8")
    endif()
    add_definitions(-DUNICODE -D_UNICODE) # Windows 系统中使用Unicode编码
endif()

#添加源码，保存在变量SRC_FILES中
aux_source_directory(${DUILIB_PROJECT_SRC_DIR} SRC_FILES)

#设置项目的include路径（duilib）
include_directories(${DUILIB_SRC_ROOT_DIR})

if(DUILIB_ENABLE_CEF)
    #使用CEF模块：需要添加CEF源码根目录到include路径
    include_directories(${CEF_SRC_ROOT_DIR})
endif()

#设置项目的link路径
link_directories("${DUILIB_LIB_PATH}")  #duilib库所在路径(已经包含了cef_dll_wrapper这个CEF模块的库)
link_directories("${SKIA_LIB_PATH}")    #skia库所在路径
if(DUILIB_ENABLE_SDL)
    link_directories("${SDL_LIB_PATH}") #SDL库所在路径
endif()
if(DUILIB_ENABLE_CEF)
    link_directories("${CEF_LIB_PATH}") #cef库所在路径（.dll/.so对应的动态库）
endif()

#设置可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DUILIB_BIN_PATH}")

#MingGW-w64编译时的属性设置
if(DUILIB_MINGW AND DUILIB_OS_WINDOWS)
    #生成Windows程序，而不是控制台程序
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows ${CMAKE_EXE_LINKER_FLAGS}")
    
    if(DUILIB_MINGW_STATIC)
        # 使用静态链接
        set(CMAKE_EXE_LINKER_FLAGS "-static ${CMAKE_EXE_LINKER_FLAGS}")
    endif()
endif()

#设置编译可执行程序依赖的源码
add_executable(${PROJECT_NAME} ${SRC_FILES})

#设置链接可执行程序所依赖的库
if(DUILIB_OS_WINDOWS)
    if(DUILIB_MINGW)
        # MingGW-w64编译时的属性设置: 添加*.rc文件，并配置manifest文件
        if(DUILIB_BITS_64)
            set(DUILIB_WIN_MANIFEST "${DUILIB_SRC_ROOT_DIR}/MSVC/manifest/duilib.x64.manifest")
        else()
            set(DUILIB_WIN_MANIFEST "${DUILIB_SRC_ROOT_DIR}/MSVC/manifest/duilib.x86.manifest")
        endif()
        
        # 创建manifest.rc文件
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/${PROJECT_NAME}_manifest.rc" "1 24 \"${DUILIB_WIN_MANIFEST}\"")
        target_sources(${PROJECT_NAME} PRIVATE "${DUILIB_WINRES_FILE_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/${PROJECT_NAME}_manifest.rc")
    elseif()
        if(DUILIB_WINRES_FILE_NAME)
            target_sources(${PROJECT_NAME} PRIVATE "${DUILIB_WINRES_FILE_NAME}")
        endif()    
    endif()
    
    # Windows平台所依赖的库
    if(DUILIB_ENABLE_SDL)
        set(DUILIB_WINDOWS_LIBS Comctl32 Imm32 Opengl32 User32 shlwapi Version.lib Winmm.lib Setupapi.lib)
    else()
        set(DUILIB_WINDOWS_LIBS Comctl32 Imm32 Opengl32 User32 shlwapi)
    endif()
    target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${SDL_LIBS} ${SKIA_LIBS} ${CEF_LIBS} ${DUILIB_WINDOWS_LIBS})
endif()

if(DUILIB_OS_LINUX)
    # Linux平台
    target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${SDL_LIBS} ${SKIA_LIBS} ${CEF_LIBS} freetype fontconfig pthread dl)
endif()

if(DUILIB_OS_MACOS)
    # MacOS平台：查找必需框架
    find_library(ACCELERATE Accelerate)
    find_library(COREFOUNDATION CoreFoundation)
    find_library(CORETEXT CoreText)
    find_library(COREGRAPHICS CoreGraphics)
    # 字体配置
    find_package(Fontconfig REQUIRED)
    find_package(Freetype REQUIRED)
endif()

if(DUILIB_OS_MACOS)
    # MacOS平台：设置链接库（注意顺序！）
    target_link_libraries(${PROJECT_NAME}
                            # 第三方库（按依赖顺序）
                            ${DUILIB_LIBS} ${SDL_LIBS} ${SKIA_LIBS} Freetype::Freetype Fontconfig::Fontconfig
                            # 系统库
                            ${ACCELERATE} ${COREFOUNDATION} ${CORETEXT} ${COREGRAPHICS} pthread dl
                            # 显式框架声明（必须放在最后）
                            "-framework AppKit" "-framework Foundation" "-framework Metal"
                          )
endif()
