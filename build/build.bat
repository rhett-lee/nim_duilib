@echo off
rem nim_duilib 统一构建入口的 Windows 薄封装。
rem 仅负责定位 Python 3，其余逻辑全部在 build.py。
setlocal

set "PYTHON_EXE="
where py >nul 2>nul
if not errorlevel 1 (
    py -3 -c "import sys" >nul 2>nul
    if not errorlevel 1 set "PYTHON_EXE=py -3"
)
if not defined PYTHON_EXE (
    where python >nul 2>nul
    if not errorlevel 1 (
        python -c "import sys; sys.exit(0 if sys.version_info[0] >= 3 else 1)" >nul 2>nul
        if not errorlevel 1 set "PYTHON_EXE=python"
    )
)
if not defined PYTHON_EXE (
    where python3 >nul 2>nul
    if not errorlevel 1 set "PYTHON_EXE=python3"
)
if not defined PYTHON_EXE (
    echo [ERROR] Python 3 was not found in PATH.
    echo         Install Python 3 from https://www.python.org/downloads/
    echo         and enable "Add python.exe to PATH" during installation.
    exit /b 1
)

%PYTHON_EXE% "%~dp0build.py" %*
exit /b %ERRORLEVEL%
