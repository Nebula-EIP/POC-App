@echo off

set BUILD_DIR=build
set BUILD_TYPE=%1

if "%BUILD_TYPE%"=="" (
    set BUILD_TYPE=Debug
)

echo Configuring Nebula (%BUILD_TYPE%)...

cmake -B %BUILD_DIR% ^
      -S . ^
      -G Ninja ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCPM_SOURCE_CACHE=%CD%\.cache\CPM ^
      -DCMAKE_BUILD_TYPE=%1 ^
      -DCMAKE_PREFIX_PATH=C:\raylib

if errorlevel 1 (
    echo CMake configuration failed
    exit /b 1
)

echo Configuration completed.
