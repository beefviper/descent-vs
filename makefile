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

# What make flags to use
NMAKECMD = nmake
NMAKEFLAGS = /nologo

#all Miner makefiles should include this
#%include $(INIT)\makefile.def

all: FORCE $(SUBSYSTEMS)

misc: FORCE
	cd misc && $(NMAKECMD) $(NMAKEFLAGS)

includes: FORCE
	cd includes && $(NMAKECMD) $(NMAKEFLAGS)

fix: FORCE
	cd fix && $(NMAKECMD) $(NMAKEFLAGS)

cfile: FORCE
	cd cfile && $(NMAKECMD) $(NMAKEFLAGS)

2d: FORCE
	cd 2d && $(NMAKECMD) $(NMAKEFLAGS)

bios: FORCE
	cd bios && $(NMAKECMD) $(NMAKEFLAGS)

iff: FORCE
	cd iff && $(NMAKECMD) $(NMAKEFLAGS)

div: FORCE
	cd div && $(NMAKECMD) $(NMAKEFLAGS)

mem: FORCE
	cd mem && $(NMAKECMD) $(NMAKEFLAGS)

vecmat: FORCE
	cd vecmat && $(NMAKECMD) $(NMAKEFLAGS)

3d: FORCE
	cd 3d && $(NMAKECMD) $(NMAKEFLAGS)

texmap: FORCE
	cd texmap && $(NMAKECMD) $(NMAKEFLAGS)

ui: FORCE
	cd ui && $(NMAKECMD) $(NMAKEFLAGS)

main: FORCE
	cd main && $(NMAKECMD) $(NMAKEFLAGS)

clean:
	erase /s /q *.obj *.lib *.err main\descentr.exe

FORCE:
