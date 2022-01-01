#
# Master makefile for Miner source
#
# Makes all the executable by visiting subdirs and making there
#

# The only thing to make is the subdirs
SUBSYSTEMS = misc includes fix cfile 2d bios iff div mem vecmat 3d texmap ui main
#sosdigi sosmidi

# What to make in the subdirs if nothing specified
SUBTARGETS = clean optimize no_mono no_debug linstall
#SUBTARGETS = update linstall

# When making clean here, delete libs
CLEAN_TARGS = 
#lib\*.lib lib\*.h lib\*.inc

#all Miner makefiles should include this
#%include $(INIT)\makefile.def

all: FORCE $(SUBSYSTEMS)

misc: FORCE
	cd misc && $(MAKE)

includes: FORCE
	cd includes && $(MAKE)

fix: FORCE
	cd fix && $(MAKE)

cfile: FORCE
	cd cfile && $(MAKE)

2d: FORCE
	cd 2d && $(MAKE)

bios: FORCE
	cd bios && $(MAKE)

iff: FORCE
	cd iff && $(MAKE)

div: FORCE
	cd div && $(MAKE)

mem: FORCE
	cd mem && $(MAKE)

vecmat: FORCE
	cd vecmat && $(MAKE)

3d: FORCE
	cd 3d && $(MAKE)

texmap: FORCE
	cd texmap && $(MAKE)

ui: FORCE
	cd ui && $(MAKE)

main: FORCE
	cd main && $(MAKE)

clean:
	erase /s /q *.obj *.lib *.err main\descentr.exe

FORCE:
