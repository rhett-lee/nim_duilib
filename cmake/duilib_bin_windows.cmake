# DUILIB_OS_WINDOWS
if(NOT DUILIB_OS_WINDOWS)
    message(FATAL_ERROR "NOT DUILIB_OS_WINDOWS!")
endif()

if(MSVC)
    # MSVC 编译器：设置代码的运行库
    if(DUILIB_MD)
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

# WebView2 的库文件与 DLL 所在目录（仅 Windows 平台有效）
# 目录名与 CPU 架构对应：x64 → native/x64，x86 → native/x86
set(DUILIB_WEBVIEW2_NATIVE_DIR "${DUILIB_ROOT}/duilib/third_party/Microsoft.Web.WebView2/build/native/${DUILIB_SYSTEM_PROCESSOR}")

# 需要嵌入的manifest文件路径
if(DUILIB_BITS_64)
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.x64.manifest")
else()
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.x86.manifest")
endif()
if(NOT EXISTS "${DUILIB_WIN_MANIFEST}")
    message(FATAL_ERROR "manifest file not found: ${DUILIB_WIN_MANIFEST}")
endif()

if(MSVC)
    # MSVC需要单独设置Debug还是Release，否则它会自动创建子目录Debug/Release
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

if(DUILIB_ENABLE_CEF)
    #使用CEF模块：需要添加CEF源码根目录到include路径
    list(APPEND DUILIB_TARGET_INCLUDE_DIRS "${DUILIB_CEF_SRC_ROOT_DIR}")
    
    #cef库所在路径（.lib对应的路径）
    list(APPEND DUILIB_TARGET_LINK_DIRS "${DUILIB_CEF_LIB_PATH}")
endif()

if(DUILIB_WEBVIEW2_EXE)    
    #WEBVIEW2库所在路径（.lib对应的路径）
    if(NOT EXISTS "${DUILIB_WEBVIEW2_NATIVE_DIR}")
        message(FATAL_ERROR "WebView2 native directory not found: ${DUILIB_WEBVIEW2_NATIVE_DIR}")
    endif()
    list(APPEND DUILIB_TARGET_LINK_DIRS "${DUILIB_WEBVIEW2_NATIVE_DIR}")
endif()

# Remove *.mm
list(REMOVE_ITEM SRC_FILES ${DUILIB_PROJECT_SRC_DIR}/main_macos.mm)

#设置编译可执行程序依赖的源码
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 把公共的include/link目录应用到当前目标（CM-17）
duilib_apply_target_directories()

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
    
    # 使用MSVC编译时，设置子系统和入口点（UNICODE下入口函数为wWinMain）
    target_link_options(${PROJECT_NAME} PRIVATE
        "/SUBSYSTEM:WINDOWS"
        "/ENTRY:wWinMainCRTStartup"
    )
    
    if(DUILIB_ENABLE_CEF)
        #设置libcef.dll延迟加载（需要同时链接 delayimp.lib 提供 __delayLoadHelper2）
        target_link_options(${PROJECT_NAME} PRIVATE
            "/DELAYLOAD:libcef.dll"  # 指定延迟加载的DLL文件名
            "/DEFAULTLIB:delayimp.lib"  # 提供 __delayLoadHelper2
        )
    endif()
elseif(DUILIB_MINGW)
    # MingGW-w64编译时的链接选项：生成Windows程序，而不是控制台程序
    target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
    
    if(DUILIB_MINGW_STATIC)
        # 使用静态链接（链接选项仅作用于当前目标，不影响其它目标）
        target_link_options(${PROJECT_NAME} PRIVATE -static)
    endif()
endif()

# Windows平台所依赖的库
set(DUILIB_WINDOWS_LIBS Comctl32 Imm32 Opengl32 User32 shlwapi)

if(DUILIB_ENABLE_SDL)
    # Enable SDL support for windows
    target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_SDL=1)
    
    # 添加SDL依赖的lib：除 Version/Winmm/Setupapi 外，SDL3 静态库在 Windows 上
    # 还直接引用 hid/mincore/dinput8 中的符号（见 SDL3staticTargets.cmake 的
    # INTERFACE_LINK_LIBRARIES），不补会导致 MinGW 链接出现 undefined reference
    list(APPEND DUILIB_WINDOWS_LIBS Version.lib Winmm.lib Setupapi.lib
                            Hid.lib Mincore.lib Dinput8.lib)
endif()

if(DUILIB_WEBVIEW2_EXE)
    # 定义宏，开启WebView2功能
    target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_WEBVIEW2=1)
    
    # 复制WebView2Loader.dll到bin目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD  # 在目标构建后执行
                       COMMAND ${CMAKE_COMMAND} -E copy
                               "${DUILIB_WEBVIEW2_NATIVE_DIR}/WebView2Loader.dll"
                               "$<TARGET_FILE_DIR:${PROJECT_NAME}>/WebView2Loader.dll"
                       COMMENT "Copying WebView2Loader.dll to runtime directory"
                      )
    
    # 添加WEBVIEW2依赖的lib
    list(APPEND DUILIB_WINDOWS_LIBS WinInet.lib WebView2Loader.dll.lib)
endif()

target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS} ${DUILIB_WINDOWS_LIBS})
