@echo off
pushd %~dp0
echo.>Clock.ini
echo Clock.ini created, portable mode active
popd & echo anykey to exit & cmd /C "pause>nul"
