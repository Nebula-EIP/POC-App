@echo off
setlocal enabledelayedexpansion

echo Running clang-format check...

where clang-format >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo clang-format not found
    exit /b 1
)

set FILES=

for /r src %%f in (*.cpp *.hpp *.h *.tcc) do (
    set FILES=!FILES! "%%f"
)

if "!FILES!"=="" (
    echo No source files found
    exit /b 0
)

clang-format --dry-run --Werror !FILES!
if %ERRORLEVEL% neq 0 (
    echo Format errors detected
    exit /b 1
)

echo Format check passed.
exit /b 0
