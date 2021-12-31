rem d1vars sets up compiler variables
call d1vars.bat

set CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG

cd misc
make
cd..
cd includes
make
cd ..
cd fix
make
cd ..
cd cfile
make
cd ..
cd 2d
make
cd ..
cd bios
make
cd ..
cd iff
make
cd ..
cd div
make
cd ..
cd mem
make
cd ..
cd vecmat
make
cd ..
cd 3d
make
cd ..
cd texmap
make
cd ..
cd ui
make
cd ..
cd main
rem cd editor
rem make
rem cd ..
make
cd ..
Echo Make complete.

