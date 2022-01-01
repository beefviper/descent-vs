#
# $Source: f:/miner/source/cfile/rcs/makefile $
# $Revision: 1.00 $
# $Author: beefviper $
# $Date: 2022/01/01 12:34:56 $
#
# Makefile for cfile functions
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c

CLEAN_TARGS = $(OBJDIR)\*.obj

INSTALL_TARGS = cfile.lib cfile.h nocfile.h

LIBS = cfile

CFILE_OBJS = cfile.obj

cfile.lib: $(CFILE_OBJS)
	wlib cfile.lib $(CFILE_OBJS)
	copy cfile.lib $(LIBDIR)

cfile.obj: cfile.h cfile.c
	$(CC) $(CCFLAGS) cfile.c

test.obj: test.c
	$(CC) $(CCFLAGS) test.c
