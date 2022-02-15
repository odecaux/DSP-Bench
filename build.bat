@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL

set opts=-FC -GR- -EHa- -nologo -Zi
set code="%cd%"
pushd build
cl %opts% %code%\wasapi_audio.cpp %code%\audio.cpp %code%\main.cpp %code%\descriptor.cpp -Feclang_test.exe /std:c++latest /w44624 /w44530 /w44244 /DEBUG dxguid.lib user32.lib Winmm.lib Ole32.lib uuid.lib  D2d1.lib Dinput8.lib Dwrite.lib Kernel32.lib /INCREMENTAL:NO
popd
