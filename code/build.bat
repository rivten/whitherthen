@echo off

set UntreatedWarnings=/wd4100 /wd4244 /wd4201 /wd4127 /wd4505 /wd4456 /wd4996 /wd4003 /wd4706 /wd4200 /wd4189
set CommonCompilerDebugFlags=/MT /Od /Oi /fp:fast /fp:except- /Zo /Gm- /GR- /EHa /WX /W4 %UntreatedWarnings% /Z7 /nologo /DCOMPILE_SLOW=1 /DCOMPILE_INTERNAL=1 /DCOMPILE_WINDOWS=1
set CommonLinkerDebugFlags=/incremental:no /opt:ref /subsystem:windows /ignore:4099 /NODEFAULTLIB

pushd ..\build\

REM Game
cl %CommonCompilerDebugFlags% ..\code\whitherthen.cpp -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:whitherthen_%random%.pdb -EXPORT:GameUpdateAndRender

cl %CommonCompilerDebugFlags% ..\code\sokol_whitherthen.cpp /link %CommonLinkerDebugFlags% user32.lib gdi32.lib winmm.lib kernel32.lib opengl32.lib msvcrtd.lib vcruntimed.lib ucrtd.lib msvcurtd.lib Imm32.lib Shell32.lib
popd

rem --------------------------------------------------------------------------
echo Compilation completed...
