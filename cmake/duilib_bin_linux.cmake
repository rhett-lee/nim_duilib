# DUILIB_OS_LINUX
if(NOT DUILIB_OS_LINUX)
    message(FATAL_ERROR "NOT DUILIB_OS_LINUX!")
endif()

if(DUILIB_ENABLE_CEF)
    #使用CEF模块：需要添加CEF源码根目录到include路径
    include_directories(${DUILIB_CEF_SRC_ROOT_DIR})
    
    #cef库所在路径（.a/.so对应的动态库路径）
    link_directories("${DUILIB_CEF_LIB_PATH}") 
endif()

# Remove *.mm
list(REMOVE_ITEM SRC_FILES ${DUILIB_PROJECT_SRC_DIR}/main_macos.mm)

#设置编译可执行程序依赖的源码
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 平台的标准库
set(DUILIB_LINUX_LIBS X11 freetype fontconfig pthread dl)

# Linux平台所依赖的库
target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS} ${DUILIB_LINUX_LIBS})
