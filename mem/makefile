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
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c

# What to install, and where to install it
INSTALL_TARGS = mem.lib mem.h

# What to clean
CLEAN_TARGS = $(OBJDIR)\*.obj

# What libs to make
LIBS = mem

MEM_OBJS = mem.obj

mem.lib: $(MEM_OBJS)
	wlib mem.lib $(MEM_OBJS)
	copy mem.lib $(LIBDIR)

mem.obj: mem.h mem.c
	$(CC) $(CCFLAGS) mem.c

clean:
	erase /s /q *.obj *.lib *.err
