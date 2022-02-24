@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL

set opts=/EHsc -FC -GR- -EHa- -nologo -Zi
set code="%cd%"

pushd build
cl %opts% %code%\font_test.cpp /Fefont_test.exe  /std:c++latest 
font_test.exe
popd
