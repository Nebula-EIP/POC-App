@echo off

set BUILD_DIR=build
set BUILD_TYPE=%1
set PARALLEL=%2

if "%BUILD_TYPE%"=="" (
    set BUILD_TYPE=Debug
)

if "%PARALLEL%"=="" (
    set PARALLEL=%NUMBER_OF_PROCESSORS%
)

if "%PARALLEL%"=="" (
    set PARALLEL=4
)

echo Building Nebula (%BUILD_TYPE%) with %PARALLEL% parallel jobs...

cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel %PARALLEL%

if errorlevel 1 (
    echo Build failed
    exit /b 1
)

set MODULE_DIR=%BUILD_DIR%\manual-modules
if not exist "%MODULE_DIR%" mkdir "%MODULE_DIR%"
if not exist "%BUILD_DIR%\bin\%BUILD_TYPE%\fixture_valid_module.dll" (
    echo Valid module fixture was not built
    exit /b 1
)
copy /Y "%BUILD_DIR%\bin\%BUILD_TYPE%\fixture_valid_module.dll" "%MODULE_DIR%\" >nul

if not exist "config.json" (
    >"config.json" echo {
    >>"config.json" echo   "module_folder": "%MODULE_DIR:\=/%"
    >>"config.json" echo }
    echo Created default config.json
)

if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\%BUILD_TYPE%\Nebula.exe" (
    copy /Y "%BUILD_DIR%\bin\%BUILD_TYPE%\%BUILD_TYPE%\Nebula.exe" "Nebula.exe" >nul
) else if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\Nebula.exe" (
    copy /Y "%BUILD_DIR%\bin\%BUILD_TYPE%\Nebula.exe" "Nebula.exe" >nul
)

echo Build completed.
