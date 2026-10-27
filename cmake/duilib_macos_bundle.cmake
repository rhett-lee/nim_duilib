# macOS App Bundle 支持（普通示例，不含 CEF）
#
# 由 cmake/duilib_bin_macos.cmake 在 DUILIB_EXAMPLE_MACOS_BUNDLE=ON 时 include，
# 并对目标调用 duilib_configure_macos_bundle()。
#
# 资源采用 macOS 标准 Bundle 布局：<App>.app/Contents/Resources/duilib/。
# GlobalManager::GetResourceRootPath() 在 macOS 下按运行环境自动检测该目录，
# 不再依赖调用参数，因此示例的 C++ 代码不需要修改（传 true/false 均可）。

# 在示例的 CMakeLists.txt 中，include duilib_example.cmake 之前可设置：
#   DUILIB_EXAMPLE_MACOS_BUNDLE   本示例是否编译为 .app（仅 macOS 生效）
#   DUILIB_MACOS_BUNDLE_IDENTIFIER  Bundle Identifier（默认 com.nimduilib.<目标名>）
#   DUILIB_MACOS_BUNDLE_NAME        Bundle 显示名称（默认目标名）
#   DUILIB_MACOS_BUNDLE_ICON        .icns 图标路径（绝对路径，或相对当前源码目录）

# 从 duilib_version.h 读取版本宏，生成 CFBundleVersion / CFBundleShortVersionString。
function(_duilib_macos_bundle_version out_version out_short)
    set(_header "${DUILIB_ROOT}/duilib/duilib_version.h")
    set(_major "")
    set(_minor "")
    set(_patch "")
    set(_build "")
    if(EXISTS "${_header}")
        file(STRINGS "${_header}" _lines
             REGEX "^[ \t]*#[ \t]*define[ \t]+DUILIB_VER_(MAJOR|MINOR|PATCH|BUILD)[ \t]+[0-9]+")
        foreach(_line IN LISTS _lines)
            if(_line MATCHES "DUILIB_VER_MAJOR[ \t]+([0-9]+)")
                set(_major "${CMAKE_MATCH_1}")
            elseif(_line MATCHES "DUILIB_VER_MINOR[ \t]+([0-9]+)")
                set(_minor "${CMAKE_MATCH_1}")
            elseif(_line MATCHES "DUILIB_VER_PATCH[ \t]+([0-9]+)")
                set(_patch "${CMAKE_MATCH_1}")
            elseif(_line MATCHES "DUILIB_VER_BUILD[ \t]+([0-9]+)")
                set(_build "${CMAKE_MATCH_1}")
            endif()
        endforeach()
    endif()

    if(_major STREQUAL "" OR _minor STREQUAL "" OR _patch STREQUAL "")
        set(_short "1.0")
        set(_version "1.0")
    else()
        set(_short "${_major}.${_minor}.${_patch}")
        if(_build STREQUAL "")
            set(_version "${_short}")
        else()
            set(_version "${_short}.${_build}")
        endif()
    endif()
    set(${out_version} "${_version}" PARENT_SCOPE)
    set(${out_short} "${_short}" PARENT_SCOPE)
endfunction()

