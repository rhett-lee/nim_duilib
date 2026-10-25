@echo off
setlocal enabledelayedexpansion

REM ==============================================
REM Visual Studio Version Detection Script
REM Detects the LATEST installed Visual Studio
REM Priority: VSWhere (latest) -> Directory fallback
REM Sets VS_VERSION, VS_PATH, VS_MAJOR_VERSION
REM ==============================================

set "VS_VERSION=vs2022"
set "VS_PATH="
set "VS_MAJOR_VERSION=0"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

REM ==============================================
REM Step 1: Use vswhere.exe with -latest (HIGHEST PRIORITY)
REM This ensures we get the NEWEST Visual Studio installation
REM ==============================================
if exist "%VSWHERE%" (
    for /f "delims=" %%p in ('"%VSWHERE%" -nologo -latest -prerelease -legacy -products * -sort -utf8 -property installationPath 2^>nul') do (
        set "VS_PATH=%%p"
    )

    for /f "delims=" %%v in ('"%VSWHERE%" -nologo -latest -prerelease -legacy -products * -sort -utf8 -property installationVersion 2^>nul') do (
        set "RAW_VERSION=%%v"
        for /f "delims=." %%m in ("!RAW_VERSION!") do set "VS_MAJOR_VERSION=%%m"
    )

    if !VS_MAJOR_VERSION! GEQ 18 (
        set "VS_VERSION=vs2026"
    ) else if !VS_MAJOR_VERSION! GEQ 17 (
        set "VS_VERSION=vs2022"
    ) else if !VS_MAJOR_VERSION! GEQ 16 (
        set "VS_VERSION=vs2019"
    ) else if !VS_MAJOR_VERSION! GEQ 15 (
        set "VS_VERSION=vs2017"
    )
)

REM ==============================================
REM Step 2: Fallback - Directory traversal (NEWEST to OLDEST order)
REM Only run if vswhere failed or returned no path
REM ==============================================
if "%VS_PATH%" == "" (
    set DEFAULT_DIRS="C:\Program Files\Microsoft Visual Studio" "C:\Program Files (x86)\Microsoft Visual Studio"
    set "RELEASES=18 2022 2019 2017"
    set "VERSIONS=Enterprise Professional Community BuildTools Preview"
    for %%d in (!DEFAULT_DIRS!) do (
        for %%r in (!RELEASES!) do (
            for %%v in (!VERSIONS!) do (
                set "CHECK_PATH=%%~d\%%r\%%v\VC"
                if exist "!CHECK_PATH!" (
                    set "VS_PATH=%%~d\%%r\%%v"
                    set "RELEASE_VER=%%r"

                    if "!RELEASE_VER!"=="18" (
                        set "VS_VERSION=vs2026"
                        set "VS_MAJOR_VERSION=18"
                    ) else if "!RELEASE_VER!"=="2022" (
                        set "VS_VERSION=vs2022"
                        set "VS_MAJOR_VERSION=17"
                    ) else if "!RELEASE_VER!"=="2019" (
                        set "VS_VERSION=vs2019"
                        set "VS_MAJOR_VERSION=16"
                    ) else if "!RELEASE_VER!"=="2017" (
                        set "VS_VERSION=vs2017"
                        set "VS_MAJOR_VERSION=15"
                    )
                    goto :found
                )
            )
        )
    )
)

:found
REM Export variables to the parent environment
endlocal & set "VS_VERSION=%VS_VERSION%" & set "VS_PATH=%VS_PATH%" & set "VS_MAJOR_VERSION=%VS_MAJOR_VERSION%"

REM Print final detection result
echo.
echo ==============================================
echo Latest Visual Studio Detected:
echo   VS_VERSION        = %VS_VERSION%
echo   VS_MAJOR_VERSION  = %VS_MAJOR_VERSION%
echo   VS_PATH           = %VS_PATH%
echo ==============================================
echo.