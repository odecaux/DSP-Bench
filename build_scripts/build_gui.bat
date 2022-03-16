@echo off
SETLOCAL
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL

set debug_opts=-FC -GR- -EHa- /Zi /nologo /MTd /DEBUG /D DEBUG  /std:c++latest /w44624 /w44530 /w44244 

set code=%cd%

pushd build
del gui_*.pdb > NUL 2> NUL
cl %debug_opts% "%code%\gui.cpp" /Fegui_out.dll  /D_USRDLL /D_WINDLL /link /PDB:gui_%random%.pdb /DLL
del gui.dll
ren gui_out.dll gui.dll
popd
ENDLOCAL