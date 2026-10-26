# nim_duilib SDK 运行期复制辅助函数（由 nim_duilibConfig.cmake 自动 include）
#
# 用法：
#   nim_duilib_copy_runtime(my_app)
#   nim_duilib_copy_runtime(my_app RESOURCES)
#   nim_duilib_copy_runtime(my_app DESTINATION "${CMAKE_BINARY_DIR}/bin")
#   nim_duilib_copy_runtime(my_app RESOURCES DESTINATION "${CMAKE_BINARY_DIR}/bin")
#   nim_duilib_copy_runtime(my_app FRAMEWORKS)          # 仅 macOS App Bundle
#   nim_duilib_copy_runtime(my_app RESOURCES FRAMEWORKS)
#
# - 默认把 SDK 的 bin/*.dll 复制到可执行文件同级目录；
# - RESOURCES 时额外把 SDK 的 bin/resources/ 复制到同级 resources/；
#   目标是 macOS 的 MACOSX_BUNDLE 且未指定 DESTINATION 时，改复制到
#   <target>.app/Contents/Resources/duilib/，与 GlobalManager::GetResourceRootPath(true)
#   的查找位置一致；
# - DESTINATION 可覆盖目标目录（多目标共享输出目录时使用）；
# - FRAMEWORKS（仅 macOS）要求目标为 MACOSX_BUNDLE：把 SDK 的
#   lib/libSDL3*.dylib 复制到 <target>.app/Contents/Frameworks，并把可执行文件的
#   rpath 设为 @executable_path/../Frameworks（BUILD_WITH_INSTALL_RPATH，
#   消费者已显式设置的 BUILD_RPATH/INSTALL_RPATH 条目保留在 Frameworks 之后）。
#   因此请在调用本函数之前设置消费者的 rpath 属性；之后再改不会被合并。
#   Helper 的 exe 位于 Contents/Frameworks/<Helper>.app/Contents/MacOS，需要自行
#   使用 @executable_path/../../..。

