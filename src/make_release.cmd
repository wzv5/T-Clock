@echo off
set PATH=%PATH%;C:\Program Files\7-Zip
pushd ..\Release
del *.zip 2>nul
del *.7z 2>nul
del /S *.manifest 2>nul
set xclude=-xr!*.ini -x!*.zip -xr!*.pdb -xr!*.exp -xr!*.lib -xr!*.log -xr!*.rpt
7z a %xclude% T-Clock.zip .
7z a %xclude% T-Clock.7z .
copy *.txt ..\Static
copy *.rtf ..\Static
copy *.ttf ..\Static
copy *.cmd ..\Static
popd
pushd ..\Static
del *.zip 2>nul
del *.7z 2>nul
del /S *.manifest 2>nul
set xclude=-x!*.zip -xr!*.pdb -xr!*.exp -xr!*.lib
7z a %xclude% T-Clock_static.zip .
rem 7z a %xclude% T-Clock_static.7z .
popd
pause