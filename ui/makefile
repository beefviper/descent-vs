#
# $Source: f:/miner/source/ui/rcs/makefile $
# $Revision: 1.8 $
# $Author: matt $
# $Date: 1994/11/27 14:53:50 $
#
# Makefile for user-interface functions
#

LIBDIR = ..\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG /zq

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c /nologo

# What to install
INSTALL_TARGS = ui.lib ui.h func.h

# What to clean
CLEAN_TARGS = *.obj *.lib *.err

# What libs to build
LIBS = ui

# What object in the lib
UI_OBJS = ui.obj mouse.obj window.obj uidraw.obj button.obj \
		  gadget.obj listbox.obj message.obj popup.obj scroll.obj \
		  radio.obj checkbox.obj inputbox.obj userbox.obj menu.obj \
		  harderr.obj file.obj keytrap.obj icon.obj keypress.obj \
		  menubar.obj keypad.obj

ui.lib: $(UI_OBJS)
	wlib /q ui.lib $(UI_OBJS)

barbox.obj: barbox.c
	$(CC) $(CCFLAGS) barbox.c

button.obj: button.c
	$(CC) $(CCFLAGS) button.c

checkbox.obj: checkbox.c
	$(CC) $(CCFLAGS) checkbox.c

file.obj: file.c
	$(CC) $(CCFLAGS) file.c

func.obj: func.c
	$(CC) $(CCFLAGS) func.c

gadget.obj: gadget.c
	$(CC) $(CCFLAGS) gadget.c

harderr.obj: harderr.c
	$(CC) $(CCFLAGS) harderr.c

icon.obj: icon.c
	$(CC) $(CCFLAGS) icon.c

inputbox.obj: inputbox.c
	$(CC) $(CCFLAGS) inputbox.c

keypad.obj: keypad.c
	$(CC) $(CCFLAGS) keypad.c

keypress.obj: keypress.c
	$(CC) $(CCFLAGS) keypress.c

keytrap.obj: keytrap.c
	$(CC) $(CCFLAGS) keytrap.c

listbox.obj: listbox.c
	$(CC) $(CCFLAGS) listbox.c

menu.obj: menu.c
	$(CC) $(CCFLAGS) menu.c

menubar.obj: menubar.c
	$(CC) $(CCFLAGS) menubar.c

message.obj: message.c
	$(CC) $(CCFLAGS) message.c

mouse.obj: mouse.c
	$(CC) $(CCFLAGS) mouse.c

number.obj: number.c
	$(CC) $(CCFLAGS) number.c

popup.obj: popup.c
	$(CC) $(CCFLAGS) popup.c

radio.obj: radio.c
	$(CC) $(CCFLAGS) radio.c

scroll.obj: scroll.c
	$(CC) $(CCFLAGS) scroll.c

ui.obj:	 ui.c
	$(CC) $(CCFLAGS) ui.c

uidraw.obj: uidraw.c
	$(CC) $(CCFLAGS) uidraw.c

userbox.obj: userbox.c
	$(CC) $(CCFLAGS) userbox.c

window.obj: window.c
	$(CC) $(CCFLAGS) window.c

clean:
	erase /s /q $(CLEAN_TARGS)
