@echo off
setlocal enabledelayedexpansion

set BUILD_TYPE=Debug
set FORMAT_MODE=Check
set RUN_TESTS=False
set PARALLEL=%NUMBER_OF_PROCESSORS%
set INTERACTIVE_MODE=false

if "%PARALLEL%"=="" set PARALLEL=4

if "%~1"=="" set INTERACTIVE_MODE=true

:parse_args
if "%~1"=="" goto args_done

if /I "%~1"=="-h" goto show_help
if /I "%~1"=="--help" goto show_help

if /I "%~1"=="-b" (
    if "%~2"=="" goto bad_args
    set BUILD_TYPE=%~2
    shift
    shift
    goto parse_args
)

if /I "%~1"=="--build-type" (
    if "%~2"=="" goto bad_args
    set BUILD_TYPE=%~2
    shift
    shift
    goto parse_args
)

if /I "%~1"=="-f" (
    if "%~2"=="" goto bad_args
    set FORMAT_MODE=%~2
    shift
    shift
    goto parse_args
)

if /I "%~1"=="--format-mode" (
    if "%~2"=="" goto bad_args
    set FORMAT_MODE=%~2
    shift
    shift
    goto parse_args
)

if /I "%~1"=="-t" (
    if "%~2"=="" goto bad_args
    set RUN_TESTS=%~2
    shift
    shift
    goto parse_args
)

if /I "%~1"=="--run-tests" (
    if "%~2"=="" goto bad_args
    set RUN_TESTS=%~2
    shift
    shift
    goto parse_args
)

if /I "%~1"=="-j" (
    if "%~2"=="" goto bad_args
    set PARALLEL=%~2
    shift
    shift
    goto parse_args
)

if /I "%~1"=="--parallel" (
    if "%~2"=="" goto bad_args
    set PARALLEL=%~2
    shift
    shift
    goto parse_args
)

echo Unknown option: %~1
goto show_help_error

:args_done
if /I "%BUILD_TYPE%"=="debug" set BUILD_TYPE=Debug
if /I "%BUILD_TYPE%"=="release" set BUILD_TYPE=Release

if /I "%FORMAT_MODE%"=="check" set FORMAT_MODE=Check
if /I "%FORMAT_MODE%"=="fix" set FORMAT_MODE=Fix

if /I "%RUN_TESTS%"=="true" set RUN_TESTS=True
if /I "%RUN_TESTS%"=="false" set RUN_TESTS=False

if /I not "%BUILD_TYPE%"=="Debug" if /I not "%BUILD_TYPE%"=="Release" (
    echo Invalid build type: %BUILD_TYPE% ^(must be Debug or Release^)
    exit /b 1
)

if /I not "%FORMAT_MODE%"=="Check" if /I not "%FORMAT_MODE%"=="Fix" (
    echo Invalid format mode: %FORMAT_MODE% ^(must be Check or Fix^)
    exit /b 1
)

if /I not "%RUN_TESTS%"=="True" if /I not "%RUN_TESTS%"=="False" (
    echo Invalid run tests value: %RUN_TESTS% ^(must be True or False^)
    exit /b 1
)

echo %PARALLEL%| findstr /r "^[1-9][0-9]*$" >nul
if errorlevel 1 (
    echo Invalid parallel jobs: %PARALLEL% ^(must be positive integer^)
    exit /b 1
)

if /I "%INTERACTIVE_MODE%"=="true" goto interactive_mode
goto execute_workflow

:show_help
echo.
echo Development Build ^& Test for Nebula
echo.
echo Usage:
echo   .\scripts\windows\dev.bat [OPTIONS]
echo.
echo OPTIONS:
echo   -h, --help                Show this help message
echo   -b, --build-type TYPE     Debug / Release (default: Debug)
echo   -f, --format-mode MODE    Check / Fix (default: Check)
echo   -t, --run-tests BOOL      True / False (default: False)
echo   -j, --parallel N          Number of parallel jobs (default: %PARALLEL%)
echo.
echo EXAMPLES:
echo   .\scripts\windows\dev.bat
echo   .\scripts\windows\dev.bat -b release -f fix -t true -j 8
echo.
echo NOTES:
echo   - No args: interactive mode
echo   - Any arg: non-interactive mode with defaults for missing options
exit /b 0

:show_help_error
call :show_help
exit /b 1

:bad_args
echo Missing value for option %~1
goto show_help_error

:interactive_mode
echo.
echo ========================================
echo   Development Build ^& Test Nebula
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
    set FORMAT_MODE=Check
) else if "%format_choice%"=="2" (
    set FORMAT_MODE=Fix
) else (
    echo Invalid choice
    exit /b 1
)

echo.
echo Select test mode:
echo 1) Don't run tests (default)
echo 2) Run tests
set /p test_choice="Enter choice (1 or 2): "

if "%test_choice%"=="" set test_choice=1
if "%test_choice%"=="1" (
    set RUN_TESTS=False
) else if "%test_choice%"=="2" (
    set RUN_TESTS=True
) else (
    echo Invalid choice
    exit /b 1
)

echo.
set /p parallel_choice="Enter number of parallel jobs (default: %PARALLEL%): "
if not "%parallel_choice%"=="" (
    set PARALLEL=%parallel_choice%
    echo %PARALLEL%| findstr /r "^[1-9][0-9]*$" >nul
    if errorlevel 1 (
        echo Invalid choice (must be positive integer)
        exit /b 1
    )
)

:execute_workflow

echo.
echo Building %BUILD_TYPE% with format mode: %FORMAT_MODE%, tests: %RUN_TESTS%, parallel: %PARALLEL%
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
call .\scripts\windows\build.bat %BUILD_TYPE% %PARALLEL%
if errorlevel 1 exit /b 1

if /I "%RUN_TESTS%"=="True" (
    echo [5/5] Test ^(%BUILD_TYPE%^)...
    call .\scripts\windows\test.bat %BUILD_TYPE% %PARALLEL%
    if errorlevel 1 exit /b 1
) else (
    echo [5/5] Skipping tests...
)

echo.
echo ========================================
echo   Development Completed
echo ========================================
echo.

exit /b 0
