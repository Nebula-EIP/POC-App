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
set MODULE_FILE=%BUILD_DIR%\modules\%BUILD_TYPE%\neb_c_module.dll
if not exist "%MODULE_FILE%" set MODULE_FILE=%BUILD_DIR%\modules\neb_c_module.dll
if not exist "%MODULE_FILE%" (
    echo Nebula C module was not built
    exit /b 1
)
del /Q "%MODULE_DIR%\fixture_valid_module.dll" 2>nul
copy /Y "%MODULE_FILE%" "%MODULE_DIR%\" >nul

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
