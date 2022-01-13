#
# $Source: f:/miner/source/iff/rcs/makefile $
# $Revision: 1.5 $
# $Author: john $
# $Date: 1994/02/15 12:51:20 $
#
# Makefile for IFF library
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = iff.lib iff.h

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = iff

# What objs in the lib
IFF_OBJS = iff.obj

iff.lib: $(IFF_OBJS)
	wlib /q iff.lib $(IFF_OBJS)

iff.obj: iff.c
	$(CC) $(CCFLAGS) iff.c

clean:
	erase /s /q $(CLEAN_TARGS)
