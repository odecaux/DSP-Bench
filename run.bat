@echo off
pushd build 
echo %cd%
DSP_bench.exe mix.wav sine_test.cpp
popd