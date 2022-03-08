@echo off

if not defined DevEnvDir (
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL
call "C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0\env\vars.bat" intel64 >NUL
)


set llvm_inc="C:\llvm\release_msvc\include"
set llvm_lib="C:\llvm\release_msvc\lib"

set ipp_root=C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0
set ipp_include="%ipp_root%\include"
set ipp_lib="%ipp_root%\lib\intel64"

set release_opts=-FC -GR- -EHa- /nologo /MT /O2 /D "NDEBUG" /D "RELEASE"  /std:c++latest /w44624 /w44530 /w44244 
set code="%cd%"

set libs=Kernel32.lib  user32.lib Ole32.lib opengl32.lib Gdi32.lib ippcoremt.lib ippsmt.lib ippvmmt.lib Comdlg32.lib 

pushd build
cl %release_opts% /I %llvm_inc%  %code%\wasapi_audio.cpp %code%\app.cpp %code%\audio.cpp %code%\main.cpp %code%\descriptor.cpp %code%\fft.cpp %code%\compiler.cpp -FeDSP_bench_release.exe   %libs%  /INCREMENTAL:NO /link /LIBPATH:%llvm_lib%  
popd
