#!/bin/bash

DUILIB_SRC_ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
echo "DUILIB_SRC_ROOT_DIR: $DUILIB_SRC_ROOT_DIR"

# 设置编译器
DUILIB_CC=clang
DUILIB_CXX=clang++
DUILIB_COMPILER_ID=llvm

DUILIB_CMAKE="cmake -DCMAKE_C_COMPILER=$DUILIB_CC -DCMAKE_CXX_COMPILER=$DUILIB_CXX"
DUILIB_MAKE="cmake --build"
DUILIB_MAKE_THREADS="-j 6"

# Skia库的子目录，固定使用LLVM编译的，如果想使用默认规则，可注释掉这行
DUILIB_SKIA_LIB_SUBPATH=llvm.x64.release

# lib目录
target_dir="$DUILIB_SRC_ROOT_DIR/libs/"
if [[ ! -d "$target_dir" ]]; then
    mkdir -p "$target_dir"
fi

# 编译临时目录
DUILIB_BUILD_DIR="$DUILIB_SRC_ROOT_DIR/build_temp/$DUILIB_COMPILER_ID_build"

target_dir="$DUILIB_BUILD_DIR"
if [[ ! -d "$target_dir" ]]; then
    mkdir -p "$target_dir"
fi

# 编译第三方库   
DUILIB_THIRD_PARTY_LIBS=("zlib" "libpng" "cximage" "libwebp" "libcef_linux")
for third_party_lib in "${DUILIB_THIRD_PARTY_LIBS[@]}"; do
    $DUILIB_CMAKE -S "$DUILIB_SRC_ROOT_DIR/duilib/third_party/$third_party_lib" -B "$DUILIB_BUILD_DIR/$third_party_lib" -DCMAKE_BUILD_TYPE=Release
    $DUILIB_MAKE "$DUILIB_BUILD_DIR/$third_party_lib" $DUILIB_MAKE_THREADS
done

#编译duilib
$DUILIB_CMAKE -S "$DUILIB_SRC_ROOT_DIR/duilib" -B "$DUILIB_BUILD_DIR/duilib" -DCMAKE_BUILD_TYPE=Release
$DUILIB_MAKE "$DUILIB_BUILD_DIR/duilib" $DUILIB_MAKE_THREADS

#编译examples下的各个程序
DUILIB_PROGRAMS=("basic" "controls" "ColorPicker" "DpiAware" "layouts" "ListBox" "ListCtrl" "MoveControl" "MultiLang" "render" "RichEdit" "VirtualListBox" "threads" "TreeView")
for duilib_bin in "${DUILIB_PROGRAMS[@]}"; do
    $DUILIB_CMAKE -S "$DUILIB_SRC_ROOT_DIR/examples/$duilib_bin" -B "$DUILIB_BUILD_DIR/$duilib_bin" -DCMAKE_BUILD_TYPE=Release -DDUILIB_SKIA_LIB_SUBPATH="$DUILIB_SKIA_LIB_SUBPATH"
    $DUILIB_MAKE "$DUILIB_BUILD_DIR/$duilib_bin" $DUILIB_MAKE_THREADS
done
