# macOS App Bundle 支持（普通示例，不含 CEF）
#
# 由 cmake/duilib_bin_macos.cmake 在 DUILIB_EXAMPLE_MACOS_BUNDLE=ON 时 include，
# 并对目标调用 duilib_configure_macos_bundle()。
#
# 资源采用 macOS 标准 Bundle 布局：<App>.app/Contents/Resources/duilib/resources/，
# 与普通程序的 <exe 目录>/resources/ 保持同构；zip 模式对应
# <App>.app/Contents/Resources/duilib/resources.zip。
# GlobalManager::GetResourceRootPath(true) 会定位该目录，示例的 C++ 代码不需要修改。

# 在示例的 CMakeLists.txt 中，include duilib_example.cmake 之前可设置：
#   DUILIB_EXAMPLE_MACOS_BUNDLE   本示例是否编译为 .app（仅 macOS 生效）
#   DUILIB_MACOS_BUNDLE_IDENTIFIER  Bundle Identifier（默认 com.nimduilib.<目标名>）
#   DUILIB_MACOS_BUNDLE_NAME        Bundle 显示名称（默认目标名）
#   DUILIB_MACOS_BUNDLE_ICON        .icns 图标路径（绝对路径，或相对当前源码目录）
#   DUILIB_MACOS_BUNDLE_RESOURCES   可选：覆盖默认资源清单（相对 bin/resources，
#                                   分号或逗号分隔）；不设置时按示例名自动推导

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

# 示例自己的资源目录名（resources/themes/default 下的子目录名）。
# 约定：目录名与示例的目标名对应，比较时忽略大小写和下划线差异，因此
# basic→basic、ChildWindow→child_window、MultiLang→MultiLang、
# WebView2Browser→webview2_browser 都能自动匹配。
# 优先级：
#   1. 示例显式声明的 DUILIB_THEME_DIR_NAME（任何示例都可用，cef/CefBrowser 使用
#      这种方式；显式声明时会校验目录是否存在，写错在配置阶段直接报错）；
#   2. themes/default 下与目标名匹配的目录（优先大小写完全一致的）；
#   3. 都找不到时返回空，表示该示例没有自己的 XML 资源（如 ColorPicker）。
function(_duilib_macos_bundle_theme_dir target out_dir)
    set(_theme_dir "")
    set(_theme_dir_explicit FALSE)
    if(DEFINED DUILIB_THEME_DIR_NAME AND
       NOT "${DUILIB_THEME_DIR_NAME}" STREQUAL "")
        set(_theme_dir "${DUILIB_THEME_DIR_NAME}")
        set(_theme_dir_explicit TRUE)
    else()
        string(TOLOWER "${target}" _target_key)
        string(REPLACE "_" "" _target_key "${_target_key}")
        file(GLOB _theme_dir_candidates
             LIST_DIRECTORIES true
             "${DUILIB_ROOT}/bin/resources/themes/default/*")
        foreach(_theme_dir_candidate IN LISTS _theme_dir_candidates)
            get_filename_component(_theme_dir_name "${_theme_dir_candidate}" NAME)
            string(TOLOWER "${_theme_dir_name}" _theme_dir_key)
            string(REPLACE "_" "" _theme_dir_key "${_theme_dir_key}")
            if("${_theme_dir_key}" STREQUAL "${_target_key}")
                if("${_theme_dir_name}" STREQUAL "${target}")
                    set(_theme_dir "${_theme_dir_name}")
                    break()
                elseif("${_theme_dir}" STREQUAL "")
                    set(_theme_dir "${_theme_dir_name}")
                endif()
            endif()
        endforeach()
    endif()

    if(_theme_dir_explicit)
        set(_theme_dir_path "${DUILIB_ROOT}/bin/resources/themes/default/${_theme_dir}")
        if(NOT IS_DIRECTORY "${_theme_dir_path}")
            message(FATAL_ERROR
                "DUILIB_THEME_DIR_NAME='${_theme_dir}' is not a directory: ${_theme_dir_path}\n"
                "Target '${target}': set DUILIB_THEME_DIR_NAME to a subdirectory name of "
                "bin/resources/themes/default, or remove it to auto-derive from the target name.")
        endif()
    endif()
    set(${out_dir} "${_theme_dir}" PARENT_SCOPE)
endfunction()

