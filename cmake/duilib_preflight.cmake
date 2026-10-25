# 阶段 B：编译器已探测后的预检（include 模式，在 project() 之后）
#
# 只做确定性的判断，不下载、不改文件。

include("${CMAKE_CURRENT_LIST_DIR}/duilib_i18n.cmake")

# MSVC 版本：VS 2022 / MSVC 19.30+
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.30")
        duilib_error_values("MSVC ${CMAKE_CXX_COMPILER_VERSION} is too old (19.30+ / VS 2022 required)"
                            "install Visual Studio 2022 or newer with the C++ workload")
    endif()
endif()

# GNU / Clang：仅要求可用的 C++20 编译器
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0")
        duilib_error_values("gcc ${CMAKE_CXX_COMPILER_VERSION} is too old (10.0+ required)"
                            "upgrade gcc, or use clang 12+")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
        duilib_error_values("clang ${CMAKE_CXX_COMPILER_VERSION} is too old (12.0+ required)"
                            "upgrade clang")
    endif()
endif()

# CEF + MinGW：ABI 不兼容
if(DUILIB_ENABLE_CEF AND MINGW)
    duilib_error("preflight.cef_mingw" "preflight.cef_mingw_hint")
endif()

# 目标架构与 Skia CPU 架构必须一致（子项目据此选择 Skia 子目录）
if(NOT DUILIB_TARGET_ARCH_RESOLVED STREQUAL DUILIB_SKIA_CPU_ARCH)
    duilib_error_values("DUILIB_TARGET_ARCH=${DUILIB_TARGET_ARCH_RESOLVED} but DUILIB_SKIA_CPU_ARCH=${DUILIB_SKIA_CPU_ARCH}"
                        "remove the manual -DDUILIB_SKIA_CPU_ARCH override")
endif()
