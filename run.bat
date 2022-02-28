@echo off
pushd build 
echo %cd%
DSP_bench.exe mix.wav gain_test.cpp
popd