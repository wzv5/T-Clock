goto=kkk
#!/bin/bash
# start of bash
cbp2make --wrap-objects --keep-outdir --local -in gcc.workspace -out ___tmp -unix -windows
for file in `ls gcc-*.cbp.mak.unix | grep -oP "(?<=gcc-).*?(?=\.cbp\.)"`;do
	mv gcc-$file.cbp.mak.unix linux-$file.make
	mv gcc-$file.cbp.mak.windows gcc-$file.make
done
rm ___tmp.unix
rm ___tmp.windows
exit $?


:=kkk
@echo off
rem start of batch
pushd %~dp0
cbp2make.exe --wrap-objects --keep-outdir --local -in gcc.workspace -out ___tmp -unix -windows
for %%f in (*.cbp.mak.unix) do (
	call :process linux %%f %%~nf
	call :process gcc %%~nf.windows %%~nf
)
del ___tmp.unix
del ___tmp.windows
popd & echo anykey to exit & cmd /C "pause>nul"
exit /B %errorlevel%
:process
call :process2 %1 %2 %~n3
exit /B 0
:process2
set prefix=%1-
set makefile=%2
set name=%~n3
set name=%name:~4%
echo %prefix%%name%.make
move %makefile% %prefix%%name%.make
exit /B 0