function(duilib_configure_macos_bundle target)
    if(NOT DUILIB_OS_MACOS)
        return()
    endif()
    if(NOT TARGET ${target})
        message(FATAL_ERROR "duilib_configure_macos_bundle: target not found: ${target}")
    endif()

    _duilib_macos_bundle_version(_duilib_bundle_version _duilib_bundle_short)

    set(_duilib_bundle_identifier "com.nimduilib.${target}")
    if(DEFINED DUILIB_MACOS_BUNDLE_IDENTIFIER AND
       NOT "${DUILIB_MACOS_BUNDLE_IDENTIFIER}" STREQUAL "")
        set(_duilib_bundle_identifier "${DUILIB_MACOS_BUNDLE_IDENTIFIER}")
    endif()

    set(_duilib_bundle_name "${target}")
    if(DEFINED DUILIB_MACOS_BUNDLE_NAME AND
       NOT "${DUILIB_MACOS_BUNDLE_NAME}" STREQUAL "")
        set(_duilib_bundle_name "${DUILIB_MACOS_BUNDLE_NAME}")
    endif()

    set(_duilib_bundle_icon_path "")
    set(_duilib_bundle_icon_name "")
    if(DEFINED DUILIB_MACOS_BUNDLE_ICON AND
       NOT "${DUILIB_MACOS_BUNDLE_ICON}" STREQUAL "")
        set(_duilib_bundle_icon_path "${DUILIB_MACOS_BUNDLE_ICON}")
        if(NOT IS_ABSOLUTE "${_duilib_bundle_icon_path}")
            set(_duilib_bundle_icon_path
                "${CMAKE_CURRENT_SOURCE_DIR}/${_duilib_bundle_icon_path}")
        endif()
        if(EXISTS "${_duilib_bundle_icon_path}")
            get_filename_component(_duilib_bundle_icon_name
                                   "${_duilib_bundle_icon_path}" NAME)
        else()
            message(WARNING
                "DUILIB_MACOS_BUNDLE_ICON not found: ${_duilib_bundle_icon_path}")
            set(_duilib_bundle_icon_path "")
        endif()
    endif()

    # 版本/标识/Info.plist 模板。模板见 cmake/macos/Info.plist.in，
    # 使用 CMake 标准的 MACOSX_BUNDLE_* 变量。
    set_target_properties(${target} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "${_duilib_bundle_name}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "${_duilib_bundle_identifier}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${_duilib_bundle_version}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${_duilib_bundle_short}"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${_duilib_bundle_version}"
        MACOSX_BUNDLE_INFO_STRING "${_duilib_bundle_name} ${_duilib_bundle_short}"
        MACOSX_BUNDLE_COPYRIGHT "nim_duilib"
        MACOSX_BUNDLE_INFO_PLIST
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/macos/Info.plist.in"
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
    if(_duilib_bundle_icon_name)
        set_target_properties(${target} PROPERTIES
            MACOSX_BUNDLE_ICON_FILE "${_duilib_bundle_icon_name}")
    endif()

    # 资源复制到 Contents/Resources/duilib：macOS 标准 Bundle 布局，
    # 与 GetResourceRootPath() 的自动检测逻辑一致。
    set(_duilib_resources_from "${DUILIB_ROOT}/bin/resources")
    if(NOT EXISTS "${_duilib_resources_from}")
        message(FATAL_ERROR
            "macOS bundle resources not found: ${_duilib_resources_from}\n"
            "Restore bin/resources in the source tree before building a bundle.")
    endif()
    set(_duilib_resources_to
        "$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources/duilib")
    add_custom_command(TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_duilib_resources_to}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${_duilib_resources_from}" "${_duilib_resources_to}"
        COMMENT "Copying duilib resources into ${target}.app"
        VERBATIM)

    # 切换为 Bundle 后，bin/ 中可能还留有上一次生成的非 Bundle 可执行文件；
    # 删除它，避免 pack.py 同时收集 bin/<target> 与 bin/<target>.app。
    add_custom_command(TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -f "${DUILIB_BIN_PATH}/${target}"
        COMMENT "Removing stale non-bundle executable: ${target}"
        VERBATIM)

    # 可选图标：只复制进 Contents/Resources，Info.plist 引用文件名。
    if(_duilib_bundle_icon_path)
        add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_duilib_bundle_icon_path}"
                    "$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources/${_duilib_bundle_icon_name}"
            COMMENT "Copying ${_duilib_bundle_icon_name} into ${target}.app"
            VERBATIM)
    endif()

    # SDL3 在 macOS 上是动态库：复制到 Contents/Frameworks，并配合上面的
    # @executable_path/../Frameworks rpath。静态 SDL3 时脚本会直接跳过。
    if(DUILIB_ENABLE_SDL)
        add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                    "-DSDL3_LIB_DIR=${DUILIB_SDL_LIB_PATH}"
                    "-DBUNDLE_FRAMEWORKS_DIR=$<TARGET_BUNDLE_DIR:${target}>/Contents/Frameworks"
                    -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/duilib_copy_sdl_bundle.cmake"
            COMMENT "Copying SDL3 runtime into ${target}.app"
            VERBATIM)
    endif()
endfunction()
