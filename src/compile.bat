goto=kkk
#!/bin/bash
# start of bash
pushd "`dirname "${BASH_SOURCE[0]}"`/~ide"
export TRAVIS_COMMIT_MSG="${TRAVIS_COMMIT_MSG:-$(git log --format=%B --no-merges -n1)}"

AUVER_IF_NOT=${TRAVIS_TAG:-${APPVEYOR_REPO_TAG_NAME}}
if [ -z "$AUVER_IF_NOT" ];then export AUVER_SET="STATUS=0"; fi
if [[ $TRAVIS_COMMIT_MSG =~ \[(log(ging)?)\] ]];then
	LOGGING=1
fi
if [ "$LOGGING" = 1 ];then
	echo "#define LOGGING" > ../common/utl_logging.h
fi

./makex "$@"
ret=$?
popd
return $ret 2>/dev/null;exit $ret


:=kkk
@echo off
rem # start of batch
pushd %~dp0~ide
if not defined signtool set "signtool="C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\signtool.exe""
if not defined vcvarsall set "vcvarsall="C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat""
if not defined SetEnv set "SetEnv="C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd""

set "AUVER_IF_NOT=%TRAVIS_TAG%%APPVEYOR_REPO_TAG_NAME%"
if not defined AUVER_IF_NOT set "AUVER_SET=STATUS=0"

rem if not defined APPVEYOR_REPO_COMMIT_MESSAGE_EXTENDED (
	git log --format^=%%B --no-merges -n1 >.tmp_msg
	(findstr /C:"[logging]" .tmp_msg || findstr /C:"[log]" .tmp_msg) >nul
	if %errorlevel% equ 0 (
		set LOGGING=1
	)
	del .tmp_msg
rem )
if "%LOGGING%"=="1" (
	echo #define LOGGING > ../common/utl_logging.h
)
if defined APPVEYOR (
	if not defined MSBUILD_LOGGER set "MSBUILD_LOGGER=/logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll""
)
if "%1"=="clean" (
	shift
	set "target=/t:Clean"
)
rem %vcvarsall% x86
call %SetEnv% /release /x86
msbuild "msvc-vs.sln" /p:Platform=x86 /p:Configuration=Release /verbosity:minimal %MSBUILD_LOGGER% %target% "%1" "%2" "%3" "%4" "%5" "%6" "%7" "%8" "%9"
if %errorlevel% neq 0 exit /B %errorlevel%
rem %vcvarsall% amd64
call %SetEnv% /release /x64
msbuild "msvc-vs.sln" /p:Platform=x64 /p:Configuration=Release /verbosity:minimal %MSBUILD_LOGGER% %target% "%1" "%2" "%3" "%4" "%5" "%6" "%7" "%8" "%9"
if %errorlevel% neq 0 exit /B %errorlevel%
popd
exit /B %errorlevel%
