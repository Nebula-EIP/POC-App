@echo off
REM Build script for Windows

setlocal enabledelayedexpansion

REM Default values
set BUILD_TYPE=Release
set CLEAN_BUILD=0
set RUN_AFTER_BUILD=0
set BUILD_DIR=build
set GENERATOR=

REM Parse arguments
:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="-d" set BUILD_TYPE=Debug& shift & goto parse_args
if /i "%~1"=="--debug" set BUILD_TYPE=Debug& shift & goto parse_args
if /i "%~1"=="-r" set BUILD_TYPE=Release& shift & goto parse_args
if /i "%~1"=="--release" set BUILD_TYPE=Release& shift & goto parse_args
if /i "%~1"=="-c" set CLEAN_BUILD=1& shift & goto parse_args
if /i "%~1"=="--clean" set CLEAN_BUILD=1& shift & goto parse_args
if /i "%~1"=="--run" set RUN_AFTER_BUILD=1& shift & goto parse_args
if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help
if /i "%~1"=="--mingw" set GENERATOR=-G "MinGW Makefiles"& shift & goto parse_args
if /i "%~1"=="--vs" set GENERATOR=-G "Visual Studio 17 2022"& shift & goto parse_args

echo Unknown option: %~1
echo Use --help for usage information
exit /b 1

:show_help
echo Usage: build.bat [OPTIONS]
echo.
echo Options:
echo   -d, --debug     Build in Debug mode
echo   -r, --release   Build in Release mode (default)
echo   -c, --clean     Clean build directory before building
echo   --run           Run the editor executable after building
echo   --mingw         Use MinGW Makefiles generator
echo   --vs            Use Visual Studio 2022 generator
echo   -h, --help      Show this help message
exit /b 0

:end_parse

echo === POC-App Build Script ===
echo Build type: %BUILD_TYPE%

REM Clean build directory if requested
if %CLEAN_BUILD%==1 (
    echo Cleaning build directory...
    if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
)

REM Create build directory
if not exist %BUILD_DIR% (
    echo Creating build directory...
    mkdir %BUILD_DIR%
)

REM Configure with CMake
echo Configuring project...
cmake -B %BUILD_DIR% -S . -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %GENERATOR%
if errorlevel 1 (
    echo Configuration failed!
    exit /b 1
)

REM Build
echo Building project...
cmake --build %BUILD_DIR% --config %BUILD_TYPE%
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
echo Executable location: %BUILD_DIR%\bin\%BUILD_TYPE%\editor.exe

REM Run if requested
if %RUN_AFTER_BUILD%==1 (
    echo Running editor...
    %BUILD_DIR%\bin\%BUILD_TYPE%\editor.exe
)

endlocal
