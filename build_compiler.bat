@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL

set inc="%cd%\../llvm\include"

set opts=-FC -GR- -EHa- -nologo -Zi
set code="%cd%"
pushd build
cl %opts% %code%\compiler.cpp /Fecompiler.dll  /I %inc% /std:c++latest /w44624 /w44530 /w44244 /DEBUG /D_USRDLL /D_WINDLL /link /LIBPATH:"C:\Users\Octave\Documents\code projects\llvm\lib" /DLL
popd
