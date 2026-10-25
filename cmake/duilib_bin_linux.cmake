# DUILIB_OS_LINUX
if(NOT DUILIB_OS_LINUX)
    message(FATAL_ERROR "NOT DUILIB_OS_LINUX!")
endif()

# 仅 CEF 示例（cef/CefBrowser）链接 CEF；其他示例不应产生 libcef.so 依赖
set(DUILIB_EXAMPLE_CEF_LIBS "")
if(DUILIB_ENABLE_CEF AND DUILIB_EXAMPLE_CEF)
    #使用CEF模块：需要添加CEF源码根目录到include路径
    list(APPEND DUILIB_TARGET_INCLUDE_DIRS "${DUILIB_CEF_SRC_ROOT_DIR}")
    
    #cef库所在路径（.a/.so对应的动态库路径）
    list(APPEND DUILIB_TARGET_LINK_DIRS "${DUILIB_CEF_LIB_PATH}")
    set(DUILIB_EXAMPLE_CEF_LIBS ${DUILIB_CEF_LIBS})
endif()

# Remove *.mm
list(REMOVE_ITEM SRC_FILES ${DUILIB_PROJECT_SRC_DIR}/main_macos.mm)

#设置编译可执行程序依赖的源码
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 把公共的include/link目录应用到当前目标（CM-17）
duilib_apply_target_directories()

# 平台的标准库（字体相关依赖统一使用 imported target，见 duilib_unix_font_libs.cmake）
include("${CMAKE_CURRENT_LIST_DIR}/duilib_unix_font_libs.cmake")
find_package(X11 REQUIRED)
set(DUILIB_LINUX_LIBS X11::X11 ${DUILIB_FONT_LIBS} pthread dl)

# Linux平台所依赖的库
target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_EXAMPLE_CEF_LIBS} ${DUILIB_LINUX_LIBS})
