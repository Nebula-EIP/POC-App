@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build
set DEMO_NAME=codegen_demo
set PARALLEL=%NUMBER_OF_PROCESSORS%

if "%PARALLEL%"=="" set PARALLEL=4
if "%~1" NEQ "" set PARALLEL=%~1

echo %PARALLEL%| findstr /r "^[1-9][0-9]*$" >nul
if errorlevel 1 (
    echo Invalid parallel jobs: %PARALLEL% ^(must be a positive integer^)
    exit /b 1
)

echo Building %DEMO_NAME% with %PARALLEL% parallel jobs...
cmake --build %BUILD_DIR% --target %DEMO_NAME% --parallel %PARALLEL%
if errorlevel 1 (
    echo Build failed
    exit /b 1
)

set DEMO_PATH=
for /r "%BUILD_DIR%\bin" %%F in (%DEMO_NAME%.exe) do (
    if exist "%%~fF" (
        set DEMO_PATH=%%~fF
        goto run_demo
    )
)

echo Could not find built demo executable for %DEMO_NAME%
exit /b 1

:run_demo
echo Running !DEMO_PATH!
call "!DEMO_PATH!"
exit /b %ERRORLEVEL%