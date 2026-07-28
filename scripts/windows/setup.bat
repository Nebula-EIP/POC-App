@echo off

echo Setting up Windows build environment...

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo Error: Visual Studio Installer's vswhere.exe was not found.
    exit /b 1
)

set "VS_INSTALL_PATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_INSTALL_PATH=%%i"
)

if not defined VS_INSTALL_PATH (
    echo Error: No Visual Studio installation with the C++ build tools was found.
    exit /b 1
)

call "%VS_INSTALL_PATH%\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
if errorlevel 1 (
    echo Error: Failed to initialize the Visual Studio build environment.
    exit /b 1
)

echo Environment ready.
exit /b 0
