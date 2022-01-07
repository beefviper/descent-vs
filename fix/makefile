#
# $Source: f:/miner/source/fix/rcs/makefile $
# $Revision: 1.5 $
# $Author: matt $
# $Date: 1994/11/27 23:21:18 $
#
# Makefile for fixed-point library
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = fix.lib fix.h fix.inc

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = fix

# What objs in the lib
FIX_OBJS = fix.obj 

fix.lib: $(FIX_OBJS)
	wlib /q fix.lib $(FIX_OBJS)
	copy fix.lib $(LIBDIR)

fix.obj: fix.asm fix.inc
	$(ASM) $(ASMFLAGS) fix.asm

clean:
	erase /s /q $(CLEAN_TARGS)
