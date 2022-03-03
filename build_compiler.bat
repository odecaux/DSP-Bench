@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL

set my_inc="C:\llvm\release_msvc\include"
set my_lib="C:\llvm\release_msvc\lib"

set release_opts=-FC -GR- -EHa- /nologo /MT /O2 /D "NDEBUG"   /std:c++latest /w44624 /w44530 /w44244 
set debug_opts=-FC -GR- -EHa- /Zi /nologo /MTd /DEBUG   /std:c++latest /w44624 /w44530 /w44244 

set code=%cd%

pushd build

cl %release_opts% "%code%\compiler.cpp" /Fecompiler.dll  /I %my_inc%  /D_USRDLL /D_WINDLL /link /LIBPATH:%my_lib% /DLL

popd
