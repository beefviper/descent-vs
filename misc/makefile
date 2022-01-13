#
# $Source: f:/miner/source/misc/rcs/makefile $
# $Revision: 1.4 $
# $Author: matt $
# $Date: 1994/11/27 22:27:23 $
#
# Makefile for misc libaray
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = misc.lib parsarg.h error.h

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = misc

# What objs in the lib
MISC_OBJS = parsarg.obj error.obj

misc.lib: $(MISC_OBJS)
	wlib /q misc.lib $(MISC_OBJS)

error.obj: error.c error.h
	$(CC) $(CCFLAGS) error.c

parsarg.obj: parsarg.c parsarg.h
	$(CC) $(CCFLAGS) parsarg.c

clean:
	erase /s /q $(CLEAN_TARGS)
