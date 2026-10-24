#!/bin/bash

# 任一目标配置或编译失败时立即退出，避免被后续命令的返回值掩盖
set -e

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

if ! is_windows; then
    echo "Please run this script on Windows/MSYS2 system."
    exit 1
fi

# 判断编译器目标是否为 mingw-w64（排除 MSYS/Cygwin 目标的编译器）
# MSYS2 的 MINGW64/UCRT64 下 gcc 的 triple 为 x86_64-w64-mingw32；
# CLANG64 下 clang 的 triple 为 x86_64-w64-windows-gnu，两种都要接受
is_mingw_compiler() {
    local triple
    triple=$("$1" -dumpmachine 2>/dev/null) || return 1
    case "$triple" in
        *mingw*|*w64-windows-gnu*) return 0 ;;
        *)                         return 1 ;;
    esac
}

if [ "$1" == "-sdl" ]; then    
    SDL_PARAM="-DDUILIB_ENABLE_SDL=ON"
else
    SDL_PARAM=""
fi

echo "SDL_PARAM: ${SDL_PARAM}"

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

# 只接受 mingw-w64 目标的编译器：MSYS/Cygwin 目标的编译器会产出非 Windows 程序
if [ "$has_clang" -eq 1 ] && ! is_mingw_compiler clang; then
    echo "忽略 clang：目标不是 mingw-w64（$(clang -dumpmachine 2>/dev/null)）"
    has_clang=0
fi
if [ "$has_gcc" -eq 1 ] && ! is_mingw_compiler gcc; then
    echo "忽略 gcc：目标不是 mingw-w64（$(gcc -dumpmachine 2>/dev/null)）"
    has_gcc=0
fi

if [ "$has_gcc$has_clang" == "00" ]; then
    echo "- GCC/G++ not found in PATH"
    echo "- Clang/Clang++ not found in PATH"
    echo "- 需要 mingw-w64 目标的编译器（x86_64-w64-mingw32-*）"
    exit 1
fi

# 设置编译器
if [ "$has_clang" -eq 1 ]; then
    DUILIB_CC=clang
    DUILIB_CXX=clang++
    DUILIB_COMPILER_ID=msys2-llvm
else
    DUILIB_CC=gcc
    DUILIB_CXX=g++
    DUILIB_COMPILER_ID=msys2-gcc
fi

# 固定 CMake 生成器：原生 Windows 版 CMake（/mingw64/bin/cmake）的默认生成器是
# Visual Studio，会静默忽略 -DCMAKE_C_COMPILER/-DCMAKE_CXX_COMPILER 并改用 MSVC。
if command -v ninja &> /dev/null; then
    DUILIB_GENERATOR="Ninja"
elif command -v make &> /dev/null; then
    DUILIB_GENERATOR="MSYS Makefiles"
elif command -v mingw32-make &> /dev/null; then
    DUILIB_GENERATOR="MinGW Makefiles"
else
    echo "未找到 ninja/make/mingw32-make：请先安装其中任意一个构建工具"
    exit 1
fi
echo "DUILIB_GENERATOR: $DUILIB_GENERATOR"

