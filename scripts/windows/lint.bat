@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build
set JOBS=%1

if "%JOBS%"=="" (
    set JOBS=%NUMBER_OF_PROCESSORS%
)

if "%JOBS%"=="" (
    set JOBS=4
)

echo %JOBS%| findstr /r "^[1-9][0-9]*$" >nul
if errorlevel 1 (
    echo Invalid jobs value: %JOBS%
    exit /b 1
)

set TMP_DIR=%TEMP%\nebula_lint_%RANDOM%%RANDOM%
mkdir "%TMP_DIR%" >nul 2>&1

echo Running clang-tidy with %JOBS% parallel jobs...

set FAILED=0

for /r src %%f in (*.cpp) do (
    call :wait_for_slot
    set "TASK_ID=!RANDOM!!RANDOM!!RANDOM!"
    set "RUN_MARK=%TMP_DIR%\!TASK_ID!.running"
    set "RESULT_FILE=%TMP_DIR%\!TASK_ID!.result"
    > "!RUN_MARK!" echo running
    echo Checking %%f
    start "" /b cmd /c "clang-tidy \"%%f\" -p \"%BUILD_DIR%\" --quiet && (echo OK>\"!RESULT_FILE!\") || (echo FAIL>\"!RESULT_FILE!\") & del \"!RUN_MARK!\""
)

call :wait_all

for %%r in ("%TMP_DIR%\*.result") do (
    findstr /I /C:"FAIL" "%%~fr" >nul && set FAILED=1
)

rmdir /s /q "%TMP_DIR%" >nul 2>&1

if %FAILED% neq 0 (
    echo Lint errors detected
    exit /b 1
)

echo Lint passed.
exit /b 0

:wait_for_slot
set RUNNING=0
for /f %%c in ('dir /b /a-d "%TMP_DIR%\*.running" 2^>nul ^| find /c /v ""') do set RUNNING=%%c
if %RUNNING% geq %JOBS% (
    timeout /t 1 /nobreak >nul
    goto wait_for_slot
)
exit /b 0

:wait_all
set RUNNING=0
for /f %%c in ('dir /b /a-d "%TMP_DIR%\*.running" 2^>nul ^| find /c /v ""') do set RUNNING=%%c
if %RUNNING% gtr 0 (
    timeout /t 1 /nobreak >nul
    goto wait_all
)
exit /b 0
