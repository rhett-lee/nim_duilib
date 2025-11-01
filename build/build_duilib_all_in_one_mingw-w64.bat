echo OFF

:: Force enable SDL
set ENABLE_SDL=0
if "%1" == "-sdl" (
    set ENABLE_SDL=1
)
echo ENABLE_SDL: %ENABLE_SDL%

:: build duilib: MinGW-w64 gcc/g++ or clang/clang++
set CURRENT_DIR=%cd%
set SCRIPT_DIR=%~dp0

echo Checking the necessary software
where git.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo git.exe found at:  
    where git.exe
) else (
    echo git.exe not found in PATH
    cd /d %CURRENT_DIR%
    exit /b 1
)

where python3.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo python3.exe found at:  
    where python3.exe
) else (
    echo python3.exe not found in PATH
    cd /d %CURRENT_DIR%
    exit /b 1
)

where cmake.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo cmake.exe found at:  
    where cmake.exe
) else (
    echo cmake.exe not found in PATH
    cd /d %CURRENT_DIR%
    exit /b 1
)

set has_gcc=0
set has_clang=0

:: gcc/g++
where gcc >nul 2>&1 && where g++ >nul 2>&1 && set has_gcc=1

:: clang/clang++
where clang >nul 2>&1 && where clang++ >nul 2>&1 && set has_clang=1

if %has_gcc%%has_clang% equ 00 (
    echo  "GCC/G++ (MinGW) not found in PATH"
    echo  "Clang/Clang++ (LLVM) not found in PATH"
    cd /d %CURRENT_DIR%
    exit /b 1
)

if %has_clang% equ 1 (
    where clang
    where clang++
) else (
    where gcc
    where g++
)

cd /d %SCRIPT_DIR%
echo %cd%
if not exist ".\nim_duilib\.git" (
    if exist "..\..\nim_duilib\.git" (
        cd ..\..\
    )
)
echo %cd%

set retry_delay=10

echo - Cloning nim_duilib ...
:retry_clone_duilib
if not exist ".\nim_duilib\.git" (
    git clone https://github.com/rhett-lee/nim_duilib
) else (
    git -C ./nim_duilib pull
)
if %errorlevel% neq 0 (
    timeout /t %retry_delay% >nul
    goto retry_clone_duilib
)

if not exist ".\nim_duilib\.git" (
    echo clone retry_clone_skia_compile failed!
    cd /d %CURRENT_DIR%
    exit /b 1
)

echo - Cloning skia_compile ...
:retry_clone_skia_compile
if not exist ".\skia_compile\.git" (
    git clone https://github.com/rhett-lee/skia_compile
) else (    
    git -C ./skia_compile pull
)
if %errorlevel% neq 0 (
    timeout /t %retry_delay% >nul
    goto retry_clone_skia_compile
)

if not exist ".\skia_compile\.git" (
    echo clone retry_clone_skia_compile failed!
    cd /d %CURRENT_DIR%
    exit /b 1
)

echo - Cloning skia ...
:retry_clone_skia
if not exist ".\skia\.git" (
    git clone https://github.com/google/skia.git
) else (
    git -C ./skia stash
    git -C ./skia checkout main
    git -C ./skia pull
)
if %errorlevel% neq 0 (
    timeout /t %retry_delay% >nul
    goto retry_clone_skia
)

if not exist ".\skia\.git" (
    echo clone skia failed!
    cd /d %CURRENT_DIR%
    exit /b 1
)

if %ENABLE_SDL% equ 1 (
echo - Cloning SDL ...
:retry_clone_SDL
    if not exist ".\SDL\.git" (
        git clone https://github.com/libsdl-org/SDL.git
    ) else (    
        git -C ./SDL pull
    )
    if %errorlevel% neq 0 (
        timeout /t %retry_delay% >nul
        goto retry_clone_SDL
    )
)

set SKIA_PATCH_SRC_ZIP=skia.2025-11-01.src.zip
if not exist ".\skia_compile\%SKIA_PATCH_SRC_ZIP%" (
    echo ".\skia_compile\%SKIA_PATCH_SRC_ZIP%" not found!
    cd /d %CURRENT_DIR%
    exit /b 1
)

cd skia
git checkout c5cd862d6fa511be244f7c2db1fe05563ff8fc72
if %errorlevel% neq 0 (
    echo git checkout skia failed!
    cd /d %CURRENT_DIR%
    exit /b 1
)
cd ..

.\skia_compile\windows\bin\miniunz.exe -o skia_compile/%SKIA_PATCH_SRC_ZIP% -d ./skia/
if %errorlevel% neq 0 (
    echo ".\skia_compile\%SKIA_PATCH_SRC_ZIP%" Expand-Archive failed!
    cd /d %CURRENT_DIR%
    exit /b 1
)

