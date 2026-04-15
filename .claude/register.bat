@echo off
setlocal
chcp 65001 >nul 2>&1
REM nim_duilib AI toolchain - global register script (Windows)
REM Usage: cd nim_duilib && .claude\register.bat
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0register.ps1"
endlocal
