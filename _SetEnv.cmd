@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
set sdkenv="C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd"
rem call %sdkenv% /debug
call %sdkenv% /release
set xmsbuild=msbuild /p:TargetFrameworkMoniker=".NETFramework,Version=v3.5" /p:Configuration="Release" /p:Platform="Win32"
rem set xmsbuild=msbuild /p:Configuration="Release" /p:Platform="Win32"
echo __________________________________________
echo %%xmsbuild%% /t:Clean "T-Clock 2010.sln"
echo.
echo %%xmsbuild%% "T-Clock 2010.sln"
echo.
echo __________________________________________
cmd /K
