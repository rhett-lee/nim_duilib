# nim_duilib SDK 运行期复制辅助函数（由 nim_duilibConfig.cmake 自动 include）
#
# 用法：
#   nim_duilib_copy_runtime(my_app)
#   nim_duilib_copy_runtime(my_app RESOURCES)
#   nim_duilib_copy_runtime(my_app DESTINATION "${CMAKE_BINARY_DIR}/bin")
#   nim_duilib_copy_runtime(my_app RESOURCES DESTINATION "${CMAKE_BINARY_DIR}/bin")
#
# - 默认把 SDK 的 bin/*.dll 复制到可执行文件同级目录；
# - RESOURCES 时额外把 SDK 的 bin/resources/ 复制到同级 resources/；
# - DESTINATION 可覆盖目标目录（多目标共享输出目录时使用）。

function(nim_duilib_copy_runtime target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "nim_duilib_copy_runtime: 目标不存在：${target}")
    endif()

    cmake_parse_arguments(_nim_copy "RESOURCES" "DESTINATION" "" ${ARGN})
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

    if(_nim_copy_RESOURCES)
        if(NOT EXISTS "${_nim_root}/bin/resources")
            message(WARNING
                "nim_duilib_copy_runtime: SDK 中没有 bin/resources 目录：${_nim_root}")
        else()
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${_nim_root}/bin/resources"
                        "${_nim_dest}/resources"
                COMMENT "Copying nim_duilib resources")
        endif()
    endif()
endfunction()
