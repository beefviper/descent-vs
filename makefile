#
# $Source: f:/miner/source/rcs/makefile $
# $Revision: 1.1 $
# $Author: beefviper $
# $Date: 2022/01/07 22:27:23 $
#
# Makefile for Descent executable 
#

# What dir to build in
BUILD_DIR = build

# What subsystems to make 
SUBSYSTEMS = $(BUILD_DIR)\misc.lib $(BUILD_DIR)\fix.lib \
	$(BUILD_DIR)\cfile.lib $(BUILD_DIR)\gr.lib $(BUILD_DIR)\io.lib $(BUILD_DIR)\iff.lib \
	$(BUILD_DIR)\div0.lib $(BUILD_DIR)\mem.lib $(BUILD_DIR)\vecmat.lib $(BUILD_DIR)\3d.lib \
	$(BUILD_DIR)\texmap.lib $(BUILD_DIR)\ui.lib $(BUILD_DIR)\main.lib $(BUILD_DIR)\inferno.obj
	# readd $(BUILD_DIR)\incudes later

# What to clean
CLEAN_TARGS = *.obj *.lib *.err main\descentr.exe

# What make flags to use
MFLAGS = /nologo

$(BUILD_DIR)\descentr.exe: makebuilddir $(SUBSYSTEMS)
	cd $(BUILD_DIR) && wcl386 /zq /fe=descentr.exe /l=dos4g /k50000 inferno.obj main.lib 3d.lib gr.lib fix.lib io.lib iff.lib vecmat.lib \
	cfile.lib mem.lib ui.lib div0.lib misc.lib texmap.lib
	
makebuilddir:
	@mkdir $(BUILD_DIR)

$(BUILD_DIR)\misc.lib: misc\misc.lib
	copy misc\misc.lib $(BUILD_DIR)

misc\misc.lib:
	cd misc && $(MAKE) $(MFLAGS)

includes:
	cd includes && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\fix.lib: fix\fix.lib
	copy fix\fix.lib $(BUILD_DIR)

fix\fix.lib:
	cd fix && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\cfile.lib: cfile\cfile.lib
	copy cfile\cfile.lib $(BUILD_DIR)

cfile\cfile.lib:
	cd cfile && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\gr.lib: 2d\gr.lib
	copy 2d\gr.lib $(BUILD_DIR)

2d\gr.lib:
	cd 2d && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\io.lib: bios\io.lib
	copy bios\io.lib $(BUILD_DIR)

bios\io.lib:
	cd bios && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\iff.lib: iff\iff.lib
	copy iff\iff.lib $(BUILD_DIR)

iff\iff.lib:
	cd iff && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\div0.lib: div\div0.lib
	copy div\div0.lib $(BUILD_DIR)

div\div0.lib:
	cd div && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\mem.lib: mem\mem.lib
	copy mem\mem.lib $(BUILD_DIR)

mem\mem.lib:
	cd mem && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\vecmat.lib: vecmat\vecmat.lib
	copy vecmat\vecmat.lib $(BUILD_DIR)

vecmat\vecmat.lib:
	cd vecmat && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\3d.lib: 3d\3d.lib
	copy 3d\3d.lib $(BUILD_DIR)

3d\3d.lib:
	cd 3d && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\texmap.lib: texmap\texmap.lib
	copy texmap\texmap.lib $(BUILD_DIR)

texmap\texmap.lib:
	cd texmap && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\ui.lib: ui\ui.lib
	copy ui\ui.lib $(BUILD_DIR)

ui\ui.lib:
	cd ui && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\main.lib: main\main.lib
	copy main\main.lib $(BUILD_DIR)

main\main.lib:
	cd main && $(MAKE) $(MFLAGS)

$(BUILD_DIR)\inferno.obj: main\inferno.obj
	copy main\inferno.obj $(BUILD_DIR)

clean:
	erase /s /q $(CLEAN_TARGS)
