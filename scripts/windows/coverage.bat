@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%\..\..") do set "REPOSITORY_ROOT=%%~fI"

set "BUILD_DIR=build-coverage"
set "BUILD_TYPE=Debug"
set "TEST_REGEX=%~1"
set "SOURCE_FILTER=%~2"
set "PARALLEL=%~3"
set "REPORT_PATH=%BUILD_DIR%\coverage\renderer-coverage.html"

if /I "%~1"=="-h" goto show_help
if /I "%~1"=="--help" goto show_help

if "%TEST_REGEX%"=="" (
    set "TEST_REGEX=Renderer(Component|Capability)Test"
)

if "%SOURCE_FILTER%"=="" (
    set "SOURCE_FILTER=src/new_core/modules/capabilities/renderer_capability.*"
)

if "%PARALLEL%"=="" (
    set "PARALLEL=%NUMBER_OF_PROCESSORS%"
)

if "%PARALLEL%"=="" (
    set "PARALLEL=4"
)

echo %PARALLEL%| findstr /r "^[1-9][0-9]*$" >nul
if errorlevel 1 (
    echo Invalid parallel jobs: %PARALLEL% ^(must be a positive integer^)
    exit /b 1
)

where cmake >nul 2>&1
if errorlevel 1 (
    echo CMake was not found in PATH.
    exit /b 1
)

where gcov >nul 2>&1
if errorlevel 1 (
    echo gcov was not found in PATH. Install MinGW GCC and add its bin directory to PATH.
    exit /b 1
)

set "PYTHON_COMMAND="
where py >nul 2>&1
if not errorlevel 1 set "PYTHON_COMMAND=py"

if not defined PYTHON_COMMAND (
    where python >nul 2>&1
    if not errorlevel 1 set "PYTHON_COMMAND=python"
)

if not defined PYTHON_COMMAND (
    echo Python was not found in PATH.
    exit /b 1
)

%PYTHON_COMMAND% -m gcovr --version >nul 2>&1
if errorlevel 1 (
    echo gcovr is not installed.
    echo Install it with: %PYTHON_COMMAND% -m pip install --user gcovr
    exit /b 1
)

pushd "%REPOSITORY_ROOT%"

echo [1/5] Configuring the instrumented build...
cmake -S . -B "%BUILD_DIR%" -G Ninja ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      "-DCMAKE_CXX_FLAGS=--coverage -O0 -g" ^
      "-DCMAKE_EXE_LINKER_FLAGS=--coverage" ^
      "-DCPM_SOURCE_CACHE=%CD%\.cache\CPM"
if errorlevel 1 goto configure_failed

echo [2/5] Building unit_tests with coverage enabled...
for /r "%BUILD_DIR%" %%F in (*.gcda) do del /q "%%F"
cmake --build "%BUILD_DIR%" --target unit_tests --parallel %PARALLEL%
if errorlevel 1 goto build_failed

echo [3/5] Removing counters from previous runs...
for /r "%BUILD_DIR%" %%F in (*.gcda) do del /q "%%F"

echo [4/5] Running selected tests...
ctest --test-dir "%BUILD_DIR%" ^
      -C %BUILD_TYPE% ^
      --parallel %PARALLEL% ^
      --output-on-failure ^
      -R "%TEST_REGEX%"
if errorlevel 1 goto tests_failed

echo [5/5] Generating the HTML report...
if not exist "%BUILD_DIR%\coverage" mkdir "%BUILD_DIR%\coverage"
%PYTHON_COMMAND% -m gcovr ^
    --root . ^
    --filter "%SOURCE_FILTER%" ^
    --gcov-executable gcov ^
    --exclude-throw-branches ^
    --exclude-unreachable-branches ^
    --print-summary ^
    --html-details "%REPORT_PATH%" ^
    "%BUILD_DIR%"
if errorlevel 1 goto report_failed

echo.
echo Coverage report generated: %CD%\%REPORT_PATH%
start "" "%REPORT_PATH%"
popd
exit /b 0

:show_help
echo.
echo Renderer coverage report for Nebula
echo.
echo Usage:
echo   .\scripts\windows\coverage.bat [TEST_REGEX] [SOURCE_FILTER] [PARALLEL]
echo.
echo Defaults:
echo   TEST_REGEX    Renderer(Component^|Capability)Test
echo   SOURCE_FILTER src/new_core/modules/capabilities/renderer_capability.*
echo   PARALLEL      NUMBER_OF_PROCESSORS, or 4
echo.
echo Examples:
echo   .\scripts\windows\coverage.bat
echo   .\scripts\windows\coverage.bat "RendererCapabilityTest" "src/new_core/.*" 4
exit /b 0

:configure_failed
echo Coverage configuration failed.
goto failed

:build_failed
echo Coverage build failed.
goto failed

:tests_failed
echo Coverage tests failed.
goto failed

:report_failed
echo Coverage report generation failed.
goto failed

:failed
popd
exit /b 1
