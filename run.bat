@echo off
pushd build 
echo %cd%
clang_test.exe mix2.wav gain_test.cpp
popd