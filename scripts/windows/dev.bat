@echo off
setlocal enabledelayedexpansion

for /F %%a in ('echo prompt $E ^| cmd /Q /D /A /C') do set "ESC=%%a"
set "GREEN=%ESC%[0;32m"
set "YELLOW=%ESC%[1;33m"
set "RED=%ESC%[0;31m"
set "BLUE=%ESC%[0;34m"
set "NC=%ESC%[0m"

echo.
echo %BLUE%========================================%NC%
echo %BLUE%  Development Build ^& Test Nebula%NC%
echo %BLUE%========================================%NC%
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
    echo %RED%Invalid choice%NC%
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
    echo %RED%Invalid choice%NC%
    exit /b 1
)

echo.
echo %BLUE%Building %BUILD_TYPE% with format mode: %FORMAT_MODE%%NC%
echo.

echo %YELLOW%[1/5] Configure...%NC%
call .\scripts\windows\configure.bat %BUILD_TYPE%
if errorlevel 1 exit /b 1

echo %YELLOW%[2/5] Format (%FORMAT_MODE%)...%NC%
call .\scripts\windows\format.bat %FORMAT_MODE%
if errorlevel 1 exit /b 1

echo %YELLOW%[3/5] Lint...%NC%
call .\scripts\windows\lint.bat
if errorlevel 1 exit /b 1

echo %YELLOW%[4/5] Build (%BUILD_TYPE%)...%NC%
call .\scripts\windows\build.bat %BUILD_TYPE%
if errorlevel 1 exit /b 1

echo %YELLOW%[5/5] Test (%BUILD_TYPE%)...%NC%
call .\scripts\windows\test.bat %BUILD_TYPE%
if errorlevel 1 exit /b 1

echo.
echo %GREEN%========================================%NC%
echo %GREEN%  Development Completed%NC%
echo %GREEN%========================================%NC%
echo.

exit /b 0
