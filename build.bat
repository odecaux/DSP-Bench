@echo off

if not defined DevEnvDir (
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL
call "C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0\env\vars.bat" intel64 >NUL
)


set ipp_root=C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0
set ipp_include="%ipp_root%\include"
set ipp_lib="%ipp_root%\lib\intel64"

set opts=-FC -GR- -EHa- -nologo -Zi  /w44624 /w44530 /w44244 /wd4668 /std:c++latest 
set code="%cd%"

set libs=Kernel32.lib  user32.lib Ole32.lib opengl32.lib Gdi32.lib ippcoremt.lib ippsmt.lib ippvmmt.lib

pushd build
cl %opts% %code%\wasapi_audio.cpp %code%\app.cpp %code%\audio.cpp %code%\main.cpp %code%\descriptor.cpp %code%\fft.cpp -Feclang_test.exe /DEBUG %libs%  /INCREMENTAL:NO
popd
