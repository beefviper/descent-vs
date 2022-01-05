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

# What to install, and where to install it
INSTALL_TARGS = div0.lib div0.h div0.inc

# What to clean
CLEAN_TARGS = $(OBJDIR)\*.obj

# What libs to make
LIBS = div0

# What test programs to make
TEST_PROGS = testd

# What object files for the lib
DIV0_OBJS =  div0.obj

# What object files and libs for the test programs
TESTD_SRCS = testd.c testda.asm
TESTD_LIBS = div0.lib

div0.lib: $(DIV0_OBJS)
	wlib /q div0.lib $(DIV0_OBJS)
	copy div0.lib $(LIBDIR)

div0.obj: div0.asm
	$(ASM) $(ASMFLAGS) div0.asm

testd.obj: 

clean:
	erase /s /q *.obj *.lib *.err
