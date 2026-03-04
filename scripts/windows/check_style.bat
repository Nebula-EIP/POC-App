@echo off
REM Standalone code style checker - runs clang-format and clang-tidy on all src files

setlocal enabledelayedexpansion

REM Color codes simulation (limited in cmd)
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "RESET=[0m"

echo %BLUE%Code Style Checker%RESET%
echo %BLUE%(clang-format + clang-tidy)%RESET%
echo.

REM Check if clang-format is installed
where clang-format >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo %RED%clang-format is not installed or not in PATH%RESET%
    exit /b 1
)

REM Check if clang-tidy is installed
where clang-tidy >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo %RED%clang-tidy is not installed or not in PATH%RESET%
    exit /b 1
)

REM Check if build directory exists with compile_commands.json
if not exist "build\compile_commands.json" (
    echo %YELLOW%compile_commands.json not found. Running CMake configure first...%RESET%
    if not exist build mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cd ..
)

REM ============ CLANG-FORMAT ============
echo.
echo %BLUE%Running clang-format on all source files...%RESET%

set FILES_TO_FORMAT=
for /r src %%f in (*.cpp *.hpp *.h *.tcc) do (
    set "file=%%f"
    REM Exclude test and cache directories
    echo !file! | findstr /i "\\test\\" >nul || (
        echo !file! | findstr /i "\\.cache\\" >nul || (
            echo !file! | findstr /i "\\utils\\" >nul || (
                set FILES_TO_FORMAT=!FILES_TO_FORMAT! "%%f"
            )
        )
    )
)

if "!FILES_TO_FORMAT!"=="" (
    echo %YELLOW%No files to format.%RESET%
) else (
    echo %YELLOW%Formatting files...%RESET%
    clang-format -i !FILES_TO_FORMAT!
    echo %GREEN%✓ Formatting complete!%RESET%
)

REM ============ CLANG-TIDY ============
echo.
echo %BLUE%Running clang-tidy on all source files...%RESET%

set TIDY_FAILED=0
for /r src %%f in (*.cpp *.h *.tcc) do (
    set "file=%%f"
    REM Exclude test, cache, and utils directories
    echo !file! | findstr /i "\\test\\" >nul || (
        echo !file! | findstr /i "\\.cache\\" >nul || (
            echo !file! | findstr /i "\\utils\\" >nul || (
                echo %YELLOW%Checking: %%f%RESET%
                clang-tidy -p build "%%f" 2>&1 | findstr /v "no template named 'expected'" | findstr /v "no member named 'unexpected'" >temp_tidy.txt
                findstr /r "error: warning:" temp_tidy.txt >nul
                if !ERRORLEVEL! equ 0 (
                    type temp_tidy.txt
                    echo %RED%✗ Errors found in: %%f%RESET%
                    set TIDY_FAILED=1
                )
                del temp_tidy.txt
            )
        )
    )
)

if !TIDY_FAILED! equ 1 (
    echo %YELLOW%Fix the errors above, then run this script again.%RESET%
    exit /b 1
)

echo.
echo %GREEN%✓ All files passed style and tidy checks!%RESET%
exit /b 0
