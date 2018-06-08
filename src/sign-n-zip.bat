goto=kkk
#!/bin/bash
# start of bash
pushd "`dirname "${BASH_SOURCE[0]}"`"
xclude="-xr!*.ini -xr!*.log -xr!*.rpt -x!*.zip -xr!*.pdb -xr!*.exp -xr!*.lib -xr!*.def -xr!*.a -xr!*.manifest"
tag=${TRAVIS_TAG:-${APPVEYOR_REPO_TAG_NAME}}
# require tag
#[ -z "$tag" ] && (return 0 2>/dev/null;exit 0)
echo "	$tag"
if [ "$signpwd" ];then
	# get cert and signtool for linux
	(pushd .. && echo "wget 'osslsigncode'"
	wget -q "$signurl" -O .cert.zip && 7z x -yp$signpwd .cert.zip>/dev/null && chmod +x osslsigncode-src/configure
	e=$?;[ $e != 0 ]&&(return $e 2>/dev/null;exit $e)
	echo "make 'osslsigncode'" && cd osslsigncode-src && ./configure -q && make V=0 && mv osslsigncode ../ && popd)
	e=$?;[ $e != 0 ]&&(return $e 2>/dev/null;exit $e)
	# sign
	pushd ../Release; echo "sign"
	for f in *.exe *.dll */*.exe */*.dll;do
		[ -f "$f" ] || continue
		echo -n "$f: "
		../osslsigncode sign -pkcs12 ../.cert.pfx -t http://timestamp.verisign.com/scripts/timstamp.dll "$f" "$f.tmp"
		e=$?;[ $e != 0 ]&&(return $e 2>/dev/null;exit $e)
		mv "$f.tmp" "$f"
	done
	popd
fi
# compress
cd ../Release; echo "compress"
rm -f *.zip *.7z
7z a $xclude T-Clock.zip .
ret=$?
[ "$tag" ] && 7z a $xclude T-Clock.7z .
if [ "$GDRIVE_REFRESH_TOKEN" ];then
	wget -qO gdrive "https://drive.google.com/uc?id=1L1iWOR_yCvgR7L_FrcIYqcPLjwXwlAxX&export=download" && chmod +x gdrive
	./gdrive update --refresh-token $GDRIVE_REFRESH_TOKEN "1m18Jb-eZya6to3NsXUlZeC2ITjXdM7IU" T-Clock.zip
fi
popd
return $ret 2>/dev/null;exit $ret


:=kkk
@echo off
rem # start of batch
pushd %~dp0
if not defined signtool set signtool=signtool
set "xclude=-xr!*.ini -xr!*.log -xr!*.rpt -x!*.zip -xr!*.pdb -xr!*.exp -xr!*.lib -xr!*.def -xr!*.a -xr!*.manifest -xr!_*"
if defined TRAVIS_TAG set tag=%TRAVIS_TAG%
if defined APPVEYOR_REPO_TAG_NAME set tag=%APPVEYOR_REPO_TAG_NAME%
rem # require tag
rem if not defined tag exit /B 0
echo.	%tag%
if defined signpwd (
	rem # get cert
	pushd .. && echo wget '.cert.zip'
	powershell wget "%signurl%" -OutFile .cert.zip >nul 2>nul && 7z x -y -p%signpwd% .cert.zip >nul
	if %errorlevel% neq 0 exit /B %errorlevel%
	if not exist .cert.pfx exit /B 666
	popd
	rem # sign
	pushd ..\Release & echo sign
	%signtool% sign /v /f ..\.cert.pfx /t http://timestamp.verisign.com/scripts/timstamp.dll *.exe misc\*.exe misc\*.dll
	if %errorlevel% neq 0 exit /B %errorlevel%
	rem %signtool% verify Clock.exe
	rem if %errorlevel% neq 0 exit /B %errorlevel%
	popd
)
rem # compress
cd ..\Release & echo compress
del *.zip *.7z 2>nul
7z a %xclude% T-Clock_vc2010.zip .
set ret=%errorlevel%
rem 7z a %xclude% T-Clock_vc2010.7z .
if defined CI (
	rem powershell Push-AppveyorArtifact T-Clock_vc2010.7z
	powershell Push-AppveyorArtifact T-Clock_vc2010.zip
	if %errorlevel% neq 0 exit /B %errorlevel%
)
popd
exit /B %ret%
