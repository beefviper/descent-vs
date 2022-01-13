#
# $Source: f:/miner/source/div/rcs/makefile $
# $Revision: 1.3 $
# $Author: matt $
# $Date: 1994/11/21 11:34:49 $
#
# Makefile for bios functions
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = div0.lib div0.h div0.inc

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = div0

# What objs in the lib
DIV0_OBJS =  div0.obj

div0.lib: $(DIV0_OBJS)
	wlib /q div0.lib $(DIV0_OBJS)

div0.obj: div0.asm
	$(ASM) $(ASMFLAGS) div0.asm

clean:
	erase /s /q $(CLEAN_TARGS)
