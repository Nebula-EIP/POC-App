@echo off
setlocal

set BUILD_DIR=build
set BUILD_TYPE=%1

if "%BUILD_TYPE%"=="" (
    set BUILD_TYPE=Release
)

echo Building Nebula (%BUILD_TYPE%)...

if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

cmake -B %BUILD_DIR% ^
      -S . ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCPM_SOURCE_CACHE=%CD%\.cache\CPM

if errorlevel 1 (
    echo CMake configuration failed
    exit /b 1
)

cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo Build failed
    exit /b 1
)

echo Build completed.
exit /b 0