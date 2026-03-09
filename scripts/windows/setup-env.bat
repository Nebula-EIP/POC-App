@echo off
setlocal

echo Setting up Windows build environment...

REM Enable MSVC environment
call "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat"

REM Verify tools
where cmake >nul 2>&1 || (
    echo CMake not found
    exit /b 1
)

where clang-format >nul 2>&1 || (
    echo clang-format not found
)

where clang-tidy >nul 2>&1 || (
    echo clang-tidy not found
)

echo Environment ready.

endlocal
