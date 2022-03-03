@echo off
pushd build 
echo %cd%
DSP_bench.exe sine_test.cpp
popd