#
# $Source: f:/miner/source/rcs/makefile $
# $Revision: 1.1 $
# $Author: beefviper $
# $Date: 2022/01/07 22:27:23 $
#
# Makefile for Descent executable 
#

# What subsystems to make 
SUBSYSTEMS = misc includes fix cfile 2d bios iff div mem vecmat 3d texmap ui main

# What to clean
CLEAN_TARGS = *.obj *.lib *.err main\descentr.exe

# What make flags to use
MFLAGS = /nologo

all: FORCE $(SUBSYSTEMS)
	
misc: FORCE
	cd misc && $(MAKE) $(MFLAGS)

includes: FORCE
	cd includes && $(MAKE) $(MFLAGS)

fix: FORCE
	cd fix && $(MAKE) $(MFLAGS)

cfile: FORCE
	cd cfile && $(MAKE) $(MFLAGS)

2d: FORCE
	cd 2d && $(MAKE) $(MFLAGS)

bios: FORCE
	cd bios && $(MAKE) $(MFLAGS)

iff: FORCE
	cd iff && $(MAKE) $(MFLAGS)

div: FORCE
	cd div && $(MAKE) $(MFLAGS)

mem: FORCE
	cd mem && $(MAKE) $(MFLAGS)

vecmat: FORCE
	cd vecmat && $(MAKE) $(MFLAGS)

3d: FORCE
	cd 3d && $(MAKE) $(MFLAGS)

texmap: FORCE
	cd texmap && $(MAKE) $(MFLAGS)

ui: FORCE
	cd ui && $(MAKE) $(MFLAGS)

main: FORCE
	cd main && $(MAKE) $(MFLAGS)

clean:
	erase /s /q $(CLEAN_TARGS)

FORCE:
