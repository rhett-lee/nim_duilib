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
echo "- Cloning nim_duilib ..."
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
echo "- Cloning skia ..."
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
echo "- Cloning skia_compile ..."
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
echo "- Cloning SDL ..."
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

SKIA_PATCH_SRC_ZIP=skia.2025-11-01.src.zip
if [ ! -f "./skia_compile/$SKIA_PATCH_SRC_ZIP" ]; then
    echo "./skia_compile/$SKIA_PATCH_SRC_ZIP not found!"
    cd "$CURRENT_DIR"
    exit 1
fi

cd skia
git checkout c5cd862d6fa511be244f7c2db1fe05563ff8fc72
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

# download CEF
# Function: Download HTTPS files with curl (supports auto-retry on failure)
# Parameter 1: Target URL (required, HTTPS protocol)
# Parameter 2: Save path (optional, default: current directory, filename extracted from URL)
# Parameter 3: Max retry attempts (optional, default: 3 times)
# Parameter 4: Single request timeout (optional, default: 15 seconds)
curl_download_with_retry() {
    # Validate required parameter
    if [ -z "$1" ]; then
        echo "Error: Target URL (1st parameter) not specified"
        return 1
    fi

    # Initialize parameters with default values
    local url="$1"
    local save_path="${2:-$(basename "$url")}"  # Default: use filename from URL
    local max_retries="${3:-3}"
    local timeout="${4:-15}"
    local retry_count=0

    echo "=== Starting download(curl): $url ==="
    echo "Save path: $save_path"
    echo "Max retries: $max_retries, Timeout per attempt: $timeout seconds"

    # Loop for download with retries
    while [ $retry_count -le $max_retries ]; do
        # Core curl parameters explanation:
        # -L: Follow 301/302 redirects (common for HTTPS)
        # -f: Return non-zero exit code on failure (for error checking)
        # -sS: Silent mode (suppress redundant output, show progress only)
        # -o: Specify output file path
        # --retry: Number of retries (only for network errors)
        # --connect-timeout: Timeout for establishing connection
        curl -L -f -o "$save_path" \
            --retry "$max_retries" \
            --connect-timeout "$timeout" \
            --retry-delay 30 --speed-limit 100 --speed-time 120 \
            "$url"

        # Check download result
        if [ $? -eq 0 ]; then
            echo "Download successful! File saved to: $save_path"
            return 0
        else
            retry_count=$((retry_count + 1))
            if [ $retry_count -le $max_retries ]; then
                echo "$retry_count-th download failed. $((max_retries - retry_count)) retries left. Retrying in 10 seconds..."
                sleep 10
            else
                echo "All retries failed (total $max_retries attempts). Please check URL validity or network connection."
                return 1
            fi
        fi
    done
}

# Function: Download HTTPS files with wget (supports auto-retry on failure)
# Parameter 1: Target URL (required, HTTPS protocol)
# Parameter 2: Save path (optional, default: current directory, filename extracted from URL)
# Parameter 3: Max retry attempts (optional, default: 3 times)
# Parameter 4: Single request timeout (optional, default: 15 seconds)
wget_download_with_retry() {
    # Validate required parameter
    if [ -z "$1" ]; then
        echo "Error: Target URL (1st parameter) not specified"
        return 1
    fi

    # Initialize parameters with default values
    local url="$1"
    local save_path="${2:-$(basename "$url")}"  # Default: use filename from URL
    local max_retries="${3:-3}"
    local timeout="${4:-15}"

    echo "=== Starting download(wget): $url ==="
    echo "Save path: $save_path"
    echo "Max retries: $max_retries, Timeout per attempt: $timeout seconds"

    # Core wget parameters explanation:
    # -O: Specify output file path (uppercase O, distinguish from lowercase -o for logs)
    # -t: Number of retries (0 = unlimited, here use max_retries)
    # --timeout: Timeout for connection and data transfer (in seconds)
    # --no-check-certificate: Optional (disable HTTPS certificate check, for test only)
    # -q: Quiet mode (suppress redundant output)
    # -c: Resume broken download (supports resuming if download is interrupted)
    wget -O "$save_path" \
         -t "$max_retries" \
         --timeout "$timeout" \
         -c \
         "$url"

    # Check download result
    if [ $? -eq 0 ]; then
        echo "Download successful! File saved to: $save_path"
        return 0
    else
        echo "All retries failed (total $max_retries attempts). Please check URL validity or network connection."
        return 1
    fi
}

