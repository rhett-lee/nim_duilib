# DUILIB_OS_WINDOWS
if(NOT DUILIB_OS_WINDOWS)
    message(FATAL_ERROR "NOT DUILIB_OS_WINDOWS!")
endif()

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
    
    # 获取系统逻辑核心数
    cmake_host_system_information(RESULT CPU_CORES QUERY NUMBER_OF_LOGICAL_CORES)
    # 设置C/C++编译选项（开启多线程编译）
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/MP${CPU_CORES}>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/MP${CPU_CORES}>)
endif()

# Windows 系统中使用Unicode编码
add_definitions(-DUNICODE -D_UNICODE) 

if(MSVC)
    # MSVC需要单独设置Debug还是Release，否则它会自动创建子目录Debug/Release
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

#MingGW-w64编译时的属性设置
if(DUILIB_MINGW)
    #生成Windows程序，而不是控制台程序
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows ${CMAKE_EXE_LINKER_FLAGS}")
    
    if(DUILIB_MINGW_STATIC)
        # 使用静态链接
        set(CMAKE_EXE_LINKER_FLAGS "-static ${CMAKE_EXE_LINKER_FLAGS}")
    endif()
endif()

if(DUILIB_ENABLE_CEF)
    #使用CEF模块：需要添加CEF源码根目录到include路径
    include_directories(${DUILIB_CEF_SRC_ROOT_DIR})
    
    #cef库所在路径（.lib对应的路径）
    link_directories("${DUILIB_CEF_LIB_PATH}") 
endif()

# Remove *.mm
list(REMOVE_ITEM SRC_FILES ${DUILIB_PROJECT_SRC_DIR}/main_macos.mm)

#设置编译可执行程序依赖的源码
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 需要嵌入manifest文件路径
if(DUILIB_BITS_64)
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.x64.manifest")
else()
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.x86.manifest")
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

target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS} ${DUILIB_WINDOWS_LIBS})
