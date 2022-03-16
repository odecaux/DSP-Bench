@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL
call "C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0\env\vars.bat" intel64 >NUL

set debug_opts=-GR- -EHa- /Zi /nologo /MTd /DEBUG  /D DEBUG /D ATOMIC_FORCE_SLEEP /std:c++latest /w44624 /w44530 /w44244 
set code="%cd%"

set libs=Kernel32.lib  user32.lib Ole32.lib opengl32.lib Gdi32.lib ippcoremt.lib ippsmt.lib ippvmmt.lib Comdlg32.lib

pushd build
cl %debug_opts% %code%\wasapi_audio.cpp %code%\app.cpp %code%\audio.cpp %code%\main.cpp %code%\plugin.cpp %code%\fft.cpp -FeDSP_bench.exe   %libs%  /INCREMENTAL:NO
popd
