# nim_duilib SDK 消费者用例工程（发布包测试专用）
#
# 该工程只消费解包后的 SDK，不引用 nim_duilib 源码树。
# 由 build/sdk_package_test.py 驱动，也可以手工配置：
#   cmake -S build/sdk_test -B <build-dir> -DNIM_DUILIB_SDK_DIR=<解包后的 SDK 根>
#
# 可选期望值（空表示不校验）：
#   -DSDK_EXPECT_VERSION / SYSTEM / ARCH / CONFIG / TOOLCHAIN
#   -DSDK_EXPECT_IS_DLL / HAS_SDL / HAS_CEF / HAS_WEBVIEW2 / HAS_JPEG / MSVC_RUNTIME

cmake_minimum_required(VERSION 3.18)
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

if(NOT DEFINED NIM_DUILIB_SDK_DIR OR NIM_DUILIB_SDK_DIR STREQUAL "")
    message(FATAL_ERROR "SDK 测试工程需要 -DNIM_DUILIB_SDK_DIR=<解包后的 SDK 根目录>")
endif()

project(nim_duilib_sdk_test LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "${NIM_DUILIB_SDK_DIR}")
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
