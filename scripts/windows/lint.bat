@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build

echo Running clang-tidy...

where clang-tidy >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo clang-tidy not found
    exit /b 1
)

if not exist "%BUILD_DIR%\compile_commands.json" (
    echo compile_commands.json not found. Configure the project first.
    exit /b 1
)

set FAILED=0

for /r src %%f in (*.cpp) do (
    echo Checking %%f
    clang-tidy "%%f" -p %BUILD_DIR%
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
