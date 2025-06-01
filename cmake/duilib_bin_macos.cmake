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

if(DUILIB_ENABLE_CEF)
    # CEF在MACOS的实现比较复杂，相关的代码单独放一个文件中
    include("duilib_cef_macos.cmake") 
else()
    #设置编译可执行程序依赖的源码
    add_executable(${PROJECT_NAME} ${SRC_FILES})
    
    # MacOS平台：设置链接库（注意顺序！）
    target_link_libraries(${PROJECT_NAME}
                            # 第三方库（按依赖顺序）
                            ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS}
                            # 系统库
                            ${ACCELERATE} ${COREFOUNDATION} ${CORETEXT} ${COREGRAPHICS} ${DUILIB_MACOS_LIBS}
                            # 显式框架声明（必须放在最后）
                            "-framework AppKit" "-framework Foundation" "-framework Metal"
                          )
endif()