echo - Building skia ...
cd skia

if %has_clang% equ 1 (
    if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
        .\bin\gn.exe gen out/mingw64-llvm.x64.release --args="target_cpu=\"x64\" cc=\"clang\" cxx=\"clang++\" is_trivial_abi=false is_official_build=true skia_use_libwebp_encode=false skia_use_libwebp_decode=false skia_use_libpng_encode=false skia_use_libpng_decode=false skia_use_zlib=false skia_use_libjpeg_turbo_encode=false skia_use_libjpeg_turbo_decode=false skia_enable_fontmgr_win_gdi=false skia_use_icu=false skia_use_expat=false skia_use_xps=false skia_enable_pdf=false skia_use_wuffs=false skia_enable_svg=true skia_use_expat=true skia_use_system_expat=false is_debug=false extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\"]"
        .\bin\ninja.exe -C out/mingw64-llvm.x64.release
    ) else (
        .\bin\gn.exe gen out/mingw64-llvm.x86.release --args="target_cpu=\"x86\" cc=\"clang\" cxx=\"clang++\" is_trivial_abi=false is_official_build=true skia_use_libwebp_encode=false skia_use_libwebp_decode=false skia_use_libpng_encode=false skia_use_libpng_decode=false skia_use_zlib=false skia_use_libjpeg_turbo_encode=false skia_use_libjpeg_turbo_decode=false skia_enable_fontmgr_win_gdi=false skia_use_icu=false skia_use_expat=false skia_use_xps=false skia_enable_pdf=false skia_use_wuffs=false skia_enable_svg=true skia_use_expat=true skia_use_system_expat=false is_debug=false extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\"]"
        .\bin\ninja.exe -C out/mingw64-llvm.x86.release
    )
) else (
    if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
        .\bin\gn.exe gen out/mingw64-gcc.x64.release --args="target_cpu=\"x64\" cc=\"gcc\" cxx=\"g++\" is_trivial_abi=false is_official_build=true skia_use_libwebp_encode=false skia_use_libwebp_decode=false skia_use_libpng_encode=false skia_use_libpng_decode=false skia_use_zlib=false skia_use_libjpeg_turbo_encode=false skia_use_libjpeg_turbo_decode=false skia_enable_fontmgr_win_gdi=false skia_use_icu=false skia_use_expat=false skia_use_xps=false skia_enable_pdf=false skia_use_wuffs=false skia_enable_svg=true skia_use_expat=true skia_use_system_expat=false is_debug=false extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\"]"
        .\bin\ninja.exe -C out/mingw64-gcc.x64.release
    ) else (
        .\bin\gn.exe gen out/mingw64-gcc.x86.release --args="target_cpu=\"x86\" cc=\"gcc\" cxx=\"g++\" is_trivial_abi=false is_official_build=true skia_use_libwebp_encode=false skia_use_libwebp_decode=false skia_use_libpng_encode=false skia_use_libpng_decode=false skia_use_zlib=false skia_use_libjpeg_turbo_encode=false skia_use_libjpeg_turbo_decode=false skia_enable_fontmgr_win_gdi=false skia_use_icu=false skia_use_expat=false skia_use_xps=false skia_enable_pdf=false skia_use_wuffs=false skia_enable_svg=true skia_use_expat=true skia_use_system_expat=false is_debug=false extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\"]"
        .\bin\ninja.exe -C out/mingw64-gcc.x86.release
    )
)
cd ..

if %ENABLE_SDL% equ 1 (
    echo - Building SDL ...
    if %has_clang% equ 1 (
        SET DUILIB_CC=clang
        SET DUILIB_CXX=clang++
        SET DUILIB_SDL_DIR=SDL.build.mingw.llvm
    ) else (
        SET DUILIB_CC=gcc
        SET DUILIB_CXX=g++
        SET DUILIB_SDL_DIR=SDL.build.mingw.gcc
    )

    cmake --fresh -S "./SDL/" -B "./%DUILIB_SDL_DIR%" -DCMAKE_INSTALL_PREFIX="./SDL3/" -G"MinGW Makefiles" -DCMAKE_C_COMPILER=%DUILIB_CC% -DCMAKE_CXX_COMPILER=%DUILIB_CXX% -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST_LIBRARY=OFF -DCMAKE_BUILD_TYPE=Release 
    cmake --build ./%DUILIB_SDL_DIR% -j 6
    cmake --install ./%DUILIB_SDL_DIR%
)

echo - Building nim_duilib ...
cd /d %SCRIPT_DIR%
if %has_clang% equ 1 (
    call .\llvm-mingw-w64_build.bat %1
) else (
    call .\gcc-mingw-w64_build.bat %1
)

cd /d %CURRENT_DIR%
echo.
echo finished.
