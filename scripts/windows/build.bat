@echo off

set BUILD_DIR=build
set BUILD_TYPE=%1

if "%BUILD_TYPE%"=="" (
    set BUILD_TYPE=Debug
)

echo Building Nebula (%BUILD_TYPE%)...

cmake -B %BUILD_DIR% ^
      -S . ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCPM_SOURCE_CACHE=%CD%\.cache\CPM

if errorlevel 1 (
    echo CMake configuration failed
    exit /b 1
)

cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo Build failed
    exit /b 1
)

echo.
echo Searching for Nebula.exe...
dir /s /b "%BUILD_DIR%\bin\" | find "Nebula.exe"

if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\%BUILD_TYPE%\Nebula.exe" (
    echo Found at: %BUILD_DIR%\bin\%BUILD_TYPE%\%BUILD_TYPE%\Nebula.exe
    copy /Y "%BUILD_DIR%\bin\%BUILD_TYPE%\%BUILD_TYPE%\Nebula.exe" "Nebula.exe" >nul
) else if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\Nebula.exe" (
    echo Found at: %BUILD_DIR%\bin\%BUILD_TYPE%\Nebula.exe
    copy /Y "%BUILD_DIR%\bin\%BUILD_TYPE%\Nebula.exe" "Nebula.exe" >nul
) else (
    echo WARNING: Nebula.exe not found at expected paths
)

echo Build completed.
exit /b 0