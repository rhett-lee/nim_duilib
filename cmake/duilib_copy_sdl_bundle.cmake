# 把 SDL3 动态库复制到 macOS App Bundle 的 Contents/Frameworks 目录。
#
# 用法（由 cmake/duilib_cef_macos.cmake 作为 POST_BUILD 调用）：
#   cmake -DSDL3_LIB_DIR=<SDL3 的 lib64 或 lib 目录>
#         -DBUNDLE_FRAMEWORKS_DIR=<App>.app/Contents/Frameworks
#         -P cmake/duilib_copy_sdl_bundle.cmake
#
# 之所以在构建期执行（-P 脚本内 glob），而不是 configure 期展开文件列表：
# DUILIB_DEPS_MODE=auto 时 SDL3 由 duilib_deps 目标在构建阶段才安装出来，
# configure 阶段还看不到 dylib；此时展开会得到空列表。
#
# FOLLOW_SYMLINK_CHAIN 会把 libSDL3.dylib -> libSDL3.<abi>.dylib 这类版本链接链
# 一并还原到目标目录，可执行文件无论记录的是哪个名字都能被 dyld 找到。

if(NOT SDL3_LIB_DIR)
    message(FATAL_ERROR "duilib_copy_sdl_bundle: 缺少 SDL3_LIB_DIR")
endif()
if(NOT BUNDLE_FRAMEWORKS_DIR)
    message(FATAL_ERROR "duilib_copy_sdl_bundle: 缺少 BUNDLE_FRAMEWORKS_DIR")
endif()

file(GLOB _duilib_sdl3_dylibs "${SDL3_LIB_DIR}/libSDL3*.dylib")
if(NOT _duilib_sdl3_dylibs)
    message(FATAL_ERROR
        "未找到 SDL3 动态库：${SDL3_LIB_DIR}/libSDL3*.dylib\n"
        "请先执行 'build.py deps'，或传入 -DDUILIB_SDL_ROOT=<SDL3 安装前缀>。")
endif()

file(MAKE_DIRECTORY "${BUNDLE_FRAMEWORKS_DIR}")
foreach(_duilib_sdl3_dylib IN LISTS _duilib_sdl3_dylibs)
    file(COPY "${_duilib_sdl3_dylib}"
         DESTINATION "${BUNDLE_FRAMEWORKS_DIR}"
         FOLLOW_SYMLINK_CHAIN
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endforeach()

# 正常通过 CMake 安装的 SDL3，install_name 是 @rpath/libSDL3*.dylib，配合
# duilib_cef_macos.cmake 设置的 rpath 即可加载。若 install_name 是构建机绝对
# 路径（例如手工编译时关闭了 MACOSX_RPATH），复制进 Bundle 也不会被使用，
# 这里只提示修正方法，由构建者用 install_name_tool 处理。
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    list(GET _duilib_sdl3_dylibs 0 _duilib_sdl3_primary)
    execute_process(COMMAND otool -D "${_duilib_sdl3_primary}"
                    OUTPUT_VARIABLE _duilib_sdl3_id_output
                    ERROR_QUIET)
    string(REGEX REPLACE "^[^\n]*\n" "" _duilib_sdl3_install_name
           "${_duilib_sdl3_id_output}")
    string(STRIP "${_duilib_sdl3_install_name}" _duilib_sdl3_install_name)
    if(_duilib_sdl3_install_name AND
       NOT _duilib_sdl3_install_name MATCHES "^@rpath/")
        message(WARNING
            "SDL3 的 install_name 不是 @rpath（${_duilib_sdl3_install_name}）："
            "复制进 Bundle 后仍会按绝对路径加载。请重新安装带 MACOSX_RPATH 的 "
            "SDL3，或对该 dylib 执行 install_name_tool -id @rpath/<文件名>，"
            "并对 .app 内的可执行文件执行 install_name_tool -change。")
    endif()
endif()
