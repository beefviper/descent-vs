#
# $Source: f:/miner/source/vecmat/rcs/makefile $
# $Revision: 1.2 $
# $Author: matt $
# $Date: 1993/09/24 21:17:56 $
#
# Makefile for vector/matrix library
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = vecmat.lib vecmat.h vecmat.inc

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = vecmat

# What objs in the lib
VECMAT_OBJS = vecmat.obj

vecmat.lib: $(VECMAT_OBJS)
	wlib /q vecmat.lib $(VECMAT_OBJS)

vecmat.obj: vecmat.asm vecmat.inc
	$(ASM) $(ASMFLAGS) vecmat.asm

clean:
	erase /s /q $(CLEAN_TARGS)
