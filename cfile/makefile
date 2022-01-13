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
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = cfile.lib cfile.h nocfile.h

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = cfile

# What objs in the lib
CFILE_OBJS = cfile.obj

cfile.lib: $(CFILE_OBJS)
	wlib /q cfile.lib $(CFILE_OBJS)

cfile.obj: cfile.c cfile.h
	$(CC) $(CCFLAGS) cfile.c

clean:
	erase /s /q $(CLEAN_TARGS)
