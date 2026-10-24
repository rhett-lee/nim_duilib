#!/bin/bash

# check windows
is_windows() {
    case "$(uname -s)" in
        CYGWIN*|MINGW32*|MSYS*|MINGW*)
            return 0  # Windows
            ;;
        *)
            return 1  # Not Windows
            ;;
    esac
}

if [ "$(uname -s)" == "Darwin" ]; then
    echo "Please run this script on Linux system."
    exit 1
elif [ "$(uname -s)" == "FreeBSD" ]; then
    echo "Please run this script on Linux system."
    exit 1
elif is_windows; then
    echo "Please run this script on Linux system."
    exit 1
else
    echo "Linux"
fi

DUILIB_SRC_ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
echo "DUILIB_SRC_ROOT_DIR: $DUILIB_SRC_ROOT_DIR"

# compiler flag
has_gcc=0
has_clang=0

# gcc/g++
if command -v gcc &> /dev/null && command -v g++ &> /dev/null; then
    has_gcc=1
fi

# clang/clang++
if command -v clang &> /dev/null && command -v clang++ &> /dev/null; then
    has_clang=1
fi

if [ "$has_gcc$has_clang" == "00" ]; then
    echo "- GCC/G++ not found in PATH"
    echo "- Clang/Clang++ not found in PATH"
    exit 1
fi

# 设置编译器
if [ "$has_clang" -eq 1 ]; then
    DUILIB_CC=clang
    DUILIB_CXX=clang++
    DUILIB_COMPILER_ID=llvm
else
    DUILIB_CC=gcc
    DUILIB_CXX=g++
    DUILIB_COMPILER_ID=gcc
fi

# UOS 固定使用gcc编译，因为用clang++的时候，有链接错误
if grep -q "ID=uos" /etc/os-release; then
    echo "UOS"
    DUILIB_CC=gcc
    DUILIB_CXX=g++
    DUILIB_COMPILER_ID=gcc
fi

cmake_version=$(cmake --version | grep -oE '[0-9]+\.[0-9]+')
required_version=3.24
if [ $(echo "$cmake_version >= $required_version" | bc) -eq 1 ]; then
    DUILIB_CMAKE_REFRESH=--fresh
else
    DUILIB_CMAKE_REFRESH=
fi

DUILIB_CMAKE="cmake ${DUILIB_CMAKE_REFRESH} -DCMAKE_C_COMPILER=$DUILIB_CC -DCMAKE_CXX_COMPILER=$DUILIB_CXX"
DUILIB_MAKE="cmake --build"
DUILIB_MAKE_THREADS="-j 6"

# 编译类型：Debug 还是 Release（默认 Release，可用环境变量覆盖）
# 用法：DUILIB_BUILD_TYPE=Debug ./build/linux_build.sh
DUILIB_BUILD_TYPE="${DUILIB_BUILD_TYPE:-Release}"
case "$DUILIB_BUILD_TYPE" in
    [Dd][Ee][Bb][Uu][Gg]) DUILIB_BUILD_TYPE=Debug ;;
    [Rr][Ee][Ll][Ee][Aa][Ss][Ee]) DUILIB_BUILD_TYPE=Release ;;
    *)
        echo "Invalid DUILIB_BUILD_TYPE: $DUILIB_BUILD_TYPE (expected Debug or Release)"
        exit 1
        ;;
esac
echo "DUILIB_BUILD_TYPE: $DUILIB_BUILD_TYPE"

# Skia库目录由 CMake 自动推导并校验，无需在此处指定

# 编译临时目录：按配置分目录，Debug/Release 互不覆盖
DUILIB_BUILD_DIR="$DUILIB_SRC_ROOT_DIR/build/build_temp/${DUILIB_COMPILER_ID}_build/${DUILIB_BUILD_TYPE}"

target_dir="$DUILIB_BUILD_DIR"
if [[ ! -d "$target_dir" ]]; then
    mkdir -p "$target_dir"
fi

# 编译第三方库   
DUILIB_THIRD_PARTY_LIBS=("zlib" "libpng" "cximage" "libwebp" "libcef/libcef_linux")
for third_party_lib in "${DUILIB_THIRD_PARTY_LIBS[@]}"; do
    $DUILIB_CMAKE -S "$DUILIB_SRC_ROOT_DIR/duilib/third_party/$third_party_lib" -B "$DUILIB_BUILD_DIR/$third_party_lib" -DCMAKE_BUILD_TYPE=${DUILIB_BUILD_TYPE}
    $DUILIB_MAKE "$DUILIB_BUILD_DIR/$third_party_lib" $DUILIB_MAKE_THREADS
done

# 编译duilib
$DUILIB_CMAKE -S "$DUILIB_SRC_ROOT_DIR/duilib" -B "$DUILIB_BUILD_DIR/duilib" -DCMAKE_BUILD_TYPE=${DUILIB_BUILD_TYPE}
$DUILIB_MAKE "$DUILIB_BUILD_DIR/duilib" $DUILIB_MAKE_THREADS

# 编译examples下的各个程序
DUILIB_PROGRAMS=("basic" "controls" "ColorPicker" "DpiAware" "chat" "layout" "ListBox" "ListCtrl" "MoveControl" "MultiLang" "render" "RichEdit" "VirtualListBox" "threads" "TreeView" "cef" "CefBrowser" "ChildWindow" "XmlPreview" "ColorTheme" "panel")
for duilib_bin in "${DUILIB_PROGRAMS[@]}"; do
    $DUILIB_CMAKE -S "$DUILIB_SRC_ROOT_DIR/examples/$duilib_bin" -B "$DUILIB_BUILD_DIR/$duilib_bin" -DCMAKE_BUILD_TYPE=${DUILIB_BUILD_TYPE}
    $DUILIB_MAKE "$DUILIB_BUILD_DIR/$duilib_bin" $DUILIB_MAKE_THREADS
done
