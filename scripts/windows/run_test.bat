@echo off
call "%~dp0build.bat" %* || exit /b
ctest --test-dir "%~dp0..\..\build" -C Release