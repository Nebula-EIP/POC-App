@echo off
setlocal

set BUILD_DIR=build
set BUILD_TYPE=%1

if "%BUILD_TYPE%"=="" (
    set BUILD_TYPE=Release
)

echo Running tests (%BUILD_TYPE%)...

if not exist %BUILD_DIR% (
    echo Build directory not found
    exit /b 1
)

ctest --test-dir %BUILD_DIR% ^
      -C %BUILD_TYPE% ^
      --output-on-failure ^
      --verbose

if errorlevel 1 (
    echo Some tests failed
    exit /b 1
)

echo All tests passed.
exit /b 0
