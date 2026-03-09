@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build
set BUILD_TYPE=%1

if "%BUILD_TYPE%"=="" (
    set BUILD_TYPE=Release
)

set LINT_BUILD_DIR=build-lint-%BUILD_TYPE%

echo Running clang-tidy...

where clang-tidy >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo clang-tidy not found
    exit /b 1
)

if not exist "%BUILD_DIR%\compile_commands.json" (
    echo compile_commands.json not found in %BUILD_DIR%.
    echo Generating a dedicated lint database in %LINT_BUILD_DIR%...

    where ninja >nul 2>&1
    if !ERRORLEVEL! neq 0 (
        echo ninja not found
        exit /b 1
    )

    cmake -B %LINT_BUILD_DIR% ^
          -S . ^
          -G Ninja ^
          -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
          -DCPM_SOURCE_CACHE=%CD%\.cache\CPM

    if !ERRORLEVEL! neq 0 (
        echo CMake configuration for lint failed
        exit /b 1
    )

    set BUILD_DIR=%LINT_BUILD_DIR%
)

set FAILED=0

for /r src %%f in (*.cpp) do (
    echo Checking %%f
    clang-tidy "%%f" -p "%BUILD_DIR%" --quiet --extra-arg=-std=c++2b
    if !ERRORLEVEL! neq 0 (
        set FAILED=1
    )
)

if %FAILED% neq 0 (
    echo Lint errors detected
    exit /b 1
)

echo Lint passed.
exit /b 0
