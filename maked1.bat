rem d1vars sets up compiler variables
call d1vars.bat

set CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG

cd misc
nmake
cd..
cd includes
nmake
cd ..
cd fix
nmake
cd ..
cd cfile
nmake
cd ..
cd 2d
nmake
cd ..
cd bios
nmake
cd ..
cd iff
nmake
cd ..
cd div
nmake
cd ..
cd mem
nmake
cd ..
cd vecmat
nmake
cd ..
cd 3d
nmake
cd ..
cd texmap
nmake
cd ..
cd ui
nmake
cd ..
cd main
rem cd editor
rem nmake
rem cd ..
nmake
cd ..
Echo nmake complete.

