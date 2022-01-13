#
# $Source: f:/miner/source/3d/rcs/makefile $
# $Revision: 1.9 $
# $Author: matt $
# $Date: 1994/03/15 21:22:47 $
#
# Makefile for 3d library
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = 3d.lib 3d.h 3d.inc

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = 3d

# What objs in the lib
3D_OBJS = globvars.obj setup.obj matrix.obj points.obj draw.obj clipper.obj \
	horizon.obj instance.obj rod.obj interp.obj

3d.lib: $(3D_OBJS)
	wlib /q 3d.lib $(3D_OBJS)

3dtest.obj: 3dtest.c
	$(CC) $(CCFLAGS) 3dtest.c

clipper.obj: clipper.asm
	$(ASM) $(ASMFLAGS) clipper.asm

draw.obj: draw.asm
	$(ASM) $(ASMFLAGS) draw.asm

globvars.obj: globvars.asm
	$(ASM) $(ASMFLAGS) globvars.asm

horizon.obj: horizon.asm
	$(ASM) $(ASMFLAGS) horizon.asm

instance.obj: instance.asm
	$(ASM) $(ASMFLAGS) instance.asm

interp.obj: interp.asm
	$(ASM) $(ASMFLAGS) interp.asm

matrix.obj: matrix.asm
	$(ASM) $(ASMFLAGS) matrix.asm

objects.obj: objects.asm
	$(ASM) $(ASMFLAGS) objects.asm

out.obj: out.asm
	$(ASM) $(ASMFLAGS) out.asm

points.obj: points.asm
	$(ASM) $(ASMFLAGS) points.asm

rod.obj: rod.asm
	$(ASM) $(ASMFLAGS) rod.asm

segment.obj: segment.asm
	$(ASM) $(ASMFLAGS) segment.asm

setup.obj: setup.asm
	$(ASM) $(ASMFLAGS) setup.asm

upvecs.obj: upvecs.asm
	$(ASM) $(ASMFLAGS) upvecs.asm

clean:
	erase /s /q $(CLEAN_TARGS)
