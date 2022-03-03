@echo off
pushd build 
echo %cd%
DSP_bench.exe mix2.wav gain_test.cpp
popd