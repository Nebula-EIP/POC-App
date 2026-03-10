@echo off
setlocal enabledelayedexpansion

color 0A

echo.
echo ========================================
echo Development Build ^& Test Nebula
echo ========================================
echo.

echo Select build type:
echo 1) Debug (default)
echo 2) Release
set /p build_choice="Enter choice (1 or 2): "

if "%build_choice%"=="" set build_choice=1
if "%build_choice%"=="1" (
    set BUILD_TYPE=Debug
) else if "%build_choice%"=="2" (
    set BUILD_TYPE=Release
) else (
    color 0C
    echo Invalid choice
    exit /b 1
)

echo.
echo Select format mode:
echo 1) Check (dry-run, default)
echo 2) Fix (modify files)
set /p format_choice="Enter choice (1 or 2): "

if "%format_choice%"=="" set format_choice=1
if "%format_choice%"=="1" (
    set FORMAT_MODE=check
) else if "%format_choice%"=="2" (
    set FORMAT_MODE=fix
) else (
    color 0C
    echo Invalid choice
    exit /b 1
)

color 0A
echo.
echo Building %BUILD_TYPE% with format mode: %FORMAT_MODE%
echo.

echo [1/5] Configure...
call .\scripts\windows\configure.bat %BUILD_TYPE%
if errorlevel 1 exit /b 1

echo [2/5] Format (%FORMAT_MODE%)...
call .\scripts\windows\format.bat %FORMAT_MODE%
if errorlevel 1 exit /b 1

echo [3/5] Lint...
call .\scripts\windows\lint.bat
if errorlevel 1 exit /b 1

echo [4/5] Build (%BUILD_TYPE%)...
call .\scripts\windows\build.bat %BUILD_TYPE%
if errorlevel 1 exit /b 1

echo [5/5] Test (%BUILD_TYPE%)...
call .\scripts\windows\test.bat %BUILD_TYPE%
if errorlevel 1 exit /b 1

color 0A
echo.
echo ========================================
echo Development Completed
echo ========================================
echo.

exit /b 0
