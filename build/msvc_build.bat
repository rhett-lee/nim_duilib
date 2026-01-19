@echo OFF
@REM Windows 平台编译脚本，编译器：Visual Studio 2022

@REM 确保本机已经安装了camke(最低版本号: v3.21)
@REM 可使用 cmake --version 查看版本信息（本机: cmake version 4.0.2）

@for %%i in ("%~dp0..\") do set "bat_parent_dir=%%~fi"
SET DUILIB_SRC_ROOT_DIR=%bat_parent_dir%
echo DUILIB_SRC_ROOT_DIR: "%DUILIB_SRC_ROOT_DIR%"

@for %%i in ("%~dp0..\..\skia") do set "bat_parent_dir=%%~fi"
SET SKIA_SRC_ROOT_DIR=%bat_parent_dir%
echo SKIA_SRC_ROOT_DIR: "%SKIA_SRC_ROOT_DIR%"

@REM # 设置编译器
SET DUILIB_COMPILER_ID=msvc

SET DUILIB_CMAKE=cmake --fresh -G"Visual Studio 17 2022"
SET DUILIB_MAKE=cmake --build
SET DUILIB_BUILD_TYPE=Release
SET DUILIB_BUILD_PARAM=--config %DUILIB_BUILD_TYPE%

@REM # Skia库的子目录，固定使用LLVM编译的，如果想使用默认规则，可注释掉这段
SET CPU_ARCH=%VSCMD_ARG_TGT_ARCH%
if "%CPU_ARCH%"=="" (
    SET CPU_ARCH=x64
)
SET DUILIB_SKIA_LIB_SUBPATH=llvm.%CPU_ARCH%.release
echo "DUILIB_SKIA_LIB_SUBPATH:%DUILIB_SKIA_LIB_SUBPATH%"

if not exist "%SKIA_SRC_ROOT_DIR%\out\%DUILIB_SKIA_LIB_SUBPATH%" (
    echo "Please compile the skia first or run build_duilib_all_in_one.bat."
    exit /b 1
)

if "%CPU_ARCH%"=="x86" (
    SET DUILIB_CMAKE=%DUILIB_CMAKE% -A Win32
)

@REM # lib目录
SET DUILIB_LIB_DIR=%DUILIB_SRC_ROOT_DIR%\lib\
SET target_dir=%DUILIB_LIB_DIR%
if not exist "%target_dir%" (
    @mkdir "%target_dir%"
)

@REM # 编译临时目录
SET DUILIB_BUILD_DIR=%DUILIB_SRC_ROOT_DIR%\build\build_temp\%DUILIB_COMPILER_ID%
SET target_dir=%DUILIB_SRC_ROOT_DIR%\build\build_temp\
if not exist "%target_dir%" (
    @mkdir "%target_dir%"
)
if not exist "%DUILIB_BUILD_DIR%" (
    @mkdir "%DUILIB_BUILD_DIR%"
)

@REM # 编译第三方库   
SET DUILIB_THIRD_PARTY_LIBS=zlib,libpng,cximage,libwebp,libcef\libcef_win
for %%i in (%DUILIB_THIRD_PARTY_LIBS%) do (
    %DUILIB_CMAKE% -S "%DUILIB_SRC_ROOT_DIR%duilib\third_party\%%i" -B "%DUILIB_BUILD_DIR%\%%i" -DCMAKE_BUILD_TYPE=%DUILIB_BUILD_TYPE%
    %DUILIB_MAKE% "%DUILIB_BUILD_DIR%\%%i" %DUILIB_BUILD_PARAM%
)

@REM #编译duilib
%DUILIB_CMAKE% -S "%DUILIB_SRC_ROOT_DIR%duilib" -B "%DUILIB_BUILD_DIR%\duilib" -DCMAKE_BUILD_TYPE=%DUILIB_BUILD_TYPE%
%DUILIB_MAKE% "%DUILIB_BUILD_DIR%\duilib" %DUILIB_BUILD_PARAM%

@REM #编译examples下的各个程序
SET DUILIB_PROGRAMS=basic,controls,ColorPicker,DpiAware,chat,layout,ListBox,ListCtrl,MoveControl,MultiLang,render,RichEdit,VirtualListBox,threads,TreeView,cef,CefBrowser,WebView2,WebView2Browser,ChildWindow
for %%i in (%DUILIB_PROGRAMS%) do (
    %DUILIB_CMAKE% -S "%DUILIB_SRC_ROOT_DIR%examples\%%i" -B "%DUILIB_BUILD_DIR%\%%i" -DCMAKE_BUILD_TYPE=%DUILIB_BUILD_TYPE% -DDUILIB_SKIA_LIB_SUBPATH=%DUILIB_SKIA_LIB_SUBPATH%
    %DUILIB_MAKE% "%DUILIB_BUILD_DIR%\%%i" %DUILIB_BUILD_PARAM%
)
