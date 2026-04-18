@echo off
chcp 65001 >nul
setlocal

set "SCRIPT_DIR=%~dp0"
set "FLAG_FILE=%SCRIPT_DIR%duilib_dll.flag"

echo ==============================================
echo  "切换到：动态运行库模式（/MD /MDd）"
echo ==============================================

echo. > "%FLAG_FILE%"
if exist "%FLAG_FILE%" (
    echo "已创建：%FLAG_FILE%"
    echo "运行库模式已经切换到动态运行库（/MD /MDd）！"
) else (
    echo "切换运行库模式失败，请手工创建文件：%FLAG_FILE%！"
)
echo.

if "%1" NEQ "/S" (
    echo "请关闭 VS 后重新打开工程。"
    echo "本脚本将在 7 秒后自动退出..."
    echo.
    timeout /t 7 >nul
)

endlocal
