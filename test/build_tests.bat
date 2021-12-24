@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL

set my_inc="C:\llvm\debug_msvc\include"
set my_lib="C:\llvm\debug_msvc\lib"

set opts=-FC -GR- -EHa- -nologo -Zi
set code="%cd%"
pushd build
cl %opts% %code%\test\tests.cpp %code%\descriptor.cpp %code%\compiler.cpp -Fetests.exe /I %my_inc% /std:c++latest /w44624 /w44530 /w44244 /DEBUG /MTd /link /LIBPATH:%my_lib% 


@if ERRORLEVEL == 0 (
   goto good
)

@if ERRORLEVEL != 0 (
   goto bad
)

:good
   echo "compilation successful"
   tests.exe
   goto end

:bad
   goto end

:end

popd
