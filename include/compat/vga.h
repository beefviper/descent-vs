/*
 * vga.h - state of the emulated VGA hardware (see src/compat/dos.c).
 *
 * The palette (DAC) ports 3C7h-3C9h are emulated: writes land in
 * compat_vga_dac (6-bit r,g,b per color) and bump compat_vga_dac_version,
 * which the video backend watches. Reads of the input status port 3DAh
 * toggle the retrace bits so wait loops finish.
 */

#ifndef _COMPAT_VGA_H
#define _COMPAT_VGA_H

extern unsigned char compat_vga_dac[768];
extern volatile unsigned compat_vga_dac_version;

#endif
