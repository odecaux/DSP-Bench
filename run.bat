@echo off
pushd build 
DSP_bench.exe mix.wav sine_test.cpp
popd