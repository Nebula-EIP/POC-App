@echo off
call "%~dp0build.bat" %* || exit /b
"%~dp0..\..\build\bin\Release\Release\Nebula.exe"