@echo off
pushd %~dp0
( set/P=""<nul ) > T-Clock.ini
echo T-Clock.ini created, portable mode active
popd & echo anykey to exit & cmd /C "pause>nul"
