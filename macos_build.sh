#!/bin/bash

SRC_ROOT_DIR=$(realpath $(dirname "$0"))

# 创建目录
mkdir "$SRC_ROOT_DIR/libs/"

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
rm -f "$SRC_ROOT_DIR/libs/libduilib-cximage.a"
cmake -S "$SRC_ROOT_DIR/duilib/third_party/cximage/" -B "$SRC_ROOT_DIR/build_temp/cximage"
cd "$SRC_ROOT_DIR/build_temp/cximage"
make 
cd "$SRC_ROOT_DIR/"

#编译libwebp
rm -f "$SRC_ROOT_DIR/libs/libduilib-webp.a"
cmake -S "$SRC_ROOT_DIR/duilib/third_party/libwebp/" -B "$SRC_ROOT_DIR/build_temp/libwebp" 
cd "$SRC_ROOT_DIR/build_temp/libwebp"
make 
cd "$SRC_ROOT_DIR/"

# 编译libcef_dll_wrapper
rm -f "$SRC_ROOT_DIR/libs/libcef_dll_wrapper.a"
cmake -S "$SRC_ROOT_DIR/duilib/third_party/libcef_macos/" -B "$SRC_ROOT_DIR/build_temp/libcef_dll_wrapper" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/libcef_dll_wrapper"
make -j 6
cp "$SRC_ROOT_DIR/build_temp/libcef_dll_wrapper/libcef_dll_wrapper/libcef_dll_wrapper.a" "$SRC_ROOT_DIR/libs/libcef_dll_wrapper.a"
cd "$SRC_ROOT_DIR/"

# # #编译duilib
rm -f "$SRC_ROOT_DIR/libs/libduilib.a"
cmake -S "$SRC_ROOT_DIR/duilib/" -B "$SRC_ROOT_DIR/build_temp/duilib" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/duilib"
make -j 6
cd "$SRC_ROOT_DIR/"

# 编译examples下的各个程序
cmake -S "$SRC_ROOT_DIR/examples/basic/" -B "$SRC_ROOT_DIR/build_temp/basic" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/basic"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/ColorPicker/" -B "$SRC_ROOT_DIR/build_temp/ColorPicker" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/ColorPicker"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/controls/" -B "$SRC_ROOT_DIR/build_temp/controls" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/controls"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/DpiAware/" -B "$SRC_ROOT_DIR/build_temp/DpiAware" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/DpiAware"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/layouts/" -B "$SRC_ROOT_DIR/build_temp/layouts" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/layouts"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/ListBox/" -B "$SRC_ROOT_DIR/build_temp/ListBox" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/ListBox"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/ListCtrl/" -B "$SRC_ROOT_DIR/build_temp/ListCtrl" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/ListCtrl"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/MoveControl/" -B "$SRC_ROOT_DIR/build_temp/MoveControl" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/MoveControl"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/MultiLang/" -B "$SRC_ROOT_DIR/build_temp/MultiLang" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/MultiLang"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/render/" -B "$SRC_ROOT_DIR/build_temp/render" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/render"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/RichEdit/" -B "$SRC_ROOT_DIR/build_temp/RichEdit" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/RichEdit"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/VirtualListBox/" -B "$SRC_ROOT_DIR/build_temp/VirtualListBox" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/VirtualListBox"
make clean; make
cd "$SRC_ROOT_DIR/"

cmake -S "$SRC_ROOT_DIR/examples/threads/" -B "$SRC_ROOT_DIR/build_temp/threads" -DCMAKE_BUILD_TYPE=Debug
cd "$SRC_ROOT_DIR/build_temp/threads"
make clean; make
cd "$SRC_ROOT_DIR/"

# cmake -S "$SRC_ROOT_DIR/examples/cef/" -B "$SRC_ROOT_DIR/build_temp/cef" -DCMAKE_BUILD_TYPE=Debug
# cd "$SRC_ROOT_DIR/build_temp/cef"
# make clean; make
# cd "$SRC_ROOT_DIR/"

# cmake -S "$SRC_ROOT_DIR/examples/multi_browser/" -B "$SRC_ROOT_DIR/build_temp/multi_browser" -DCMAKE_BUILD_TYPE=Debug
# cd "$SRC_ROOT_DIR/build_temp/multi_browser"
# make clean; make
# cd "$SRC_ROOT_DIR/"

# # #清理临时目录
# # #rm -rf "$SRC_ROOT_DIR/build_temp/"

