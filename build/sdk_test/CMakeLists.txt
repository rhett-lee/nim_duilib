# nim_duilib SDK 消费者用例工程（发布包测试专用）
#
# 该工程只消费解包后的 SDK，不引用 nim_duilib 源码树。
# 由 build/sdk_package_test.py 驱动，也可以手工配置：
#   cmake -S build/sdk_test -B <build-dir> -DNIM_DUILIB_SDK_DIR=<解包后的 SDK 根>
# 解包后把 sdk_test 放在 SDK 根目录里时，可以省略该变量或写相对路径：
#   cmake -S ./sdk_test -B ./build -DNIM_DUILIB_SDK_DIR=./
#
# 可选期望值（空表示不校验）：
#   -DSDK_EXPECT_VERSION / SYSTEM / ARCH / CONFIG / TOOLCHAIN
#   -DSDK_EXPECT_IS_DLL / HAS_SDL / HAS_CEF / HAS_WEBVIEW2 / HAS_JPEG / MSVC_RUNTIME
#
# 目标：
#   nim_duilib_sdk_consumer  -> sdk_consumer（全平台，可执行文件）
#   nim_duilib_sdk_bundle    -> sdk_bundle.app（仅 macOS，App Bundle 用例）

cmake_minimum_required(VERSION 3.18)
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

