# DUILIB_OS_FREEBSD
if(NOT DUILIB_OS_FREEBSD)
    message(FATAL_ERROR "NOT DUILIB_OS_FREEBSD!")
endif()

# Remove *.mm
list(REMOVE_ITEM SRC_FILES ${DUILIB_PROJECT_SRC_DIR}/main_macos.mm)

#设置编译可执行程序依赖的源码
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 平台的标准库
set(DUILIB_FREEBSD_LIBS pthread dl)

#Skia源码根目录，lib文件目录（Skia必选项）
get_filename_component(DUILIB_SKIA_SRC_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../skia/" ABSOLUTE)
if(DUILIB_SKIA_LIB_SUBPATH STREQUAL "" OR DUILIB_SKIA_LIB_SUBPATH STREQUAL "OFF")
    # 根据规则拼接
    set(DUILIB_SKIA_LIB_PATH "${DUILIB_SKIA_SRC_ROOT_DIR}/out/${DUILIB_COMPILER_NAME}.${DUILIB_SYSTEM_PROCESSOR}.${DUILIB_BUILD_TYPE}")
    message(STATUS "DUILIB_SKIA_LIB_PATH: ${DUILIB_SKIA_LIB_PATH}")
    message(STATUS "DUILIB_SKIA_SRC_ROOT_DIR: ${DUILIB_SKIA_SRC_ROOT_DIR}")
else()
    # 外部指定的子目录名
    set(DUILIB_SKIA_LIB_PATH "${DUILIB_SKIA_SRC_ROOT_DIR}/out/${DUILIB_SKIA_LIB_SUBPATH}")
endif()
set(DUILIB_SKIA_LIBS libsvg.a libskia.a libskshaper.a)

find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(X11 REQUIRED)

# FreeBSD平台所依赖的库
target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS}  ${DUILIB_FREEBSD_LIBS} ${X11_LIBRARIES} Freetype::Freetype Fontconfig::Fontconfig)


