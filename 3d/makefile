#
# $Source: f:/miner/source/3d/rcs/makefile $
# $Revision: 1.9 $
# $Author: matt $
# $Date: 1994/03/15 21:22:47 $
#
# Makefile for 3d library
#

LIBDIR = ..\main
#

CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG


# What to install
INSTALL_TARGS = 3d.lib 3d.h 3d.inc

# What libs to build
LIBS = 3d

# What objs in the lib
3D_OBJS = globvars.obj setup.obj matrix.obj points.obj draw.obj clipper.obj \
	horizon.obj instance.obj rod.obj interp.obj

# What test programs to make
TEST_PROGS = 3dtest

# What object files and libs for the test programs
3DTEST_SRCS = 3dtest.c segment.c trakball.c
3DTEST_LIBS = 3d.lib vecmat.lib gr.lib io.lib fix.lib div0.lib texmap.lib misc.lib bm.lib

#rules and targets common to all Miner makefiles

3d.lib: $(3D_OBJS)
	wlib 3d.lib $(3D_OBJS)
	copy *.lib $(LIBDIR)
3dtest.obj: 3dtest.c
	wcc386 $(CCFLAGS) 3dtest.c

clipper.obj: clipper.asm
	ml /c clipper.asm clipper.obj
draw.obj: draw.asm
	ml /c draw.asm draw.obj
globvars.obj: globvars.asm
	ml /c globvars.asm globvars.obj
horizon.obj: horizon.asm
	ml /c horizon.asm horizon.obj
instance.obj: instance.asm 
	ml /c instance.asm instance.obj

interp.obj: interp.asm
	ml /c interp.asm interp.obj
matrix.obj: matrix.asm
	ml /c matrix.asm matrix.obj

objects.obj: objects.asm
	ml /c objects.asm objects.obj

out.obj: out.asm
	ml /c out.asm out.obj

points.obj: points.asm
	ml /c points.asm points.obj

rod.obj: rod.asm
	ml /c rod.asm rod.obj

segment.obj: segment.asm
	ml /c segment.asm segment.obj

setup.obj: setup.asm
	ml /c setup.asm setup.obj

#trakball.obj: trackball.asm

upvecs.obj: upvecs.asm
	ml /c upvecs.asm upvecs.obj

upvecs2.obj: fix.h types.h vecmat.h

