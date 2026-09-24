@echo off
setlocal
chcp 65001 >nul 2>&1
REM nim_duilib AI toolchain - global unregister script (Windows)
REM Usage: cd nim_duilib && .claude\unregister.bat
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0unregister.ps1"
endlocal
