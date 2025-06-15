#!/usr/bin/env bash

# Linux / MacOS / FreeBSD / MSYS2(Windows)

# Force enable SDL (MSYS2-Windows only)
ENABLE_SDL=0
if [ "$1" == "-sdl" ]; then
    ENABLE_SDL=1
fi

CURRENT_DIR=$(pwd)
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

CPU_ARCH_STR=$(uname -m)
if [ "$CPU_ARCH_STR" = "x86_64" ] || [ "$CPU_ARCH_STR" = "amd64" ]; then
    CPU_ARCH=x64
elif [ "$CPU_ARCH_STR" = "aarch64" ] || [ "$CPU_ARCH_STR" = "arm64" ]; then
    CPU_ARCH=arm64
elif [ "$CPU_ARCH_STR" = "armv7l" ]; then
    CPU_ARCH=arm
elif [ "$CPU_ARCH_STR" = "i386" ] || [ "$CPU_ARCH_STR" = "i686" ]; then
    CPU_ARCH=x86
else
    if [ "$(uname -s)" == "Darwin" ]; then
        CPU_ARCH=arm64
    else
        CPU_ARCH=x64
    fi
fi

# Checking the necessary software
if ! command -v git &> /dev/null
then
    echo "- git not found!"
    exit 1
else
    echo "git found at:"
    which git
fi

if ! command -v python3 &> /dev/null
then
    echo "- python3 not found!"
    exit 1
else
    echo "python3 found at:"
    which python3
fi

if ! command -v gn &> /dev/null
then
    echo "- gn not found!"
    exit 1
else
    echo "gn found at:"
    which gn
fi

if ! command -v ninja &> /dev/null
then
    echo "- ninja not found!"
    exit 1
else
    echo "ninja found at:"
    which ninja
fi

if ! command -v unzip &> /dev/null
then
    echo "- unzip not found!"
    exit 1
else
    echo "unzip found at:"
    which unzip
fi

if ! command -v cmake &> /dev/null
then
    echo "- cmake not found!"
    exit 1
else
    echo "cmake found at:"
    which cmake
fi

# flag
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

cd "$SCRIPT_DIR"
pwd
if [ ! -d "./nim_duilib/.git" ]; then
    if [ -d "../../nim_duilib/.git" ]; then
        cd ../../
    fi
fi
pwd

start_time=$(date +%s)
retry_delay=10

# Retry clone nim_duilib
clone_nim_duilib() {
    if [ ! -d "./nim_duilib/.git" ]; then
        git clone https://github.com/rhett-lee/nim_duilib.git
    else
        git -C ./nim_duilib pull
    fi
    if [ $? -ne 0 ]; then
        sleep $retry_delay
        clone_nim_duilib
    fi
}
clone_nim_duilib
if [ ! -d "./nim_duilib/.git" ]; then
    echo "clone nim_duilib failed!"
    cd "$CURRENT_DIR"
    exit 1
fi

# Retry clone skia
clone_skia() {
    if [ ! -d "./skia/.git" ]; then
        git clone https://github.com/google/skia.git
    else
        git -C ./skia stash
        git -C ./skia checkout main
        git -C ./skia pull
    fi
    if [ $? -ne 0 ]; then
        sleep $retry_delay
        clone_skia
    fi
}
clone_skia
if [ ! -d "./skia/.git" ]; then
    echo "clone skia failed!"
    cd "$CURRENT_DIR"
    exit 1
fi

# Retry clone skia_compile
clone_skia_compile() {
    if [ ! -d "./skia_compile/.git" ]; then
        git clone https://github.com/rhett-lee/skia_compile.git
    else
        git -C ./skia_compile pull
    fi
    if [ $? -ne 0 ]; then
        sleep $retry_delay
        clone_skia_compile
    fi
}
clone_skia_compile
if [ ! -d "./skia_compile/.git" ]; then
    echo "clone skia_compile failed!"
    cd "$CURRENT_DIR"
    exit 1
fi

# Retry clone SDL
clone_SDL() {
    if [ ! -d "./SDL/.git" ]; then
        git clone https://github.com/libsdl-org/SDL.git
    else
        git -C ./SDL pull
    fi
    if [ $? -ne 0 ]; then
        sleep $retry_delay
        clone_SDL
    fi
}

if is_windows; then
    echo "ENABLE_SDL: $ENABLE_SDL"
fi

if ! is_windows || [ "$ENABLE_SDL" == "1" ]; then
    # clone SDL
    clone_SDL
    if [ ! -d "./SDL/.git" ]; then
        echo "clone SDL failed!"
        cd "$CURRENT_DIR"
        exit 1
    fi
fi

SKIA_PATCH_SRC_ZIP=skia.2025-06-06.src.zip
if [ ! -f "./skia_compile/$SKIA_PATCH_SRC_ZIP" ]; then
    echo "./skia_compile/$SKIA_PATCH_SRC_ZIP not found!"
    cd "$CURRENT_DIR"
    exit 1
fi

cd skia
git checkout 290495056ba5b737330ae7f2e6e722eeda9526f8
if [ $? -ne 0 ]; then
    echo "git checkout skia failed!"
    cd "$CURRENT_DIR"
    exit 1
fi
cd ..

