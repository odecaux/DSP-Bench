@echo off

if not defined DevEnvDir (
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL
call "C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0\env\vars.bat" intel64 >NUL
)


set ipp_root=C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0
set ipp_include="%ipp_root%\include"
set ipp_lib="%ipp_root%\lib\intel64"

set debug_opts=-FC -GR- -EHa- /Zi /nologo /MTd /DEBUG  /D DEBUG /std:c++latest /w44624 /w44530 /w44244 
set code="%cd%"

set libs=Kernel32.lib  user32.lib Ole32.lib opengl32.lib Gdi32.lib ippcoremt.lib ippsmt.lib ippvmmt.lib Comdlg32.lib

pushd build
cl %debug_opts% %code%\wasapi_audio.cpp %code%\app.cpp %code%\audio.cpp %code%\main.cpp %code%\plugin.cpp %code%\fft.cpp -FeDSP_bench.exe   %libs%  /INCREMENTAL:NO
popd
