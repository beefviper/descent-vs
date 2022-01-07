#
# $Source: f:/miner/source/mem/rcs/makefile $
# $Revision: 1.1 $
# $Author: john $
# $Date: 1993/11/02 17:45:38 $
#
# Makefile for memory debugging
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = mem.lib mem.h

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = mem

# What objs in the lib
MEM_OBJS = mem.obj

mem.lib: $(MEM_OBJS)
	wlib /q mem.lib $(MEM_OBJS)
	copy mem.lib $(LIBDIR)

mem.obj: mem.c mem.h
	$(CC) $(CCFLAGS) mem.c

clean:
	erase /s /q $(CLEAN_TARGS)
