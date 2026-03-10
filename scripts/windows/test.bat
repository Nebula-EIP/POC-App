@echo off
setlocal

set BUILD_DIR=build
set BUILD_TYPE=%1

if "%BUILD_TYPE%"=="" (
    set BUILD_TYPE=Debug
)

echo Running tests (%BUILD_TYPE%)...

ctest --test-dir %BUILD_DIR% ^
      -C %BUILD_TYPE% ^
      --output-on-failure

if errorlevel 1 (
    echo Some tests failed
    exit /b 1
)

echo All tests passed.
