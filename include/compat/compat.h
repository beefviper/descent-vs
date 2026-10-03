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

/* Unix permission bits for open()/creat(). MSVC's sys/stat.h only has
   _S_IREAD and _S_IWRITE, and Windows has no group permissions. */
#ifdef _MSC_VER
#ifndef S_IRUSR
#define S_IRUSR 0x0100		/* _S_IREAD */
#define S_IWUSR 0x0080		/* _S_IWRITE */
#define S_IRGRP 0
#define S_IWGRP 0
#endif
#endif

/* The build packs structures on byte boundaries (/Zp1), which the Windows
   headers reject (their C_ASSERT layout checks fail). Wrap their
   includes in these to use the default packing:
       COMPAT_PACK_DEFAULT_BEGIN
       #include <windows.h>
       COMPAT_PACK_DEFAULT_END
*/
#ifdef _MSC_VER
#define COMPAT_PACK_DEFAULT_BEGIN	__pragma(pack(push, 8))
#define COMPAT_PACK_DEFAULT_END		__pragma(pack(pop))
#else
#define COMPAT_PACK_DEFAULT_BEGIN
#define COMPAT_PACK_DEFAULT_END
#endif

/* Watcom made the DOS services in dos.h visible through conio.h and other
   headers as well; make them available everywhere. */
#include "dos.h"

/* Watcom runtime functions without a C runtime equivalent; implemented in
   src/compat/watcom.c. */
size_t stackavail(void);		/* bytes of stack left (malloc.h) */

#endif

#endif
