# ---------------------------------------------------------------------------
# Wayland 客户端库（Linux / FreeBSD，可选依赖）
#
# 用途：
#   1. 通过 wl_surface 的输入区域，实现窗口自绘阴影区域的点击穿透
#      （Wayland 下无法查询其它窗口并激活，只能由窗口管理器把点击投递给下层窗口）；
#   2. 未安装 Wayland 客户端开发包时自动降级：不定义 DUILIB_HAS_WAYLAND_CLIENT，
#      相关代码编译为空实现，构建不受影响。
#
# 使用方式：
#   include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_wayland_libs.cmake")
#   变量结果：DUILIB_HAS_WAYLAND_CLIENT / DUILIB_WAYLAND_INCLUDE_DIR / DUILIB_WAYLAND_LIBS
# ---------------------------------------------------------------------------

set(DUILIB_HAS_WAYLAND_CLIENT OFF)
set(DUILIB_WAYLAND_LIBS "")

find_path(DUILIB_WAYLAND_INCLUDE_DIR
    NAMES wayland-client.h
)

find_library(DUILIB_WAYLAND_LIBRARY
    NAMES wayland-client
)

if(DUILIB_WAYLAND_INCLUDE_DIR AND DUILIB_WAYLAND_LIBRARY)
    set(DUILIB_HAS_WAYLAND_CLIENT ON)
    set(DUILIB_WAYLAND_LIBS ${DUILIB_WAYLAND_LIBRARY})
else()
    set(DUILIB_HAS_WAYLAND_CLIENT OFF)
    set(DUILIB_WAYLAND_INCLUDE_DIR "")
    set(DUILIB_WAYLAND_LIBS "")
endif()
