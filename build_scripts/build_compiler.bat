@echo off

SETLOCAL

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL
call "C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0\env\vars.bat" intel64 >NUL

set my_inc="C:\llvm\debug_msvc\include"
set my_lib="C:\llvm\debug_msvc\lib"

set debug_opts= -GR- -EHa- /Zi /nologo /MTd /DEBUG /D DEBUG   /std:c++latest /w44624 /w44530 /w44244 

set code=%cd%

pushd build

cl %debug_opts% /Fp"compiler.pch" /Yu"llvm_include.h" "%code%\compiler.cpp"   /D_USRDLL /c
cl %debug_opts% "%code%\dsp.cpp" /c
link compiler.obj dsp.obj compiler_pch.obj   ippcoremt.lib ippsmt.lib ippvmmt.lib Comdlg32.lib /OUT:compiler.dll /LIBPATH:%my_lib% /DLL

popd

ENDLOCAL