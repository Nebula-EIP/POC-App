@echo off

set BUILD_DIR=build
set BUILD_TYPE=%1

if "%BUILD_TYPE%"=="" (
    set BUILD_TYPE=Debug
)

echo Building Nebula (%BUILD_TYPE%)...

cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo Build failed
    exit /b 1
)

if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\%BUILD_TYPE%\Nebula.exe" (
    copy /Y "%BUILD_DIR%\bin\%BUILD_TYPE%\%BUILD_TYPE%\Nebula.exe" "Nebula.exe" >nul
) else if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\Nebula.exe" (
    copy /Y "%BUILD_DIR%\bin\%BUILD_TYPE%\Nebula.exe" "Nebula.exe" >nul
)

echo Build completed.
