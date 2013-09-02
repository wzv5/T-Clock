@echo off
set PATH=%PATH%;C:\Program Files\7-Zip
pushd Release
del *.zip 2>nul
del *.7z 2>nul
del /S *.exp 2>nul
del /S *.exp 2>nul
del /S *.lib 2>nul
del /S *.manifest 2>nul
7z a T-Clock.zip .
7z a -x!*.zip T-Clock.7z .
popd
pause
