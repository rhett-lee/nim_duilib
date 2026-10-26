# DUILIB_OS_FREEBSD
if(NOT DUILIB_OS_FREEBSD)
    message(FATAL_ERROR "NOT DUILIB_OS_FREEBSD!")
endif()

# Remove *.mm
list(REMOVE_ITEM SRC_FILES ${DUILIB_PROJECT_SRC_DIR}/main_macos.mm)

#设置编译可执行程序依赖的源码
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 把公共的include/link目录应用到当前目标（CM-17）
duilib_apply_target_directories()

# 平台的标准库（字体相关依赖统一使用 imported target，见 duilib_unix_font_libs.cmake）
set(DUILIB_FREEBSD_LIBS pthread dl)

include("${CMAKE_CURRENT_LIST_DIR}/duilib_unix_font_libs.cmake")
find_package(X11 REQUIRED)

# FreeBSD平台所依赖的库
target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS}  ${DUILIB_FREEBSD_LIBS} X11::X11 ${DUILIB_FONT_LIBS})
# 记录本目标的构建配置，供 pack.py 判断 bin/ 是否混有另一配置的同名程序
duilib_add_build_config_stamp(${PROJECT_NAME})


