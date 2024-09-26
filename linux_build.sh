#!/bin/bash

SRC_ROOT_DIR=$(realpath $(dirname "$0"))

# 编译zlib(.a文件有改名)
cmake -S "$SRC_ROOT_DIR/duilib/third_party/zlib/" -B "$SRC_ROOT_DIR/build_temp/zlib" -DZLIB_BUILD_EXAMPLES=OFF
cd "$SRC_ROOT_DIR/build_temp/zlib"
make 
cd "$SRC_ROOT_DIR/"

rm -f "$SRC_ROOT_DIR/libs/libduilib-zlib.a"
cp "$SRC_ROOT_DIR/build_temp/zlib/libz.a" "$SRC_ROOT_DIR/libs/libduilib-zlib.a"

#恢复zconf.h文件
mv "$SRC_ROOT_DIR/duilib/third_party/zlib/zconf.h.included" "$SRC_ROOT_DIR/duilib/third_party/zlib/zconf.h"

#编译libpng(.a文件有改名)
cmake -S "$SRC_ROOT_DIR/duilib/third_party/libpng/" -B "$SRC_ROOT_DIR/build_temp/libpng" -DPNG_TESTS=OFF -DPNG_SHARED=OFF -DPNG_TOOLS=OFF -DZLIB_ROOT="$SRC_ROOT_DIR/duilib/third_party/zlib/"
cd "$SRC_ROOT_DIR/build_temp/libpng"
make 
cd "$SRC_ROOT_DIR/"

rm -f "$SRC_ROOT_DIR/libs/libduilib-png.a"
cp "$SRC_ROOT_DIR/build_temp/libpng/libpng16.a" "$SRC_ROOT_DIR/libs/libduilib-png.a"

#编译cximage
cmake -S "$SRC_ROOT_DIR/duilib/third_party/cximage/" -B "$SRC_ROOT_DIR/build_temp/cximage"
cd "$SRC_ROOT_DIR/build_temp/cximage"
make 
cd "$SRC_ROOT_DIR/"

#编译libwebp
cmake -S "$SRC_ROOT_DIR/duilib/third_party/libwebp/" -B "$SRC_ROOT_DIR/build_temp/libwebp"
cd "$SRC_ROOT_DIR/build_temp/libwebp"
make 
cd "$SRC_ROOT_DIR/"

#编译duilib
cmake -S "$SRC_ROOT_DIR/duilib/" -B "$SRC_ROOT_DIR/build_temp/duilib" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/duilib"
make -j 4
cd "$SRC_ROOT_DIR/"

#清理临时目录
#rm -rf "$SRC_ROOT_DIR/build_temp/"

