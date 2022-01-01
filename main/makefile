#
# Makefile for DESCENT execuable
#

LIBDIR = c:\dev\d1src\main

# What compiler and flags to use
CC = wcc386
CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG

# What assembler and flags to use
ASM = ml
ASMFLAGS = /c

#

DESCENT_LIBS =	3d.lib gr.lib fix.lib io.lib iff.lib vecmat.lib \
        	 	cfile.lib \
        	 	mem.lib \
        	 	ui.lib \
				div0.lib misc.lib texmap.lib
				
#				editor.lib

#		sosdigi.lib #sosmidi.lib \
		#iglasses.lib gclfr32.lib loadpats.lib
		#lzw.lib sosdwxcr.lib sosmwxcr.lib 

DESCENT_OBJS = inferno.obj game.obj gauges.obj mglobal.obj \
			 scores.obj object.obj laser.obj physics.obj bm.obj \
 			 menu.obj gamesave.obj switch.obj effects.obj texmerge.obj polyobj.obj \
			 gamemine.obj fireball.obj ai.obj aipath.obj hostage.obj \
			 powerup.obj fuelcen.obj digi.obj render.obj hash.obj piggy.obj args.obj \
			 gameseg.obj wall.obj collide.obj lighting.obj \
			 robot.obj morph.obj vclip.obj weapon.obj fvi.obj newdemo.obj titles.obj \
			 vfx.obj gameseq.obj controls.obj automap.obj text.obj \
			 network.obj newmenu.obj netmisc.obj victor.obj\
			 gamefont.obj joydefs.obj hud.obj playsave.obj \
			 endlevel.obj terrain.obj kconfig.obj modem.obj  \
			 multi.obj cntrlcen.obj credits.obj config.obj soscodec.obj kmatrix.obj	\
			 paging.obj mission.obj iglasses.obj songs.obj bmread.obj state.obj multibot.obj nocomlib.obj slew.obj dumpmine.obj 
#demo.obj garage.obj radar.obj coindev.obj serial.obj vfxread.obj victor.obj


# Files to link for Descent

DESCENTR.EXE: 	$(DESCENT_OBJS)
	wcl386 /fe=descentr.exe /l=dos4g /k50000 $(DESCENT_OBJS) $(DESCENT_LIBS)

ai.obj:	 ai.c ai.h
	$(CC) $(CCFLAGS) ai.c

aipath.obj: aipath.c
	$(CC) $(CCFLAGS) aipath.c

arcade.obj: arcade.c 
	$(CC) $(CCFLAGS) arcade.c

args.obj: args.c args.h
	$(CC) $(CCFLAGS) args.c

automap.obj: automap.c 
	$(CC) $(CCFLAGS) automap.c
	
autosave.obj: autosave.c 
	$(CC) $(CCFLAGS) autosave.c
	
bm.obj:	 bm.c 
	$(CC) $(CCFLAGS) bm.c

bmread.obj: bmread.c 
	$(CC) $(CCFLAGS) bmread.c
	
centers.obj: centers.c 
	$(CC) $(CCFLAGS) centers.c
	
cntrlcen.obj: cntrlcen.c 
	$(CC) $(CCFLAGS) cntrlcen.c

cockpit.obj: cockpit.asm
	$(ASM) $(ASMFLAGS) /i$INCLUDE cockpit.asm cockpit.obj

#cockpit.inc psmacros.inc types.inc

coindev.obj: coindev.c coindev.h 
	$(CC) $(CCFLAGS) coindev.c

collide.obj: collide.c collide.h
	$(CC) $(CCFLAGS) collide.c

config.obj: config.c 
	$(CC) $(CCFLAGS) config.c

controls.obj: controls.c 
	$(CC) $(CCFLAGS) controls.c

credits.obj: credits.c 
	$(CC) $(CCFLAGS) credits.c

curves.obj: curves.c 
	$(CC) $(CCFLAGS) curves.c

demo.obj: demo.c 
	$(CC) $(CCFLAGS) demo.c

digi.obj: digi.c 
	$(CC) $(CCFLAGS) digi.c

dumpmine.obj: dumpmine.c 
	$(CC) $(CCFLAGS) dumpmine.c

effects.obj: effects.c 
	$(CC) $(CCFLAGS) effects.c

eglobal.obj: eglobal.c 
	$(CC) $(CCFLAGS) eglobal.c

