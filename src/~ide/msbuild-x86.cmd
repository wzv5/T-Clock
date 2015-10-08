@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
set sdkenv="C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x86
rem call %sdkenv% /debug
call %sdkenv% /release
set xmsbuild=msbuild /p:TargetFrameworkMoniker=".NETFramework,Version=v3.5" /p:Platform=x86
rem set xmsbuild=msbuild /p:Platform=x86
echo __________________________________________
echo %%xmsbuild%% "msvc-vs.sln" /t:Clean /p:Configuration=Release
echo %%xmsbuild%% "msvc-vs.sln" /t:Clean /p:Configuration=Debug
echo.
echo %%xmsbuild%% /m "msvc-vs.sln" /p:Configuration=Release
echo.
echo %%xmsbuild%% /m "msvc-vs.sln" /p:Configuration=Debug
echo.
echo __________________________________________
cmd /K
