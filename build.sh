#!/bin/bash

code="$WD"
opts=-g
cd build > /dev/null
g++ $opts $code/main.cpp -o clang_test.exe
cd $code > /dev/null
