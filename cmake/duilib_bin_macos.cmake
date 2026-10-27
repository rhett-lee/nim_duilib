# DUILIB_OS_MACOS
if(NOT DUILIB_OS_MACOS)
    message(FATAL_ERROR "NOT DUILIB_OS_MACOS!")
endif()

# MacOS平台：查找必需框架
find_library(ACCELERATE Accelerate)
find_library(COREFOUNDATION CoreFoundation)
find_library(CORETEXT CoreText)
find_library(COREGRAPHICS CoreGraphics)

# 平台的标准库
set(DUILIB_MACOS_LIBS pthread dl)

# 仅 CEF 示例（cef/CefBrowser）走 Bundle + CEF Framework 流程；
# 其他示例默认编译为普通可执行文件，可通过 DUILIB_EXAMPLE_MACOS_BUNDLE=ON
# 单独改为 macOS App Bundle，且不链接 CEF。
if(DUILIB_ENABLE_CEF AND DUILIB_EXAMPLE_CEF)
    # CEF在MACOS的实现比较复杂，相关的代码单独放一个文件中
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_cef_macos.cmake") 
else()
    # 保持编译器的参数与CEF内部设置一致（公共定义见 cmake/duilib_compiler_flags.cmake）
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_compiler_flags.cmake")

    set(_duilib_macos_bundle FALSE)
    if(DUILIB_EXAMPLE_MACOS_BUNDLE)
        set(_duilib_macos_bundle TRUE)
    endif()

    #设置编译可执行程序依赖的源码
    if(_duilib_macos_bundle)
        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SRC_FILES})
    else()
        add_executable(${PROJECT_NAME} ${SRC_FILES})
    endif()
    
    # 把公共的include/link目录应用到当前目标（CM-17）
    duilib_apply_target_directories()
    
    #设置编译器的参数
    target_compile_options(${PROJECT_NAME} PRIVATE ${DUILIB_COMPILER_FLAGS} ${DUILIB_CXX_COMPILER_FLAGS})
    
    # MacOS平台：设置链接库（注意顺序！）
    target_link_libraries(${PROJECT_NAME}
                            # 第三方库（按依赖顺序）
                            ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS}
                            # 系统库
                            ${ACCELERATE} ${COREFOUNDATION} ${CORETEXT} ${COREGRAPHICS} ${DUILIB_MACOS_LIBS}
                            # 显式框架声明（必须放在最后）
                            "-framework AppKit" "-framework Foundation" "-framework Metal" "-framework Cocoa"
                          )

    # Bundle 属性、资源、SDL3 动态库与 rpath（不修改示例 C++ 代码）
    if(_duilib_macos_bundle)
        include("${CMAKE_CURRENT_LIST_DIR}/duilib_macos_bundle.cmake")
        duilib_configure_macos_bundle(${PROJECT_NAME})
    else()
        # 从 Bundle 切回普通可执行文件时，清掉上一次生成的 .app，避免
        # pack.py 同时收集 bin/<目标> 与 bin/<目标>.app。
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${DUILIB_BIN_PATH}/${PROJECT_NAME}.app"
            COMMENT "Removing stale app bundle: ${PROJECT_NAME}.app"
            VERBATIM)
    endif()
endif()

# 记录本目标的构建配置，供 pack.py 判断 bin/ 是否混有另一配置的同名程序
duilib_add_build_config_stamp(${PROJECT_NAME})