ehostage.obj: ehostage.c 
	$(CC) $(CCFLAGS) ehostage.c

einit.obj: einit.c 
	$(CC) $(CCFLAGS) einit.c

elight.obj: elight.c 
	$(CC) $(CCFLAGS) elight.c

endlevel.obj: endlevel.c 
	$(CC) $(CCFLAGS) endlevel.c

eobject.obj: eobject.c 
	$(CC) $(CCFLAGS) eobject.c

eswitch.obj: eswitch.c 
	$(CC) $(CCFLAGS) eswitch.c

fireball.obj: fireball.c 
	$(CC) $(CCFLAGS) fireball.c

fixseg.obj: fixseg.c 
	$(CC) $(CCFLAGS) fixseg.c

flyby.obj: flyby.c 
	$(CC) $(CCFLAGS) flyby.c

fuelcen.obj: fuelcen.c 
	$(CC) $(CCFLAGS) fuelcen.c

func.obj: func.c 
	$(CC) $(CCFLAGS) func.c

fvi.obj: fvi.c 
	$(CC) $(CCFLAGS) fvi.c

game.obj: game.c 
	$(CC) $(CCFLAGS) game.c

gamefont.obj: gamefont.c
	$(CC) $(CCFLAGS) gamefont.c

gamemine.obj: gamemine.c 
	$(CC) $(CCFLAGS) gamemine.c

gamesave.obj: gamesave.c 
	$(CC) $(CCFLAGS) gamesave.c

gameseg.obj: gameseg.c 
	$(CC) $(CCFLAGS) gameseg.c

gameseq.obj: gameseq.c
	$(CC) $(CCFLAGS) gameseq.c

garage.obj: garage.c 
	$(CC) $(CCFLAGS) garage.c

gauges.obj: gauges.c
	$(CC) $(CCFLAGS) gauges.c

group.obj: group.c
	$(CC) $(CCFLAGS) group.c

hash.obj: hash.c
	$(CC) $(CCFLAGS) hash.c

hostage.obj: hostage.c
	$(CC) $(CCFLAGS) hostage.c

hud.obj: hud.c
	$(CC) $(CCFLAGS) hud.c

iglasses.obj: iglasses.c
	$(CC) $(CCFLAGS) iglasses.c

inferno.obj: inferno.c
	$(CC) $(CCFLAGS) inferno.c

info.obj: info.c
	$(CC) $(CCFLAGS) info.c

int3hand.obj: psmacros.inc types.inc
	$(ASM) $(ASMFLAGS) int3hand.asm int3hand.obj

ipx_esr.obj: psmacros.inc types.inc
	$(ASM) $(ASMFLAGS) ipx_esr.asm ipx_esr.obj

joydefs.obj: joydefs.c
	$(CC) $(CCFLAGS) joydefs.c

kbuild.obj: 
	$(CC) $(CCFLAGS) kbuild.c

kconfig.obj: 
	$(CC) $(CCFLAGS) kconfig.c

kcurve.obj: 
	$(CC) $(CCFLAGS) kcurve.c

kfuncs.obj: 
	$(CC) $(CCFLAGS) kfuncs.c

kgame.obj: 
	$(CC) $(CCFLAGS) kgame.c

khelp.obj: 
	$(CC) $(CCFLAGS) khelp.c

kmatrix.obj: 
	$(CC) $(CCFLAGS) kmatrix.c

kmine.obj:
	$(CC) $(CCFLAGS) kmine.c

ksegmove.obj:
	$(CC) $(CCFLAGS) ksegmove.c

ksegsize.obj:
	$(CC) $(CCFLAGS) ksegsize.c

ktmap.obj:
	$(CC) $(CCFLAGS) ktmap.c

kview.obj:
	$(CC) $(CCFLAGS) kview.c

laser.obj:
	$(CC) $(CCFLAGS) laser.c

lighting.obj:
	$(CC) $(CCFLAGS) lighting.c

macro.obj: 
	$(CC) $(CCFLAGS) macro.c

mcockpit.obj: 
	$(CC) $(CCFLAGS) mcockpit.c

med.obj: 
	$(CC) $(CCFLAGS) med.c

meddraw.obj: 
	$(CC) $(CCFLAGS) meddraw.c

medmisc.obj: 
	$(CC) $(CCFLAGS) medmisc.c

medrobot.obj: 
	$(CC) $(CCFLAGS) medrobot.c

