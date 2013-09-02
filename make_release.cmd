@echo off
set PATH=%PATH%;C:\Program Files\7-Zip
del Release\Win32\*.exp 2>nul
del Release\Win32\*.lib 2>nul
del Release\x64\*.exp 2>nul
del Release\x64\*.lib 2>nul
7z a T-Clock.zip .\Release\*
7z a T-Clock.7z .\Release\*
pause