unzip -o skia_compile/$SKIA_PATCH_SRC_ZIP -d ./skia/
if [ $? -ne 0 ]; then
    echo "./skia_compile/$SKIA_PATCH_SRC_ZIP unzip failed!"
    exit 1
fi

if [ "$has_clang" -eq 1 ]; then
    # clang/clang++
    echo "clang++ found at:"
    which clang++

    echo "clang found at:"
    which clang

    cd ./skia
    # Check if the system is FreeBSD
    if [ "$(uname)" = "FreeBSD" ]; then
        echo "Building for FreeBSD"
        gn gen out/llvm.${CPU_ARCH}.release --args="target_cpu=\"${CPU_ARCH}\" ar=\"llvm-ar\" skia_enable_fontmgr_fontconfig=true skia_use_freetype=true extra_ldflags = [ \"-L/usr/local/lib\" ] cc=\"clang\" cxx=\"clang++\" is_trivial_abi=false is_official_build=true skia_use_libwebp_encode=false skia_use_libwebp_decode=false skia_use_libpng_encode=false skia_use_libpng_decode=false skia_use_zlib=false skia_use_libjpeg_turbo_encode=false skia_use_libjpeg_turbo_decode=false skia_enable_fontmgr_win_gdi=false skia_use_icu=false skia_use_expat=false skia_use_xps=false skia_enable_pdf=false skia_use_wuffs=false skia_enable_svg=true skia_use_expat=true skia_use_system_expat=false is_debug=false extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\", \"-I/usr/local/include/freetype2\", \"-I/usr/local/include\"]"
    else
        gn gen out/llvm.${CPU_ARCH}.release --args="target_cpu=\"${CPU_ARCH}\" cc=\"clang\" cxx=\"clang++\" is_trivial_abi=false is_official_build=true skia_use_libwebp_encode=false skia_use_libwebp_decode=false skia_use_libpng_encode=false skia_use_libpng_decode=false skia_use_zlib=false skia_use_libjpeg_turbo_encode=false skia_use_libjpeg_turbo_decode=false skia_enable_fontmgr_win_gdi=false skia_use_icu=false skia_use_expat=false skia_use_xps=false skia_enable_pdf=false skia_use_wuffs=false skia_enable_svg=true skia_use_expat=true skia_use_system_expat=false is_debug=false extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\"]"
        
    fi
    ninja -C out/llvm.${CPU_ARCH}.release
    cd ..
else
    if [ "$has_gcc" -eq 1 ]; then
        # gcc/g++
        echo "g++ found at:"
        which g++

        echo "gcc found at:"
        which gcc

        cd ./skia
        gn gen out/gcc.${CPU_ARCH}.release --args="target_cpu=\"${CPU_ARCH}\" cc=\"gcc\" cxx=\"g++\" is_trivial_abi=false is_official_build=true skia_use_libwebp_encode=false skia_use_libwebp_decode=false skia_use_libpng_encode=false skia_use_libpng_decode=false skia_use_zlib=false skia_use_libjpeg_turbo_encode=false skia_use_libjpeg_turbo_decode=false skia_enable_fontmgr_win_gdi=false skia_use_icu=false skia_use_expat=false skia_use_xps=false skia_enable_pdf=false skia_use_wuffs=false skia_enable_svg=true skia_use_expat=true skia_use_system_expat=false is_debug=false extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\"]"
        ninja -C out/gcc.${CPU_ARCH}.release
        cd ..
    fi
fi

if ! is_windows; then
    # build SDL on Linux/MacOS
    cmake --fresh -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DCMAKE_BUILD_TYPE=Release 
    cmake --build ./SDL.build
    cmake --install ./SDL.build
elif [ "$ENABLE_SDL" == "1" ]; then
    # build SDL on Windows
    if [ "$has_clang" -eq 1 ]; then
        DUILIB_SDL_DIR=SDL.build.msys2.llvm
    else
        DUILIB_SDL_DIR=SDL.build.msys2.gcc
    fi
    cmake --fresh -S "./SDL/" -B "./${DUILIB_SDL_DIR}" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST_LIBRARY=OFF -DCMAKE_BUILD_TYPE=Release 
    cmake --build ./${DUILIB_SDL_DIR} -j 6
    cmake --install ./${DUILIB_SDL_DIR}
fi

# build nim_duilib
if [ "$(uname -s)" == "Darwin" ]; then
    echo "macOS"
    chmod +x ./nim_duilib/build/macos_build.sh
    ./nim_duilib/build/macos_build.sh
elif [ "$(uname -s)" == "FreeBSD" ]; then
    echo "FreeBSD"
    chmod +x ./nim_duilib/build/freebsd_build.sh
    ./nim_duilib/build/freebsd_build.sh
elif is_windows; then
    echo "Windows"
    ./nim_duilib/build/msys2_build.sh $1
else
    echo "Linux"
    chmod +x ./nim_duilib/build/linux_build.sh
    ./nim_duilib/build/linux_build.sh
fi

cd "$CURRENT_DIR"
echo

end_time=$(date +%s)
duration=$((end_time - start_time))
if (( duration >= 60 )); then
    minutes=$((duration / 60))
    echo "Execution completed in $minutes minute(s)"
else
    echo "Execution completed in $duration second(s)"
fi
