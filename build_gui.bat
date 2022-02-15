@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL

set opts=-FC -GR- -EHa- -nologo -Zi  /w44624 /w44530 /w44244 /wd4668
set code="%cd%"
pushd build
cl %opts% %code%\app.cpp %code%/descriptor.cpp /Fegui.dll  user32.lib  Ole32.lib   Kernel32.lib Gdi32.lib opengl32.lib /std:c++latest  /DEBUG /D_USRDLL /D_WINDLL /link /DLL /INCREMENTAL:NO
popd
