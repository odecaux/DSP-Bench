@echo off
pushd build 
echo %cd%
clang_test.exe mix2.wav IR_test.cpp
popd