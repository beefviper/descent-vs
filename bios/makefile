#
# $Source: f:/miner/source/bios/rcs/makefile $
# $Revision: 1.18 $
# $Author: john $
# $Date: 1995/03/23 18:27:04 $
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
INSTALL_TARGS = io.lib joy.h key.h mono.h mouse.h timer.h swift.h dpmi.h ipx.h

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = io

# What objs in the lib
IO_OBJS = key.obj joy.obj mouse.obj timer.obj mono.obj joyc.obj dpmi.obj ipx.obj

io.lib: $(IO_OBJS)
	wlib /q io.lib $(IO_OBJS)

cdrom.obj: cdrom.c
	$(CC) $(CCFLAGS) cdrom.c

demo.obj: demo.c
	$(CC) $(CCFLAGS) demo.c

dpmi.obj: dpmi.c
	$(CC) $(CCFLAGS) dpmi.c

ipx.obj: ipx.c
	$(CC) $(CCFLAGS) ipx.c

joyc.obj: joyc.c
	$(CC) $(CCFLAGS) joyc.c

joy.obj: joy.asm
	$(ASM) $(ASMFLAGS) joy.asm

key.obj: key.c
	$(CC) $(CCFLAGS) key.c

mono.obj: mono.c
	$(CC) $(CCFLAGS) mono.c

mouse.obj: mouse.c
	$(CC) $(CCFLAGS) mouse.c

testj.obj: testj.c
	$(CC) $(CCFLAGS) testj.c

testk.obj: testk.c
	$(CC) $(CCFLAGS) testk.c

testm.obj: testm.c
	$(CC) $(CCFLAGS) testm.c

testmem.obj: testmem.c
	$(CC) $(CCFLAGS) testmem.c

testms.obj: testms.c
	$(CC) $(CCFLAGS) testms.c

testt.obj: testt.c
	$(CC) $(CCFLAGS) testt.c

timer.obj: timer.asm
	$(ASM) $(ASMFLAGS) timer.asm

clean:
	erase /s /q $(CLEAN_TARGS)
