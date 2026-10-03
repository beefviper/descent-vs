/*
 * compat.h - force-included into every C file when building with a
 * compiler other than Watcom.
 *
 * The original sources were written for Watcom C/386 under DOS/4GW. This
 * header neutralizes the Watcom keywords they use so the code compiles
 * with MSVC, MinGW and other modern compilers. Replacements for the DOS
 * headers (dos.h, i86.h, bios.h) live next to this file.
 */

#ifndef _COMPAT_H
#define _COMPAT_H

#ifndef __WATCOMC__

/* Segmented-memory and calling-convention keywords: meaningless in a flat
   32/64-bit address space. */
#define far
#define _far
#define __far
#define near
#define _near
#define __near
#define huge
#define _loadds
#define __loadds
#define interrupt
#define _interrupt
#define __interrupt

/* Watcom's stdlib.h provides min() and max() macros */
#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#endif

/* Watcom's errno.h names the "no error" value */
#ifndef EZERO
#define EZERO 0
#endif

/* Watcom made the DOS services in dos.h visible through conio.h and other
   headers as well; make them available everywhere. */
#include "dos.h"

#endif

#endif
