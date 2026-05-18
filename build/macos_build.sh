#!/bin/bash
if [ "$(uname -s)" != "Darwin" ]; then
    echo "Please run this script on macOS system."
    exit 1
fi

DUILIB_SRC_ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
SKIA_SRC_ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../skia" && pwd)
echo "DUILIB_SRC_ROOT_DIR: $DUILIB_SRC_ROOT_DIR"
echo "SKIA_SRC_ROOT_DIR: $SKIA_SRC_ROOT_DIR"

# 设置编译器
DUILIB_CC=clang
DUILIB_CXX=clang++
DUILIB_COMPILER_ID=llvm

# 首次构建或需要清理时: ./macos_build.sh --fresh
if [[ "${1:-}" == "--fresh" ]]; then
    DUILIB_CMAKE_REFRESH=--fresh
else
    DUILIB_CMAKE_REFRESH=
fi

DUILIB_CMAKE="cmake ${DUILIB_CMAKE_REFRESH} -DCMAKE_C_COMPILER=$DUILIB_CC -DCMAKE_CXX_COMPILER=$DUILIB_CXX -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
DUILIB_MAKE="cmake --build"
DUILIB_MAKE_THREADS="-j 6"

# 编译类型：Debug还是Release
DUILIB_BUILD_TYPE=Release

# Skia库的子目录，固定使用LLVM编译的，如果想使用默认规则，可注释掉这部分代码
# 获取CPU架构
CPU_ARCH_STR=$(uname -m)

# 转换为标准架构标识
if [ "$CPU_ARCH_STR" = "x86_64" ] || [ "$CPU_ARCH_STR" = "amd64" ]; then
    CPU_ARCH=x64
elif [ "$CPU_ARCH_STR" = "aarch64" ] || [ "$CPU_ARCH_STR" = "arm64" ]; then
    CPU_ARCH=arm64
elif [ "$CPU_ARCH_STR" = "armv7l" ]; then
    CPU_ARCH=arm
elif [ "$CPU_ARCH_STR" = "i386" ] || [ "$CPU_ARCH_STR" = "i686" ]; then
    CPU_ARCH=x86
else
    CPU_ARCH=arm64
fi

DUILIB_SKIA_LIB_SUBPATH=llvm.${CPU_ARCH}.release
echo "DUILIB_SKIA_LIB_SUBPATH:${DUILIB_SKIA_LIB_SUBPATH}"
if [[ ! -d "${SKIA_SRC_ROOT_DIR}/out/${DUILIB_SKIA_LIB_SUBPATH}" ]]; then
    echo "Please compile the skia first or run build_duilib_all_in_one.sh."
    exit 1
fi

# 编译临时目录
DUILIB_BUILD_DIR="$DUILIB_SRC_ROOT_DIR/build/build_temp/${DUILIB_COMPILER_ID}_build"

target_dir="$DUILIB_BUILD_DIR"
if [[ ! -d "$target_dir" ]]; then
    mkdir -p "$target_dir"
fi

# 编译第三方库   
DUILIB_THIRD_PARTY_LIBS=("zlib" "libpng" "cximage" "libwebp" "libcef/libcef_macos")
for third_party_lib in "${DUILIB_THIRD_PARTY_LIBS[@]}"; do
    $DUILIB_CMAKE -S "$DUILIB_SRC_ROOT_DIR/duilib/third_party/$third_party_lib" -B "$DUILIB_BUILD_DIR/$third_party_lib" -DCMAKE_BUILD_TYPE=${DUILIB_BUILD_TYPE}
    $DUILIB_MAKE "$DUILIB_BUILD_DIR/$third_party_lib" $DUILIB_MAKE_THREADS
done

# 编译duilib
$DUILIB_CMAKE -S "$DUILIB_SRC_ROOT_DIR/duilib" -B "$DUILIB_BUILD_DIR/duilib" -DCMAKE_BUILD_TYPE=${DUILIB_BUILD_TYPE}
$DUILIB_MAKE "$DUILIB_BUILD_DIR/duilib" $DUILIB_MAKE_THREADS

# 编译examples下的各个程序
DUILIB_PROGRAMS=("basic" "controls" "ColorPicker" "DpiAware" "chat" "layout" "ListBox" "ListCtrl" "MoveControl" "MultiLang" "render" "RichEdit" "VirtualListBox" "threads" "TreeView" "cef" "CefBrowser" "ChildWindow" "XmlPreview")
for duilib_bin in "${DUILIB_PROGRAMS[@]}"; do
    $DUILIB_CMAKE -S "$DUILIB_SRC_ROOT_DIR/examples/$duilib_bin" -B "$DUILIB_BUILD_DIR/$duilib_bin" -DCMAKE_BUILD_TYPE=${DUILIB_BUILD_TYPE} -DDUILIB_SKIA_LIB_SUBPATH="$DUILIB_SKIA_LIB_SUBPATH"
    $DUILIB_MAKE "$DUILIB_BUILD_DIR/$duilib_bin" $DUILIB_MAKE_THREADS
done
