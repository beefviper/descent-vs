#
# $Source: f:/miner/source/texmap/rcs/makefile $
# $Revision: 1.23 $
# $Author: john $
# $Date: 1995/02/20 18:23:00 $
#
# Makefile for hack3df testbed
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = texmap.lib texmap.h

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = texmap

# What objs in the lib
TEXMAP_OBJS = ntmap.obj tmapflat.obj tmap_per.obj tmap_lin.obj \
	tmap_ll.obj tmap_flt.obj tmapfade.obj tmap_fl1.obj scanline.obj
#TEXMAP_OBJS = ntmap.obj tmapsky.obj tmapv.obj tmapflat.obj tmap_per.obj tmap_p2.obj tmap_lin.obj tmap_ll.obj tmap_kl.obj\
#	tmap_ld.obj tmap_flt.obj tmap_sky.obj tmap_skv.obj tmap_fl1.obj tmapz.obj tmap_z.obj tmapfade.obj tmap_mul.obj

texmap.lib: $(TEXMAP_OBJS)
	wlib /q texmap.lib $(TEXMAP_OBJS)
	copy texmap.lib $(LIBDIR)

lightabs.obj: lightabs.asm
	$(ASM) $(ASMFLAGS) lightabs.asm

ntmap.obj: ntmap.c
	$(CC) $(CCFLAGS) ntmap.c

scanline.obj: scanline.c
	$(CC) $(CCFLAGS) scanline.c

tmap_16.obj: tmap_16.asm
	$(ASM) $(ASMFLAGS) tmap_16.asm

tmap_16g.obj: tmap_16g.asm
	$(ASM) $(ASMFLAGS) tmap_16g.asm

tmap_16l.obj: tmap_16l.asm
	$(ASM) $(ASMFLAGS) tmap_16l.asm

tmap_16p.obj: tmap_16p.asm
	$(ASM) $(ASMFLAGS) tmap_16p.asm
	
tmap_16r.obj: tmap_16r.asm
	$(ASM) $(ASMFLAGS) tmap_16r.asm

tmap_aa.obj: tmap_aa.asm
	$(ASM) $(ASMFLAGS) tmap_aa.asm

tmap_fl1.obj: tmap_fl1.asm
	$(ASM) $(ASMFLAGS) tmap_fl1.asm

tmap_flt.obj: tmap_flt.asm
	$(ASM) $(ASMFLAGS) tmap_flt.asm

tmap_kl.obj: tmap_kl.asm
	$(ASM) $(ASMFLAGS) tmap_kl.asm

tmap_ld.obj: tmap_ld.asm
	$(ASM) $(ASMFLAGS) tmap_ld.asm

tmap_lin.obj: tmap_lin.asm
	$(ASM) $(ASMFLAGS) tmap_lin.asm

tmap_ll.obj: tmap_ll.asm
	$(ASM) $(ASMFLAGS) tmap_ll.asm

tmap_mul.obj: tmap_mul.asm
	$(ASM) $(ASMFLAGS) tmap_mul.asm

tmap_p2.obj: tmap_p2.asm
	$(ASM) $(ASMFLAGS) tmap_p2.asm

tmap_per.obj: tmap_per.asm
	$(ASM) $(ASMFLAGS) tmap_per.asm

tmap_rgb.obj: tmap_rgb.asm
	$(ASM) $(ASMFLAGS) tmap_rgb.asm

tmap_skv.obj: tmap_skv.asm
	$(ASM) $(ASMFLAGS) tmap_skv.asm

tmap_sky.obj: tmap_sky.asm
	$(ASM) $(ASMFLAGS) tmap_sky.asm

tmap_z.obj: tmap_z.asm
	$(ASM) $(ASMFLAGS) tmap_z.asm

tmapdith.obj: tmapdith.asm
	$(ASM) $(ASMFLAGS) tmapdith.asm

tmapfade.obj: tmapfade.asm
	$(ASM) $(ASMFLAGS) tmapfade.asm

tmapflat.obj: tmapflat.c
	$(CC) $(CCFLAGS) tmapflat.c

tmapsky.obj: tmapsky.asm
	$(ASM) $(ASMFLAGS) tmapsky.asm

tmapv.obj: tmapv.asm
	$(ASM) $(ASMFLAGS) tmapv.asm

tmapz.obj: tmapz.asm
	$(ASM) $(ASMFLAGS) tmapz.asm

clean:
	erase /s /q $(CLEAN_TARGS)
