# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

#
# CEF_ROOT setup.
# This variable must be set to locate the binary distribution.
#
# macOS 上需要的是「CEF二进制分发包」的根目录（其中含 Release/Chromium Embedded
# Framework.framework、Resources/、cmake/、include/、libcef_dll/），供
# COPY_MAC_FRAMEWORK 拷贝框架、find_package(CEF) 加载 cmake 配置使用；
# 这与仓库内 duilib/third_party/libcef/libcef_macos（只含 cmake/include/libcef_dll，
# 用于编译 libcef_dll_wrapper）不是同一个目录，两者用途不同。
#
# 默认值为 nim_duilib 源码树的同级目录 cef_binary（与 build/build.py deps 的目录约定一致）
# 优先顺序：-DCEF_ROOT=... > 环境变量 CEF_ROOT > 默认目录
if(NOT DEFINED CEF_ROOT OR NOT IS_DIRECTORY "${CEF_ROOT}")
    if(DEFINED ENV{CEF_ROOT} AND IS_DIRECTORY "$ENV{CEF_ROOT}")
        file(TO_CMAKE_PATH "$ENV{CEF_ROOT}" CEF_ROOT)
    else()
        get_filename_component(CEF_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../cef_binary" ABSOLUTE)
    endif()
endif()
if(NOT IS_DIRECTORY "${CEF_ROOT}")
    message(FATAL_ERROR
        "CEF_ROOT not found: ${CEF_ROOT}\n"
        "Please download the CEF binary distribution (see build/build-reference.md), "
        "or pass -DCEF_ROOT=<path-to-cef_binary>.")
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

message(STATUS "CEF_ROOT: ${CEF_ROOT}")
message(STATUS "DUILIB_PROJECT_SRC_DIR: ${DUILIB_PROJECT_SRC_DIR}")

#
# Load the CEF configuration.
#

# Execute FindCEF.cmake which must exist in CMAKE_MODULE_PATH.
find_package(CEF REQUIRED)

# cef project sources.
set(CEF_PROJECT_SRCS ${SRC_FILES})

# cef project helper sources.
set(CEF_PROJECT_HELPER_SRCS
        ${DUILIB_PROJECT_SRC_DIR}/mac/process_helper_mac.cc
   )

# cef project resources.
set(CEF_PROJECT_RESOURCES_SRCS
        ${DUILIB_PROJECT_SRC_DIR}/mac/Info.plist.in
        ${DUILIB_PROJECT_SRC_DIR}/mac/default_icon.icns
   )

# cef project icon file name
set(CEF_PROJECT_ICON_FILE_NAME default_icon.icns)

#
# Shared configuration.
#

# Target executable names.
set(CEF_TARGET "${PROJECT_NAME}")
set(CEF_HELPER_TARGET "${PROJECT_NAME}_Helper")
set(CEF_HELPER_OUTPUT_NAME "${PROJECT_NAME} Helper")

# Determine the target output directory.
set(CEF_TARGET_OUT_DIR "${DUILIB_BIN_PATH}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CEF_TARGET_OUT_DIR}")

#
# Mac OS X configuration.
#

if(OS_MAC)
    option(OPTION_USE_ARC "Build with ARC (automatic Reference Counting) on macOS." ON)

    # Output path for the main app bundle.
    set(CEF_APP "${CEF_TARGET_OUT_DIR}/${CEF_TARGET}.app")

    # Variables referenced from the main Info.plist file.
    set(EXECUTABLE_NAME "${CEF_TARGET}")
    set(PRODUCT_NAME "${CEF_TARGET}")

    # Main app bundle target.
    add_executable(${CEF_TARGET} MACOSX_BUNDLE ${CEF_PROJECT_RESOURCES_SRCS} ${CEF_PROJECT_SRCS})
    
    # 把公共的include/link目录应用到当前目标（CM-17）
    duilib_apply_target_directories()
    
    # duilib: 编译参数改用公共定义（cmake/duilib_compiler_flags.cmake）
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_compiler_flags.cmake")
    set(CEF_COMPILER_FLAGS ${DUILIB_COMPILER_FLAGS})
    set(CEF_CXX_COMPILER_FLAGS ${DUILIB_CXX_COMPILER_FLAGS})
    
    # 开启ARC（自动引用计数）：
    # 必须在目标创建之后设置属性，且 CEF_COMPILER_FLAGS 在上方被整体赋值，
    # 因此 -fobjc-arc 也要在上方赋值之后再追加，否则会被覆盖丢失
    if(OPTION_USE_ARC)
        list(APPEND CEF_COMPILER_FLAGS -fobjc-arc)
        set_target_properties(${CEF_TARGET} PROPERTIES CLANG_ENABLE_OBJC_ARC "YES")
    endif()
    
    unset(CMAKE_OSX_DEPLOYMENT_TARGET)
    
    SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
    target_link_libraries(${CEF_TARGET} ${CEF_STANDARD_LIBS})
    
    target_link_libraries(${CEF_TARGET}
                            ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS}
                            ${ACCELERATE} ${COREFOUNDATION} ${CORETEXT} ${COREGRAPHICS} ${DUILIB_MACOS_LIBS}
                            "-framework AppKit" "-framework Foundation" "-framework Metal"
                          )
    
    set_target_properties(${CEF_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist.in)

    # Copy the CEF framework into the Frameworks directory and create the necessary symlinks.
    COPY_MAC_FRAMEWORK("${CEF_TARGET}" "${CEF_BINARY_DIR}" "${CEF_APP}")

    # temp build directory
    set(CEF_OLD_TARGET_OUT_DIR ${CEF_TARGET_OUT_DIR})
    set(CEF_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin_helper/")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CEF_TARGET_OUT_DIR}")
    
    # Create the multiple Helper app bundle targets.
    foreach(_suffix_list ${CEF_HELPER_APP_SUFFIXES})
        # Convert to a list and extract the suffix values.
        string(REPLACE ":" ";" _suffix_list ${_suffix_list})
        list(GET _suffix_list 0 _name_suffix)
        list(GET _suffix_list 1 _target_suffix)
        list(GET _suffix_list 2 _plist_suffix)

        # Define Helper target and output names.
        set(_helper_target "${CEF_HELPER_TARGET}${_target_suffix}")
        set(_helper_output_name "${CEF_HELPER_OUTPUT_NAME}${_name_suffix}")

        # Create Helper-specific variants of the helper-Info.plist file. Do this
        # manually because the configure_file command (which is executed as part of
        # MACOSX_BUNDLE_INFO_PLIST) uses global env variables and would insert the
        # wrong values with multiple targets.
        set(_helper_info_plist "${CMAKE_CURRENT_BINARY_DIR}/helper-Info${_target_suffix}.plist")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/mac/helper-Info.plist.in" _plist_contents)
        string(REPLACE "\${EXECUTABLE_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
        string(REPLACE "\${PRODUCT_NAME}" "${_helper_output_name}" _plist_contents ${_plist_contents})
        string(REPLACE "\${BUNDLE_ID_SUFFIX}" "${_plist_suffix}" _plist_contents ${_plist_contents})
        file(WRITE ${_helper_info_plist} ${_plist_contents})

        # Create Helper executable target.
        add_executable(${_helper_target} MACOSX_BUNDLE ${CEF_PROJECT_HELPER_SRCS})
        SET_EXECUTABLE_TARGET_PROPERTIES(${_helper_target})
        target_link_libraries(${_helper_target} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS} ${CEF_STANDARD_LIBS})
        set_target_properties(${_helper_target} PROPERTIES
                              MACOSX_BUNDLE_INFO_PLIST ${_helper_info_plist}
                              OUTPUT_NAME ${_helper_output_name})

        # Add the Helper as a dependency of the main executable target.
        add_dependencies(${CEF_TARGET} "${_helper_target}")

        # Copy the Helper app bundle into the Frameworks directory.
        add_custom_command(TARGET ${CEF_TARGET} 
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                                  "${CEF_TARGET_OUT_DIR}/${_helper_output_name}.app"
                                  "${CEF_APP}/Contents/Frameworks/${_helper_output_name}.app"
                           VERBATIM)
    endforeach()

    # restore target out directory
    set(CEF_TARGET_OUT_DIR ${CEF_OLD_TARGET_OUT_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CEF_TARGET_OUT_DIR}")

    # Copy duilib resources into the Resources directory.
    if("${DUILIB_THEME_DIR_NAME}" STREQUAL "")
        message(FATAL_ERROR "NOT DEFINED DUILIB_THEME_DIR_NAME!")
    endif()
    
    set(DUILIB_RES_FROM "${DUILIB_ROOT}/bin/resources")
    set(DUILIB_RES_TO "${CEF_APP}/Contents/Resources/duilib")
    add_custom_command(TARGET ${CEF_TARGET} 
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${DUILIB_RES_FROM}/themes/default/public" "${DUILIB_RES_TO}/themes/default/public/"
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${DUILIB_RES_FROM}/themes/default/${DUILIB_THEME_DIR_NAME}" "${DUILIB_RES_TO}/themes/default/${DUILIB_THEME_DIR_NAME}"
                       COMMAND ${CMAKE_COMMAND} -E copy "${DUILIB_RES_FROM}/themes/default/global.xml" "${DUILIB_RES_TO}/themes/default/"
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${DUILIB_RES_FROM}/lang/" "${DUILIB_RES_TO}/lang/"
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${DUILIB_RES_FROM}/fonts/" "${DUILIB_RES_TO}/fonts/"
                       VERBATIM)
    
    # Manually process and copy over resource files.
    # The Xcode generator can support this via the set_target_properties RESOURCE
    # directive but that doesn't properly handle nested resource directories.
    # Remove these prefixes from input file paths.
    set(PREFIXES "mac/")
    COPY_MAC_RESOURCES("${CEF_PROJECT_RESOURCES_SRCS}" "${PREFIXES}" "${CEF_TARGET}" "${DUILIB_PROJECT_SRC_DIR}" "${CEF_APP}")
endif()