# --fresh 默认不启用：它会清空 CMakeFiles（含 Ninja 依赖文件），使增量构建退化为全量重编。
# 构建目录已按“工具链 + 配置”隔离，缓存串味风险很小；需要强制重新配置时设置
# DUILIB_CMAKE_FRESH=1（需要 CMake 3.24+，低版本忽略该参数）
DUILIB_CMAKE_REFRESH=""
cmake_version=$(cmake --version | head -n1 | grep -oE '[0-9]+\.[0-9]+') || true
if [ "${DUILIB_CMAKE_FRESH:-0}" = "1" ] && [ -n "$cmake_version" ]; then
    cmake_major=${cmake_version%%.*}
    cmake_minor=${cmake_version#*.}
    if [ "$cmake_major" -gt 3 ] || { [ "$cmake_major" -eq 3 ] && [ "$cmake_minor" -ge 24 ]; }; then
        DUILIB_CMAKE_REFRESH=--fresh
    fi
fi
echo "CMake version: ${cmake_version:-unknown}, refresh: ${DUILIB_CMAKE_REFRESH:-none}"

DUILIB_CMAKE="cmake ${DUILIB_CMAKE_REFRESH} -DCMAKE_C_COMPILER=$DUILIB_CC -DCMAKE_CXX_COMPILER=$DUILIB_CXX"
DUILIB_MAKE="cmake --build"
# 并行任务数：默认取 CPU 核数，可用 DUILIB_MAKE_JOBS 覆盖
DUILIB_MAKE_JOBS="${DUILIB_MAKE_JOBS:-$(nproc 2>/dev/null || echo 6)}"
DUILIB_MAKE_THREADS="-j ${DUILIB_MAKE_JOBS}"

# 编译类型：Debug还是Release（可用环境变量 DUILIB_BUILD_TYPE 覆盖）
DUILIB_BUILD_TYPE="${DUILIB_BUILD_TYPE:-Release}"
case "$DUILIB_BUILD_TYPE" in
    Debug|Release) ;;
    *)
        echo "无效的 DUILIB_BUILD_TYPE：${DUILIB_BUILD_TYPE}（仅支持 Debug / Release）"
        exit 1
        ;;
esac
echo "DUILIB_BUILD_TYPE: ${DUILIB_BUILD_TYPE}"

# Skia库目录由 CMake 自动推导并校验，无需在此处指定

# 编译临时目录（按配置隔离，切换 Debug/Release 时不复用生成器缓存）
DUILIB_BUILD_DIR="$DUILIB_SRC_ROOT_DIR/build/build_temp/${DUILIB_COMPILER_ID}_build/${DUILIB_BUILD_TYPE}"

target_dir="$DUILIB_BUILD_DIR"
if [[ ! -d "$target_dir" ]]; then
    mkdir -p "$target_dir"
fi

# 编译第三方库   
DUILIB_THIRD_PARTY_LIBS=("zlib" "libpng" "cximage" "libwebp")
for third_party_lib in "${DUILIB_THIRD_PARTY_LIBS[@]}"; do
    $DUILIB_CMAKE -G "$DUILIB_GENERATOR" -S "$DUILIB_SRC_ROOT_DIR/duilib/third_party/$third_party_lib" -B "$DUILIB_BUILD_DIR/$third_party_lib" -DCMAKE_BUILD_TYPE=${DUILIB_BUILD_TYPE}
    $DUILIB_MAKE "$DUILIB_BUILD_DIR/$third_party_lib" $DUILIB_MAKE_THREADS
done

# 编译duilib
$DUILIB_CMAKE -G "$DUILIB_GENERATOR" -S "$DUILIB_SRC_ROOT_DIR/duilib" -B "$DUILIB_BUILD_DIR/duilib" -DCMAKE_BUILD_TYPE=${DUILIB_BUILD_TYPE} ${SDL_PARAM}
$DUILIB_MAKE "$DUILIB_BUILD_DIR/duilib" $DUILIB_MAKE_THREADS

# 编译examples下的各个程序
DUILIB_PROGRAMS=("basic" "controls" "ColorPicker" "DpiAware" "chat" "layout" "ListBox" "ListCtrl" "MoveControl" "MultiLang" "render" "RichEdit" "VirtualListBox" "threads" "TreeView" "WebView2" "WebView2Browser" "ChildWindow" "XmlPreview" "ColorTheme" "panel")
for duilib_bin in "${DUILIB_PROGRAMS[@]}"; do
    $DUILIB_CMAKE -G "$DUILIB_GENERATOR" -S "$DUILIB_SRC_ROOT_DIR/examples/$duilib_bin" -B "$DUILIB_BUILD_DIR/$duilib_bin" -DCMAKE_BUILD_TYPE=${DUILIB_BUILD_TYPE} ${SDL_PARAM}
    $DUILIB_MAKE "$DUILIB_BUILD_DIR/$duilib_bin" $DUILIB_MAKE_THREADS
done
