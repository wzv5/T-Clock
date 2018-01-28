@echo off
set PATH=%PATH%;C:\Program Files\7-Zip
set "xclude=-xr!*.ini -xr!*.log -xr!*.rpt -x!*.zip -xr!*.pdb -xr!*.exp -xr!*.lib -xr!*.def -xr!*.a -xr!*.manifest -xr!_*"
pushd ..\Release
del *.zip *.7z 2>nul
7z a %xclude% T-Clock.zip .
7z a %xclude% T-Clock.7z .
popd
pause