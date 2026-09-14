@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build
set "HEADER_FILTER=%CD:\=[\\/]%[\\/]src[\\/].*"
echo Running clang-tidy...

set FAILED=0

for /r src %%f in (*.cpp) do (
    echo Checking %%f
    clang-tidy "%%f" -p "%BUILD_DIR%" --quiet --header-filter="%HEADER_FILTER%"
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
