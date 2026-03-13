@echo off
setlocal

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

echo Running tests (%BUILD_TYPE%) with %PARALLEL% parallel jobs...

ctest --test-dir %BUILD_DIR% ^
      -C %BUILD_TYPE% ^
      --parallel %PARALLEL% ^
      --output-on-failure

if errorlevel 1 (
    echo Some tests failed
    exit /b 1
)

echo All tests passed.
