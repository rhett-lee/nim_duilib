if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

#启用C++20
set(CMAKE_CXX_STANDARD 20) # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++20

#输出基本信息
message(STATUS "PROJECT_NAME: ${PROJECT_NAME}")
message(STATUS "C compiler  : ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

if(DUILIB_OS_WINDOWS)  # Windows平台
    if(MSVC)
        # MSVC 编译器：设置代码的运行库
        if("${DUILIB_MD}" STREQUAL "ON")
            # MD/MDd
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")
        else()
            # MT/MTd
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
        
        # 源码文件使用UTF8编码
        add_compile_options("/utf-8")
    endif()
    add_definitions(-DUNICODE -D_UNICODE) # Windows 系统中使用Unicode编码
endif()

#设置项目的include路径（duilib）
include_directories(${DUILIB_SRC_ROOT_DIR})

#设置该程序的根目录到include路径
include_directories(${DUILIB_PROJECT_SRC_DIR})

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
    link_directories("${CEF_LIB_PATH}") #cef库所在路径（.lib/.so对应的动态库）
endif()

#设置可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DUILIB_BIN_PATH}")
if(MSVC)
    # MSVC需要单独设置Debug还是Release，否则它会自动创建子目录Debug/Release
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

#MingGW-w64编译时的属性设置
if(DUILIB_MINGW AND DUILIB_OS_WINDOWS)
    #生成Windows程序，而不是控制台程序
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows ${CMAKE_EXE_LINKER_FLAGS}")
    
    if(DUILIB_MINGW_STATIC)
        # 使用静态链接
        set(CMAKE_EXE_LINKER_FLAGS "-static ${CMAKE_EXE_LINKER_FLAGS}")
    endif()
endif()

#添加源码，保存在变量SRC_FILES中
aux_source_directory(${DUILIB_PROJECT_SRC_DIR} SRC_FILES)

#追加子目录的源代码文件
if(DUILIB_SRC_SUB_DIRS)
    foreach(ITEM IN LISTS DUILIB_SRC_SUB_DIRS)
        # 逐个子目录添加
        aux_source_directory("${DUILIB_PROJECT_SRC_DIR}/${ITEM}" SUB_DIR_SRC_FILES)
        list(APPEND SRC_FILES ${SUB_DIR_SRC_FILES})  # 合并列表
    endforeach()
endif()

#设置编译可执行程序依赖的源码
add_executable(${PROJECT_NAME} ${SRC_FILES})

#设置链接可执行程序所依赖的库
if(DUILIB_OS_WINDOWS)
    # 需要嵌入manifest文件路径
    if(DUILIB_BITS_64)
        set(DUILIB_WIN_MANIFEST "${DUILIB_SRC_ROOT_DIR}/msvc/manifest/duilib.x64.manifest")
    else()
        set(DUILIB_WIN_MANIFEST "${DUILIB_SRC_ROOT_DIR}/msvc/manifest/duilib.x86.manifest")
    endif()
         
    if(DUILIB_MINGW)
        # MingGW-w64编译时的属性设置: 添加*.rc文件，并配置manifest文件
        # 创建manifest.rc文件
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/${PROJECT_NAME}_manifest.rc" "1 24 \"${DUILIB_WIN_MANIFEST}\"")
        target_sources(${PROJECT_NAME} PRIVATE "${DUILIB_WINRES_FILE_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/${PROJECT_NAME}_manifest.rc")
    elseif(DUILIB_WINRES_FILE_NAME)
        # 添加程序的rc文件
        target_sources(${PROJECT_NAME} PRIVATE "${DUILIB_WINRES_FILE_NAME}")
    endif()
    
    if(MSVC)
        # 配置manifest文件
        target_sources(${PROJECT_NAME} PRIVATE 
            ${DUILIB_WIN_MANIFEST}
        )
        
        # 使用MSVC编译时，需要设置子系统属性
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup"
        )
        
        if(DUILIB_ENABLE_CEF)
            #设置libcef.dll延迟加载
            target_link_options(${PROJECT_NAME} PRIVATE
                "/DELAYLOAD:libcef.dll"  # 指定延迟加载的DLL文件名
            )
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
endif()

if(DUILIB_OS_MACOS)
    # MacOS平台：设置链接库（注意顺序！）
    target_link_libraries(${PROJECT_NAME}
                            # 第三方库（按依赖顺序）
                            ${DUILIB_LIBS} ${SDL_LIBS} ${SKIA_LIBS}
                            # 系统库
                            ${ACCELERATE} ${COREFOUNDATION} ${CORETEXT} ${COREGRAPHICS} pthread dl
                            # 显式框架声明（必须放在最后）
                            "-framework AppKit" "-framework Foundation" "-framework Metal"
                          )
endif()
