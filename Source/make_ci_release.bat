goto=kkk
#!/bin/bash
# start of bash
xclude="-xr!*.ini -xr!*.log -xr!*.rpt -x!*.zip -xr!*.pdb -xr!*.exp -xr!*.lib -xr!*.def -xr!*.a -xr!*.manifest"
tag=${TRAVIS_TAG:-${APPVEYOR_REPO_TAG_NAME}}
function errchk(){ e=$?; if [ $e -ne 0 ];then exit $e; fi }
# require tag
[ -z "$tag" ] && exit 0
echo "	$tag"
# get cert and signtool for linux
(cd .. && echo "wget 'osslsigncode'"
wget -q "$signurl" -O .cert.zip && 7z x -yp$signpwd .cert.zip>/dev/null && chmod +x osslsigncode-src/configure
errchk
echo "make 'osslsigncode'" && cd osslsigncode-src && ./configure -q && make V=0 && mv osslsigncode ../)
errchk
# sign
cd ../Release && echo "sign"
for f in *.exe *.dll */*.exe */*.dll;do
	[ -f "$f" ] || continue
	echo -n "$f: "
	../osslsigncode sign -pkcs12 ../.cert.pfx -t http://timestamp.verisign.com/scripts/timstamp.dll "$f" "$f.tmp"
	errchk
	mv "$f.tmp" "$f"
done
# compress
rm -f *.zip *.7z
7z a $xclude T-Clock_static.zip .
#7z a $xclude T-Clock_static.7z .
exit $?


:=kkk
@echo off
rem start of batch
if not defined signtool set signtool=signtool
set "xclude=-xr!*.ini -xr!*.log -xr!*.rpt -x!*.zip -xr!*.pdb -xr!*.exp -xr!*.lib -xr!*.def -xr!*.a -xr!*.manifest"
if defined TRAVIS_TAG set tag=%TRAVIS_TAG%
if defined APPVEYOR_REPO_TAG_NAME set tag=%APPVEYOR_REPO_TAG_NAME%
rem require tag
if not defined tag exit /B 0
echo 	%tag%
rem get cert
pushd .. && echo "wget '.cert.zip'"
powershell wget "%signurl%" -OutFile .cert.zip >nul 2>nul && 7z x -y -p%signpwd% .cert.zip >nul
if %errorlevel% neq 0 exit /B %errorlevel%
if not exist .cert.pfx exit /B 666
popd
rem sign
cd ..\Release
%signtool% sign /v /f ..\.cert.pfx /t http://timestamp.verisign.com/scripts/timstamp.dll *.exe misc\*.exe misc\*.dll
if %errorlevel% neq 0 exit /B %errorlevel%
rem %signtool% verify Clock.exe
rem if %errorlevel% neq 0 exit /B %errorlevel%
rem compress
del *.zip *.7z 2>nul
7z a %xclude% T-Clock.zip .
7z a %xclude% T-Clock.7z .
appveyor PushArtifact T-Clock.zip
appveyor PushArtifact T-Clock.7z
exit /B %errorlevel%
