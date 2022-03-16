@echo off
SETLOCAL 
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL

set my_inc="C:\llvm\debug_msvc\include"
set my_lib="C:\llvm\debug_msvc\lib"

set debug_opts= -GR- -EHa- /Zi /nologo /MTd /DEBUG /D DEBUG   /std:c++latest /w44624 /w44530 /w44244 

set code=%cd%

pushd build

cl %debug_opts% /Yc"..\llvm_include.h" "%code%\build_scripts\compiler_pch.cpp" /Fp"compiler.pch" /I %my_inc% /c

popd
ENDLOCAL