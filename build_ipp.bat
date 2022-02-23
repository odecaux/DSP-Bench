@echo off

if not defined DevEnvDir (
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL
call "C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0\env\vars.bat" intel64 >NUL
)

set ipp_root=C:\Program Files (x86)\Intel\oneAPI\ipp\2021.5.0
set ipp_include="%ipp_root%\include"
set ipp_lib="%ipp_root%\lib\intel64"

set opts=/EHsc -FC -GR- -EHa- -nologo -Zi
set code="%cd%"

pushd build
cl %opts% %code%\fft.cpp /Feipp_test.dll  /std:c++latest ippcoremt.lib ippsmt.lib ippvmmt.lib /DEBUG /D_USRDLL /D_WINDLL /MTd /link /DLL



@if ERRORLEVEL == 0 (
   goto good
)

@if ERRORLEVEL != 0 (
   goto bad
)

:good
   echo "compilation successful"
   ipp_test.exe
   goto end

:bad
   goto end

:end

popd