medsel.obj: 
	$(CC) $(CCFLAGS) medsel.c

medwall.obj: 
	$(CC) $(CCFLAGS) medwall.c

menu.obj: 
	$(CC) $(CCFLAGS) menu.c

mglobal.obj: 
	$(CC) $(CCFLAGS) mglobal.c

mine.obj: 
	$(CC) $(CCFLAGS) mine.c

mission.obj: 
	$(CC) $(CCFLAGS) mission.c

modem.obj: 
	$(CC) $(CCFLAGS) modem.c

morph.obj: 
	$(CC) $(CCFLAGS) morph.c

multi.obj: 
	$(CC) $(CCFLAGS) multi.c

multibot.obj:
	$(CC) $(CCFLAGS) multibot.c

netmisc.obj: 
	$(CC) $(CCFLAGS) netmisc.c

netware.obj: 
	$(CC) $(CCFLAGS) netware.c

network.obj: 
	$(CC) $(CCFLAGS) network.c

newdemo.obj: 
	$(CC) $(CCFLAGS) newdemo.c

newdemo1.obj:
	$(CC) $(CCFLAGS) newdemo1.c

newmenu.obj: 
	$(CC) $(CCFLAGS) newmenu.c

object.obj: 
	$(CC) $(CCFLAGS) object.c

objfly.obj: 
	$(CC) $(CCFLAGS) objfly.c

objpage.obj: 
	$(CC) $(CCFLAGS) objpage.c

ourcom.obj: 
	$(CC) $(CCFLAGS) ourcom.c

paging.obj: 
	$(CC) $(CCFLAGS) paging.c

photos.obj: 
	$(CC) $(CCFLAGS) photos.c

physics.obj:
	$(CC) $(CCFLAGS) physics.c

piggy.obj: 
	$(CC) $(CCFLAGS) piggy.c

playsave.obj: 
	$(CC) $(CCFLAGS) playsave.c

polyobj.obj: 
	$(CC) $(CCFLAGS) polyobj.c

powerup.obj: 
	$(CC) $(CCFLAGS) powerup.c

radar.obj: 
	$(CC) $(CCFLAGS) radar.c

render.obj: 
	$(CC) $(CCFLAGS) render.c

robot.obj: 
	$(CC) $(CCFLAGS) robot.c

scores.obj: 
	$(CC) $(CCFLAGS) scores.c

segment.obj: 
	$(CC) $(CCFLAGS) segment.c

seguvs.obj: 
	$(CC) $(CCFLAGS) seguvs.c

serial.obj: 
	$(CC) $(CCFLAGS) serial.c

ship.obj: 
	$(CC) $(CCFLAGS) ship.c

slew.obj: 
	$(CC) $(CCFLAGS) slew.c

songs.obj: 
	$(CC) $(CCFLAGS) songs.c

soscodec.obj: soscodec.asm
	$(ASM) $(ASMFLAGS) soscodec.asm soscodec.obj

state.obj: 
	$(CC) $(CCFLAGS) state.c

switch.obj:
	$(CC) $(CCFLAGS) switch.c

tempgame.obj: 
	$(CC) $(CCFLAGS) tempgame.c

terrain.obj: 
	$(CC) $(CCFLAGS) terrain.c

test1.obj:
	$(CC) $(CCFLAGS) test1.c

testgaug.obj:
	$(CC) $(CCFLAGS) testgaug.c

texmerge.obj:
	$(CC) $(CCFLAGS) texmerge.c

texpage.obj: 
	$(CC) $(CCFLAGS) texpage.c

text.obj: 
	$(CC) $(CCFLAGS) text.c

texture.obj: 
	$(CC) $(CCFLAGS) texture.c

titles.obj: 
	$(CC) $(CCFLAGS) titles.c

vclip.obj: 
	$(CC) $(CCFLAGS) vclip.c

vfx.obj: 
	$(CC) $(CCFLAGS) vfx.c

#vfxread.obj: 
#	$(CC) $(CCFLAGS) vfxread.c

victor.obj: 
	$(CC) $(CCFLAGS) victor.c

wall.obj: 
	$(CC) $(CCFLAGS) wall.c

weapon.obj: 
	$(CC) $(CCFLAGS) weapon.c

nocomlib.obj: nocomlib.c
	$(CC) $(CCFLAGS) nocomlib.c
