echo OFF

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

set SKIA_PATCH_SRC_ZIP=skia.2025-06-06.src.zip
if not exist ".\skia_compile\%SKIA_PATCH_SRC_ZIP%" (
    echo ".\skia_compile\%SKIA_PATCH_SRC_ZIP%" not found!
    cd /d %CURRENT_DIR%
    exit /b 1
)

cd skia
git checkout 290495056ba5b737330ae7f2e6e722eeda9526f8
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

@REM build skia
cd skia

if %has_clang% equ 1 (
    .\bin\gn.exe gen out/llvm.x64.release --args="target_cpu=\"x64\" cc=\"clang\" cxx=\"clang++\" is_trivial_abi=false is_official_build=true skia_use_libwebp_encode=false skia_use_libwebp_decode=false skia_use_libpng_encode=false skia_use_libpng_decode=false skia_use_zlib=false skia_use_libjpeg_turbo_encode=false skia_use_libjpeg_turbo_decode=false skia_enable_fontmgr_win_gdi=false skia_use_icu=false skia_use_expat=false skia_use_xps=false skia_enable_pdf=false skia_use_wuffs=false skia_enable_svg=true skia_use_expat=true skia_use_system_expat=false is_debug=false extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\"]"
) else (
    .\bin\gn.exe gen out/gcc.x64.release --args="target_cpu=\"x64\" cc=\"gcc\" cxx=\"g++\" is_trivial_abi=false is_official_build=true skia_use_libwebp_encode=false skia_use_libwebp_decode=false skia_use_libpng_encode=false skia_use_libpng_decode=false skia_use_zlib=false skia_use_libjpeg_turbo_encode=false skia_use_libjpeg_turbo_decode=false skia_enable_fontmgr_win_gdi=false skia_use_icu=false skia_use_expat=false skia_use_xps=false skia_enable_pdf=false skia_use_wuffs=false skia_enable_svg=true skia_use_expat=true skia_use_system_expat=false is_debug=false extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\"]"
)
cd ..

@REM build nim_duilib
cd /d %SCRIPT_DIR%
if %has_clang% equ 1 (
    call .\llvm-mingw-w64_build.bat
) else (
    call .\gcc-mingw-w64_build.bat
)

cd /d %CURRENT_DIR%
echo.
echo finished.
