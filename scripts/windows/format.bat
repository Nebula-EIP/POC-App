@echo off
setlocal enabledelayedexpansion

set MODE=%1

if "%MODE%"=="" (
    set MODE=fix
)

set FILES=

for /r src %%f in (*.cpp *.hpp *.h *.tcc) do (
    set FILES=!FILES! "%%f"
)

if "!FILES!"=="" (
    echo No source files found
    exit /b 0
)

if /I "%MODE%"=="check" (
    echo Running clang-format check...
    clang-format --dry-run --Werror !FILES!
    if %ERRORLEVEL% neq 0 (
        echo Format errors detected
        exit /b 1
    )
    echo Format check passed.
) else (
    echo Running clang-format...
    clang-format -i !FILES!
    if %ERRORLEVEL% neq 0 (
        echo Formatting failed
        exit /b 1
    )
    echo Formatting applied.
)
