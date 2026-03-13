@echo off

set BUILD_DIR=build
set BUILD_TYPE=%1
set PARALLEL=%2

if "%BUILD_TYPE%"=="" (
    set BUILD_TYPE=Debug
)

if "%PARALLEL%"=="" (
    set PARALLEL=%NUMBER_OF_PROCESSORS%
)

if "%PARALLEL%"=="" (
    set PARALLEL=4
)

echo Building Nebula (%BUILD_TYPE%) with %PARALLEL% parallel jobs...

cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel %PARALLEL%

if errorlevel 1 (
    echo Build failed
    exit /b 1
)

if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\%BUILD_TYPE%\Nebula.exe" (
    copy /Y "%BUILD_DIR%\bin\%BUILD_TYPE%\%BUILD_TYPE%\Nebula.exe" "Nebula.exe" >nul
) else if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\Nebula.exe" (
    copy /Y "%BUILD_DIR%\bin\%BUILD_TYPE%\Nebula.exe" "Nebula.exe" >nul
)

echo Build completed.
