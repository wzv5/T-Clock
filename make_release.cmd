@echo off
set PATH=%PATH%;C:\Program Files\7-Zip
pushd Release
del *.zip 2>nul
del *.7z 2>nul
del /S *.manifest 2>nul
set xclude=-x!*.zip -xr!*.exp -xr!*.lib
7z a %xclude% T-Clock.zip .
7z a %xclude% T-Clock.7z .
popd
pause