# 默认资源清单（相对 bin/resources），所有示例共用同一约定，示例的
# CMakeLists.txt 不需要再写任何资源列表：
#   themes/default/global.xml    全局变量、字体配置与公共控件样式（启动必需）
#   themes/default/public        公共图片与样式（被 ${PATH_...} 引用）
#   themes/default/<示例目录>     示例自己的 XML（由 _duilib_macos_bundle_theme_dir 推导）
#   themes/color_light|color_dark/global.xml  启动时按系统深浅色加载语义色值（必需）
#   fonts、lang                  启动时加载的字体与 text_id 文本
function(_duilib_macos_bundle_default_resources target out_list)
    _duilib_macos_bundle_theme_dir(${target} _theme_dir)
    set(_list
        "themes/default/global.xml"
        "themes/default/public")
    if(NOT "${_theme_dir}" STREQUAL "")
        list(APPEND _list "themes/default/${_theme_dir}")
    else()
        # 没有自己的主题目录（如 ColorPicker），只复制公共资源；
        # 新示例若目录名与项目名差异较大，可显式设置 DUILIB_THEME_DIR_NAME。
        message(STATUS
            "No example theme directory matched '${target}' in themes/default; "
            "copying shared resources only")
    endif()
    list(APPEND _list
        "themes/color_light/global.xml"
        "themes/color_dark/global.xml"
        "fonts"
        "lang")
    set(${out_list} "${_list}" PARENT_SCOPE)
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

    # 资源复制到 Contents/Resources/duilib/resources：与普通程序的 resources/
    # 目录保持同构，GetResourceRootPath(true) 与 resources.zip 模式都依赖该布局。
    # 默认清单由 _duilib_macos_bundle_default_resources() 按示例名推导，示例的
    # CMakeLists.txt 无需配置；设置 DUILIB_MACOS_BUNDLE_RESOURCES 时以它为准，
    # 用于补充额外资源或去掉默认条目。
    set(_duilib_resources_from "${DUILIB_ROOT}/bin/resources")
    if(NOT EXISTS "${_duilib_resources_from}")
        message(FATAL_ERROR
            "macOS bundle resources not found: ${_duilib_resources_from}\n"
            "Restore bin/resources in the source tree before building a bundle.")
    endif()
    set(_duilib_resources_to
        "$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources/duilib/resources")

    set(_duilib_resources_explicit FALSE)
    set(_duilib_resource_list "")
    if(DEFINED DUILIB_MACOS_BUNDLE_RESOURCES AND
       NOT "${DUILIB_MACOS_BUNDLE_RESOURCES}" STREQUAL "")
        set(_duilib_resource_entries "")
        string(REPLACE "," ";" _duilib_resource_entries
               "${DUILIB_MACOS_BUNDLE_RESOURCES}")
        foreach(_duilib_resource IN LISTS _duilib_resource_entries)
            string(STRIP "${_duilib_resource}" _duilib_resource)
            if(NOT "${_duilib_resource}" STREQUAL "")
                list(APPEND _duilib_resource_list "${_duilib_resource}")
            endif()
        endforeach()
        set(_duilib_resources_explicit TRUE)
    endif()
    list(LENGTH _duilib_resource_list _duilib_resource_count)
    if(_duilib_resource_count EQUAL 0)
        # 未设置（或列表为空）时使用公共约定，示例无需配置。
        _duilib_macos_bundle_default_resources(${target} _duilib_resource_list)
        set(_duilib_resources_explicit FALSE)
    endif()

    # 校验清单条目：显式清单写错要立刻失败；默认清单里缺失的条目（例如某个示例
    # 没有自己的主题目录）直接跳过，只复制公共资源。
    set(_duilib_resource_sync_entries "")
    foreach(_duilib_resource IN LISTS _duilib_resource_list)
        set(_duilib_resource_src
            "${_duilib_resources_from}/${_duilib_resource}")
        if(NOT EXISTS "${_duilib_resource_src}")
            if(_duilib_resources_explicit)
                message(FATAL_ERROR
                    "macOS bundle resource not found: ${_duilib_resource_src}\n"
                    "Check DUILIB_MACOS_BUNDLE_RESOURCES in ${CMAKE_CURRENT_SOURCE_DIR}")
            endif()
            message(STATUS
                "Skipping macOS bundle resource (not found): ${_duilib_resource}")
            continue()
        endif()
        list(APPEND _duilib_resource_sync_entries "${_duilib_resource}")
    endforeach()

    # 资源同步不能挂在链接后的 POST_BUILD 上：只改资源、不改源码时目标不会重新
    # 链接，POST_BUILD 不执行，Bundle 里就会一直是旧资源（测试时看到的仍是旧界面）。
    # 这里改为「每次构建都执行」的依赖目标，由脚本按内容比较，只复制变化的文件，
    # 并删除清单之外的旧文件。
    if(_duilib_resource_sync_entries)
        string(REPLACE ";" "|" _duilib_resource_sync_arg
               "${_duilib_resource_sync_entries}")
        add_custom_target(${target}_bundle_resources
            COMMAND ${CMAKE_COMMAND}
                    "-DDUILIB_RES_SRC=${_duilib_resources_from}"
                    "-DDUILIB_RES_DST=${_duilib_resources_to}"
                    "-DDUILIB_RES_ENTRIES=${_duilib_resource_sync_arg}"
                    -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/duilib_copy_bundle_resources.cmake"
            COMMENT "Syncing duilib resources into ${target}.app"
            VERBATIM)
        add_dependencies(${target} ${target}_bundle_resources)
    endif()

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