# flag
has_curl=0
has_wget=0
has_linux=0
has_macos=0

# curl
if command -v curl &> /dev/null; then
    has_curl=1
fi

# wget
if command -v wget &> /dev/null; then
    has_wget=1
fi

if [ "$(uname -s)" == "Darwin" ]; then
    has_macos=1
elif [ "$(uname -s)" == "FreeBSD" ]; then
    has_linux=0
elif is_windows; then
    has_linux=0
else
    has_linux=1
fi

if [ "$has_curl$has_wget" != "00" ] && [ "$has_linux$has_macos" != "00" ]; then
    # download CEF on Linux and MacOS
    libcef_linux_dest_dir=./nim_duilib/bin/libcef_linux
    libcef_cef_binary_dir=./cef_binary
    
    # libcef file name prefix
    libcef_file_name_prefix=cef_binary_142.0.10+g29548e2+chromium-142.0.7444.135
    if [ "$has_linux" == "1" ]; then
        # Linux
        if [ "$CPU_ARCH" == "arm64" ]; then
            libcef_file_name="${libcef_file_name_prefix}_linuxarm64_minimal"
        else
            libcef_file_name="${libcef_file_name_prefix}_linux64_minimal"
        fi
    else
        # MacOS
        if [ "$CPU_ARCH" == "arm64" ]; then
            libcef_file_name="${libcef_file_name_prefix}_macosarm64_minimal"
        else
            libcef_file_name="${libcef_file_name_prefix}_macosx64_minimal"
        fi
    fi

    if [ "$libcef_file_name" != "" ]; then
        # download .tar.bz2
        libcef_local_file=${libcef_file_name}.tar.bz2
        if [ "$has_curl" == "1" ]; then
            curl_download_with_retry "https://cef-builds.spotifycdn.com/${libcef_file_name}.tar.bz2" "$libcef_local_file" 100 30
        else
            wget_download_with_retry "https://cef-builds.spotifycdn.com/${libcef_file_name}.tar.bz2" "$libcef_local_file" 100 30
        fi
        
        if [ $? -eq 0 ]; then
            if [ -f "$libcef_local_file" ]; then
                # Linux and MacOS: extract .tar.bz2
                echo "Extracting: ${libcef_local_file} ..."
                tar -xjf "$libcef_local_file"
                mv "$libcef_file_name" "$libcef_cef_binary_dir"
                echo "Extracted: ${libcef_local_file}."
                
                if [ "$has_linux" == "1" ]; then
                    # Linux
                    mkdir -p $libcef_linux_dest_dir
                    cp -rf $libcef_cef_binary_dir/Release/* $libcef_linux_dest_dir
                    cp -rf $libcef_cef_binary_dir/Resources/* $libcef_linux_dest_dir
                fi
            fi
        fi
    fi
fi
# download CEF end

echo "- Building skia ..."
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

cmake_version=$(cmake --version | grep -oE '[0-9]+\.[0-9]+')
required_version=3.24
if [ $(echo "$cmake_version >= $required_version" | bc) -eq 1 ]; then
    DUILIB_CMAKE_REFRESH=--fresh
else
    DUILIB_CMAKE_REFRESH=
fi

if ! is_windows; then
    # build SDL on Linux/MacOS
    echo "- Building SDL ..."
    cmake ${DUILIB_CMAKE_REFRESH} -S "./SDL/" -B "./SDL.build" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF -DCMAKE_BUILD_TYPE=Release
    cmake --build ./SDL.build
    cmake --install ./SDL.build
elif [ "$ENABLE_SDL" == "1" ]; then
    # build SDL on Windows
    echo "- Building SDL ..."
    if [ "$has_clang" -eq 1 ]; then
        DUILIB_SDL_DIR=SDL.build.msys2.llvm
    else
        DUILIB_SDL_DIR=SDL.build.msys2.gcc
    fi
    cmake ${DUILIB_CMAKE_REFRESH} -S "./SDL/" -B "./${DUILIB_SDL_DIR}" -DCMAKE_INSTALL_PREFIX="./SDL3/" -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST_LIBRARY=OFF -DCMAKE_BUILD_TYPE=Release 
    cmake --build ./${DUILIB_SDL_DIR} -j 6
    cmake --install ./${DUILIB_SDL_DIR}
fi

# build nim_duilib
echo "- Building nim_duilib ..."
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
