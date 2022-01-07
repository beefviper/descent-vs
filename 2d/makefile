#
# $Source: f:/miner/source/2d/rcs/makefile $
# $Revision: 1.35 $
# $Author: john $
# $Date: 1995/01/29 17:08:57 $
#
# Makefile for 2D graphics functions
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = effect2d.h gr.h gr.lib grdef.h gr.inc palette.h ibitblt.h rle.h pcx.h

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = gr

# What objs in the lib
GR_OBJS = linear.obj vesa.obj modex.obj canvas.obj bitmap.obj gr.obj font.obj \
	bitblt.obj scanline.obj pixel.obj poly.obj disc.obj rect.obj box.obj \
	gpixel.obj palette.obj line.obj lbitblt.obj \
	scale.obj scalea.obj circle.obj ibitblt.obj rle.obj pcx.obj

gr.lib: $(GR_OBJS)
	wlib /q gr.lib $(GR_OBJS)
	copy gr.lib $(LIBDIR)

bitblt.obj: bitblt.c
	$(CC) $(CCFLAGS) bitblt.c

bitmap.obj: bitmap.c
	$(CC) $(CCFLAGS) bitmap.c

box.obj: box.c
	$(CC) $(CCFLAGS) box.c

canvas.obj: canvas.c
	$(CC) $(CCFLAGS) canvas.c

circle.obj: circle.c
	$(CC) $(CCFLAGS) circle.c

disc.obj: disc.c
	$(CC) $(CCFLAGS) disc.c

effects.obj: effects.c
	$(CC) $(CCFLAGS) effects.c

fdraw.obj: fdraw.c
	$(CC) $(CCFLAGS) fdraw.c

font.obj: font.c
	$(CC) $(CCFLAGS) font.c

gpixel.obj: gpixel.c
	$(CC) $(CCFLAGS) gpixel.c

gr.obj: gr.c
	$(CC) $(CCFLAGS) gr.c

ibitblt.obj: ibitblt.c
	$(CC) $(CCFLAGS) ibitblt.c

inv_cmap.obj: inv_cmap.c
	$(CC) $(CCFLAGS) inv_cmap.c

l.obj: l.obj
	$(CC) $(CCFLAGS) l.c

lbitblt.obj: lbitblt.asm
	$(ASM) $(ASMFLAGS) lbitblt.asm

line.obj: line.c
	$(CC) $(CCFLAGS) line.c

linear.obj: linear.asm
	$(ASM) $(ASMFLAGS) linear.asm

median.obj: median.c
	$(CC) $(CCFLAGS) median.c

modex.obj: modex.asm
	$(ASM) $(ASMFLAGS) modex.asm

mspeed.obj: mspeed.c
	$(CC) $(CCFLAGS) mspeed.c

palette.obj: palette.c
	$(CC) $(CCFLAGS) palette.c

pcx.obj: pcx.c
	$(CC) $(CCFLAGS) pcx.c

pixel.obj: pixel.c
	$(CC) $(CCFLAGS) pixel.c

poly.obj: poly.c
	$(CC) $(CCFLAGS) poly.c

rect.obj: rect.c
	$(CC) $(CCFLAGS) rect.c

rle.obj: rle.c
	$(CC) $(CCFLAGS) rle.c

roller.obj: roller.c
	$(CC) $(CCFLAGS) roller.c

scale.obj: scale.c
	$(CC) $(CCFLAGS) scale.c

scalea.obj: scalea.asm
	$(ASM) $(ASMFLAGS) scalea.asm

scanline.obj: scanline.c
	$(CC) $(CCFLAGS) scanline.c

testg.obj: testg.c
	$(CC) $(CCFLAGS) testg.c

vesa.obj: vesa.asm
	$(ASM) $(ASMFLAGS) vesa.asm

vesalib.obj: vesalib.c
	$(CC) $(CCFLAGS) vesalib.c

vesatest.obj: vesatest.c
	$(CC) $(CCFLAGS) vesatest.c

clean:
	erase /s /q $(CLEAN_TARGS)
