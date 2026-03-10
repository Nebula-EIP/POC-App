@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build

echo Running clang-tidy...

set FAILED=0

for /r src %%f in (*.cpp) do (
    echo Checking %%f
    clang-tidy "%%f" -p "%BUILD_DIR%" --quiet --extra-arg=-std=c++23
    if !ERRORLEVEL! neq 0 (
        set FAILED=1
    )
)

if %FAILED% neq 0 (
    echo Lint errors detected
    exit /b 1
)

echo Lint passed.