# ---------------------------------------------------------------------------
# 解析 SDK 根目录
#
# NIM_DUILIB_SDK_DIR 允许省略、写绝对路径或写相对路径：
#   - 绝对路径：直接使用；
#   - 相对路径：CMake 不提供“调用 cmake 的目录”，因此按多个基准分别解析，
#     任一命中即可（sdk_test 自身、它的上一级/上两级、构建目录及其上一级/上两级）；
#   - 省略：按常见布局探测。
# 典型用法 1（sdk_test 拷进 SDK 根目录，cwd 就是 SDK 根）：
#     cmake -S ./sdk_test -B ./build -DNIM_DUILIB_SDK_DIR=./
# 典型用法 2（sdk_test 与解包目录并列，cwd 是它们的上一级）：
#     cmake -S ./sdk_test -B ./build -DNIM_DUILIB_SDK_DIR=./nim_duilib-1.0.2026.1023-win-x64
# 两种写法都要能找到 lib/cmake/nim_duilib/nim_duilibConfig.cmake。
# ---------------------------------------------------------------------------
get_filename_component(_sdk_src_dir "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
get_filename_component(_sdk_bin_dir "${CMAKE_CURRENT_BINARY_DIR}" ABSOLUTE)

# 相对路径的可能基准目录：调用 cmake 的目录通常是其中之一。
set(_sdk_anchor_dirs
    "${_sdk_src_dir}"
    "${_sdk_src_dir}/.."
    "${_sdk_src_dir}/../.."
    "${_sdk_bin_dir}"
    "${_sdk_bin_dir}/.."
    "${_sdk_bin_dir}/../..")

set(_sdk_user_absolute FALSE)
set(_sdk_bases "")
if(DEFINED NIM_DUILIB_SDK_DIR AND NOT NIM_DUILIB_SDK_DIR STREQUAL "")
    file(TO_CMAKE_PATH "${NIM_DUILIB_SDK_DIR}" _sdk_user_dir)
    if(IS_ABSOLUTE "${_sdk_user_dir}")
        set(_sdk_user_absolute TRUE)
        list(APPEND _sdk_bases "${_sdk_user_dir}")
    else()
        foreach(_sdk_anchor IN LISTS _sdk_anchor_dirs)
            list(APPEND _sdk_bases "${_sdk_anchor}/${_sdk_user_dir}")
        endforeach()
    endif()
else()
    # 未指定时才按常见布局兜底：SDK 根通常是 sdk_test 的上一级，
    # 或构建目录的上两级（<SDK>/build/xxx）。
    list(APPEND _sdk_bases
        "${_sdk_src_dir}/.."
        "${_sdk_bin_dir}/.."
        "${_sdk_bin_dir}/../..")
endif()

set(_sdk_candidates "${_sdk_bases}")

set(NIM_DUILIB_SDK_PREFIX "")
set(NIM_DUILIB_SDK_CONFIG_DIR "")
set(_sdk_tried "")
foreach(_sdk_candidate IN LISTS _sdk_candidates)
    get_filename_component(_sdk_dir "${_sdk_candidate}" ABSOLUTE)
    list(FIND _sdk_tried "${_sdk_dir}" _sdk_seen)
    if(NOT _sdk_seen EQUAL -1)
        continue()
    endif()
    list(APPEND _sdk_tried "${_sdk_dir}")
    if(EXISTS "${_sdk_dir}/lib/cmake/nim_duilib/nim_duilibConfig.cmake")
        set(NIM_DUILIB_SDK_PREFIX "${_sdk_dir}")
        break()
    endif()
    # 也允许直接指定包配置目录：<SDK>/lib/cmake/nim_duilib
    if(EXISTS "${_sdk_dir}/nim_duilibConfig.cmake")
        set(NIM_DUILIB_SDK_CONFIG_DIR "${_sdk_dir}")
        break()
    endif()
endforeach()

if(NIM_DUILIB_SDK_PREFIX STREQUAL "" AND NIM_DUILIB_SDK_CONFIG_DIR STREQUAL "")
    # 允许直接用 -Dnim_duilib_DIR / -Dnim_duilib_ROOT 指定包，此时交给 find_package。
    if(NOT DEFINED nim_duilib_DIR AND NOT DEFINED nim_duilib_ROOT)
        string(REPLACE ";" "\n  " _sdk_tried_text "${_sdk_tried}")
        message(FATAL_ERROR
            "未找到 nim_duilib 的 CMake 包配置（lib/cmake/nim_duilib/nim_duilibConfig.cmake）。\n"
            "NIM_DUILIB_SDK_DIR = '${NIM_DUILIB_SDK_DIR}'\n"
            "已尝试的目录：\n  ${_sdk_tried_text}\n"
            "请把 NIM_DUILIB_SDK_DIR 指向解包后的 SDK 根目录（同时含 include/ 与 lib/ 的那层），例如：\n"
            "  cmake -S <SDK>/sdk_test -B <SDK>/build -DNIM_DUILIB_SDK_DIR=<SDK>")
    endif()
elseif(NIM_DUILIB_SDK_CONFIG_DIR STREQUAL "")
    list(APPEND CMAKE_PREFIX_PATH "${NIM_DUILIB_SDK_PREFIX}")
    if(NOT _sdk_user_absolute)
        message(STATUS "nim_duilib SDK 根目录（自动探测）：${NIM_DUILIB_SDK_PREFIX}")
    endif()
else()
    set(nim_duilib_DIR "${NIM_DUILIB_SDK_CONFIG_DIR}")
    if(NOT _sdk_user_absolute)
        message(STATUS "nim_duilib 包配置目录（自动探测）：${NIM_DUILIB_SDK_CONFIG_DIR}")
    endif()
endif()

project(nim_duilib_sdk_test LANGUAGES CXX)

find_package(nim_duilib CONFIG REQUIRED)

# 元数据自洽：SDK 的 nim_duilibPackageInfo.cmake 必须与包的实际内容一致。
foreach(_sdk_item IN ITEMS
        "VERSION|NIM_DUILIB_PACKAGE_VERSION"
        "SYSTEM|NIM_DUILIB_PACKAGE_SYSTEM"
        "ARCH|NIM_DUILIB_PACKAGE_ARCH"
        "CONFIG|NIM_DUILIB_PACKAGE_CONFIG"
        "TOOLCHAIN|NIM_DUILIB_PACKAGE_TOOLCHAIN"
        "IS_DLL|NIM_DUILIB_PACKAGE_IS_DLL"
        "HAS_SDL|NIM_DUILIB_PACKAGE_HAS_SDL"
        "HAS_CEF|NIM_DUILIB_PACKAGE_HAS_CEF"
        "HAS_WEBVIEW2|NIM_DUILIB_PACKAGE_HAS_WEBVIEW2"
        "HAS_JPEG|NIM_DUILIB_PACKAGE_HAS_JPEG"
        "MSVC_RUNTIME|NIM_DUILIB_PACKAGE_MSVC_RUNTIME")
    string(REPLACE "|" ";" _sdk_pair "${_sdk_item}")
    list(GET _sdk_pair 0 _sdk_expect_name)
    list(GET _sdk_pair 1 _sdk_actual_var)
    set(_sdk_expect_var "SDK_EXPECT_${_sdk_expect_name}")
    if(DEFINED ${_sdk_expect_var} AND NOT "${${_sdk_expect_var}}" STREQUAL "")
        if(NOT "${${_sdk_actual_var}}" STREQUAL "${${_sdk_expect_var}}")
            message(FATAL_ERROR
                "SDK 测试失败：${_sdk_actual_var} 期望 '${${_sdk_expect_var}}'，"
                "实际 '${${_sdk_actual_var}}'")
        endif()
    endif()
endforeach()

add_executable(nim_duilib_sdk_consumer
    main.cpp
    header_duilib.cpp
)
set_target_properties(nim_duilib_sdk_consumer PROPERTIES OUTPUT_NAME "sdk_consumer")

# 头文件用例只在 SDK 声明支持时参与编译：声明支持但没随包会直接编译失败。
if(NIM_DUILIB_PACKAGE_HAS_CEF)
    target_sources(nim_duilib_sdk_consumer PRIVATE header_cef.cpp)
endif()
if(NIM_DUILIB_PACKAGE_HAS_WEBVIEW2)
    target_sources(nim_duilib_sdk_consumer PRIVATE header_webview2.cpp)
endif()

target_link_libraries(nim_duilib_sdk_consumer PRIVATE nim_duilib::duilib)

# 复制运行期 DLL 与 resources/ 到可执行文件同级，模拟下游真实部署。
nim_duilib_copy_runtime(nim_duilib_sdk_consumer RESOURCES)

# ---------------------------------------------------------------------------
# macOS App Bundle 消费者用例
#
# 专门验证 bundle 形态的下游接入：
#   - MACOSX_BUNDLE 目标，产物为 sdk_bundle.app；
#   - FRAMEWORKS 把 SDK 的 lib/libSDL3*.dylib 复制到 Contents/Frameworks，
#     并把主可执行文件的 rpath 设为 @executable_path/../Frameworks（优先 bundle 内的库）；
#   - RESOURCES 在 bundle 目标上复制到 Contents/Resources/duilib/resources/，
#     即 GlobalManager::GetResourceRootPath(true) 在 macOS 下定位的标准目录。
# 运行期用例见 bundle_main.cpp；由 sdk_package_test.py 的 C11 驱动。
# 该目标只在 macOS 上创建，其他平台不参与配置与构建。
# ---------------------------------------------------------------------------
if(APPLE)
    add_executable(nim_duilib_sdk_bundle MACOSX_BUNDLE
        bundle_main.cpp
    )
    set_target_properties(nim_duilib_sdk_bundle PROPERTIES
        OUTPUT_NAME "sdk_bundle"
        MACOSX_BUNDLE_BUNDLE_NAME "nim_duilib SDK Bundle"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.nimduilib.sdk.bundle.test"
        MACOSX_BUNDLE_BUNDLE_VERSION "${NIM_DUILIB_PACKAGE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${NIM_DUILIB_PACKAGE_VERSION}"
    )
    target_link_libraries(nim_duilib_sdk_bundle PRIVATE nim_duilib::duilib)
    nim_duilib_copy_runtime(nim_duilib_sdk_bundle RESOURCES FRAMEWORKS)
endif()