function(nim_duilib_copy_runtime target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "nim_duilib_copy_runtime: 目标不存在：${target}")
    endif()

    cmake_parse_arguments(_nim_copy "RESOURCES;FRAMEWORKS" "DESTINATION" "" ${ARGN})
    if(_nim_copy_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "nim_duilib_copy_runtime: 无法识别的参数：${_nim_copy_UNPARSED_ARGUMENTS}")
    endif()

    get_property(_nim_root GLOBAL PROPERTY NIM_DUILIB_ROOT)
    get_property(_nim_dlls GLOBAL PROPERTY NIM_DUILIB_RUNTIME_DLLS)
    if(NOT _nim_root)
        message(FATAL_ERROR
            "nim_duilib_copy_runtime: 未找到 SDK 元数据，请先 find_package(nim_duilib)。")
    endif()

    if(_nim_copy_DESTINATION)
        set(_nim_dest "${_nim_copy_DESTINATION}")
    else()
        set(_nim_dest "$<TARGET_FILE_DIR:${target}>")
    endif()

    foreach(_nim_dll IN LISTS _nim_dlls)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_nim_root}/bin/${_nim_dll}"
                    "${_nim_dest}/${_nim_dll}"
            COMMENT "Copying nim_duilib runtime: ${_nim_dll}")
    endforeach()

    if(_nim_copy_FRAMEWORKS)
        if(NOT APPLE)
            message(FATAL_ERROR
                "nim_duilib_copy_runtime: FRAMEWORKS 仅适用于 macOS 的 App Bundle")
        endif()

        get_target_property(_nim_is_bundle ${target} MACOSX_BUNDLE)
        if(NOT _nim_is_bundle)
            message(FATAL_ERROR
                "nim_duilib_copy_runtime: FRAMEWORKS 要求 ${target} 是 MACOSX_BUNDLE，"
                "请用 add_executable(${target} MACOSX_BUNDLE ...) 创建目标。")
        endif()

        get_property(_nim_sdl_dylibs GLOBAL PROPERTY NIM_DUILIB_SDL_DYLIBS)
        if(NOT _nim_sdl_dylibs)
            message(WARNING
                "nim_duilib_copy_runtime: SDK 没有 SDL3 动态库（静态 SDL 或无 SDL），"
                "FRAMEWORKS 没有可复制的内容：${_nim_root}")
        else()
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                        "$<TARGET_BUNDLE_CONTENT_DIR:${target}>/Frameworks"
                COMMENT "Preparing ${target}.app/Contents/Frameworks")

            foreach(_nim_dylib IN LISTS _nim_sdl_dylibs)
                # 压缩包解压到不支持符号链接的环境时，链接名可能缺失；跳过并告警，
                # 不影响另一个实体文件名的复制。
                if(NOT EXISTS "${_nim_root}/lib/${_nim_dylib}")
                    message(WARNING
                        "nim_duilib_copy_runtime: SDK 中缺少 ${_nim_dylib}，"
                        "已跳过：${_nim_root}/lib")
                    continue()
                endif()
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${_nim_root}/lib/${_nim_dylib}"
                            "$<TARGET_BUNDLE_CONTENT_DIR:${target}>/Frameworks/${_nim_dylib}"
                    COMMENT "Copying nim_duilib SDL3 runtime: ${_nim_dylib}")
            endforeach()

            # 主可执行文件位于 Contents/MacOS，rpath 必须优先指向自身的 Frameworks：
            # CMake 自动生成的构建树 rpath（SDK 的 lib/ 绝对路径）会排在 BUILD_RPATH
            # 前面；若只是追加，复制到别处或换机器运行时可能先命中 SDK 里的同名旧库。
            # 因此这里统一改用 INSTALL_RPATH，并把消费者显式设置的 BUILD_RPATH /
            # INSTALL_RPATH 条目合并到 Frameworks 之后。
            # Helper 的 exe 在 Contents/Frameworks/<Helper>.app/Contents/MacOS，
            # 需自行设置 @executable_path/../../..。
            get_target_property(_nim_prev_install_rpath ${target} INSTALL_RPATH)
            get_target_property(_nim_prev_build_rpath ${target} BUILD_RPATH)
            set(_nim_bundle_rpath "@executable_path/../Frameworks")
            foreach(_nim_rpath IN LISTS _nim_prev_install_rpath _nim_prev_build_rpath)
                if(_nim_rpath AND NOT _nim_rpath MATCHES "-NOTFOUND$")
                    list(APPEND _nim_bundle_rpath "${_nim_rpath}")
                endif()
            endforeach()
            list(REMOVE_DUPLICATES _nim_bundle_rpath)
            set_target_properties(${target} PROPERTIES
                BUILD_WITH_INSTALL_RPATH TRUE
                INSTALL_RPATH "${_nim_bundle_rpath}")
        endif()
    endif()

    if(_nim_copy_RESOURCES)
        if(NOT EXISTS "${_nim_root}/bin/resources")
            message(WARNING
                "nim_duilib_copy_runtime: SDK 中没有 bin/resources 目录：${_nim_root}")
        else()
            set(_nim_res_dest "${_nim_dest}/resources")
            if(APPLE AND NOT _nim_copy_DESTINATION)
                # App Bundle 遵循 bundle 约定：GlobalManager::GetResourceRootPath(true)
                # 查找 Contents/Resources/duilib/，而不是可执行文件同级的 resources/。
                get_target_property(_nim_res_is_bundle ${target} MACOSX_BUNDLE)
                if(_nim_res_is_bundle)
                    set(_nim_res_dest
                        "$<TARGET_BUNDLE_CONTENT_DIR:${target}>/Resources/duilib")
                endif()
            endif()
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${_nim_root}/bin/resources"
                        "${_nim_res_dest}"
                COMMENT "Copying nim_duilib resources")
        endif()
    endif()
endfunction()
