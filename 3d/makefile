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
	copy 3d.lib $(LIBDIR)

3dtest.obj: 3dtest.c
	$(CC) $(CCFLAGS) 3dtest.c

clipper.obj: clipper.asm
	$(ASM) $(ASMFLAGS) clipper.asm clipper.obj

draw.obj: draw.asm
	$(ASM) $(ASMFLAGS) draw.asm draw.obj

globvars.obj: globvars.asm
	$(ASM) $(ASMFLAGS) globvars.asm globvars.obj

horizon.obj: horizon.asm
	$(ASM) $(ASMFLAGS) horizon.asm horizon.obj

instance.obj: instance.asm
	$(ASM) $(ASMFLAGS) instance.asm instance.obj

interp.obj: interp.asm
	$(ASM) $(ASMFLAGS) interp.asm interp.obj

matrix.obj: matrix.asm
	$(ASM) $(ASMFLAGS) matrix.asm matrix.obj

objects.obj: objects.asm
	$(ASM) $(ASMFLAGS) objects.asm objects.obj

out.obj: out.asm
	$(ASM) $(ASMFLAGS) out.asm out.obj

points.obj: points.asm
	$(ASM) $(ASMFLAGS) points.asm points.obj

rod.obj: rod.asm
	$(ASM) $(ASMFLAGS) rod.asm rod.obj

segment.obj: segment.asm
	$(ASM) $(ASMFLAGS) segment.asm segment.obj

setup.obj: setup.asm
	$(ASM) $(ASMFLAGS) setup.asm setup.obj

#trakball.obj: trackball.asm

upvecs.obj: upvecs.asm
	$(ASM) $(ASMFLAGS) upvecs.asm upvecs.obj

upvecs2.obj: fix.h types.h vecmat.h

clean:
	erase /s /q $(CLEAN_TARGS)
