/*
 * dos.h - stand-in for the Watcom/DOS header of the same name.
 *
 * Register-level DOS and BIOS services (int386, port I/O, interrupt
 * vectors) have no equivalent outside DOS; they are declared here so the
 * code compiles and implemented in src/compat as harmless stubs that
 * report failure. The file-system calls (_dos_findfirst and friends) are
 * implemented on top of the C runtime and work.
 */

#ifndef _COMPAT_DOS_H
#define _COMPAT_DOS_H

#include <stddef.h>

/* The C runtime's conio.h declares inp(), outp() and friends. Pull it in
   before the macros below rename them, so a later #include <conio.h> in
   the game sources is a no-op instead of a garbled declaration. */
#ifdef _WIN32
#include <conio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Register sets as laid out by Watcom C/386 */
struct DWORDREGS {
	unsigned int eax, ebx, ecx, edx, esi, edi, cflag;
};

struct WORDREGS {
	unsigned short ax, _1, bx, _2, cx, _3, dx, _4, si, _5, di, _6;
	unsigned int cflag;
};

struct BYTEREGS {
	unsigned char al, ah; unsigned short _1;
	unsigned char bl, bh; unsigned short _2;
	unsigned char cl, ch; unsigned short _3;
	unsigned char dl, dh; unsigned short _4;
};

union REGS {
	struct DWORDREGS x;
	struct DWORDREGS e;
	struct WORDREGS w;
	struct BYTEREGS h;
};

struct SREGS {
	unsigned short es, cs, ss, ds, fs, gs;
};

int int386(int intno, union REGS *in, union REGS *out);
int int386x(int intno, union REGS *in, union REGS *out, struct SREGS *seg);
void segread(struct SREGS *seg);

/* Port I/O */
int compat_inp(unsigned short port);
unsigned short compat_inpw(unsigned short port);
int compat_outp(unsigned short port, int value);
unsigned short compat_outpw(unsigned short port, unsigned short value);
#define inp(p)		compat_inp(p)
#define inpw(p)		compat_inpw(p)
#define outp(p,v)	compat_outp((p),(v))
#define outpw(p,v)	compat_outpw((p),(v))

#include "vga.h"		/* emulated VGA palette ports */

/* Interrupt flag and vectors */
#define _disable()	((void)0)
#define _enable()	((void)0)
typedef void (*compat_isr)(void);
compat_isr _dos_getvect(unsigned intno);
void _dos_setvect(unsigned intno, compat_isr handler);
void _chain_intr(compat_isr handler);

/* PC speaker and timing */
void delay(unsigned milliseconds);
void sound(unsigned frequency);
void nosound(void);

/* Critical error handler (abort/retry/fail) */
#define _HARDERR_IGNORE	0
#define _HARDERR_RETRY	1
#define _HARDERR_ABORT	2
#define _HARDERR_FAIL	3
void _harderr(int (*handler)(unsigned deverr, unsigned errcode, unsigned *devhdr));
void _hardresume(int result);
void _hardretn(int error);

/* Far pointer helpers */
#define FP_SEG(p)	((unsigned short)0)
#define FP_OFF(p)	((unsigned)(size_t)(p))
#define MK_FP(s,o)	((void *)(size_t)(o))

/* File attributes */
#define _A_NORMAL	0x00
#define _A_RDONLY	0x01
#define _A_HIDDEN	0x02
#define _A_SYSTEM	0x04
#define _A_VOLID	0x08
#define _A_SUBDIR	0x10
#define _A_ARCH		0x20

struct find_t {
	char reserved[21];
	char attrib;
	unsigned short wr_time;
	unsigned short wr_date;
	unsigned long size;
	char name[260];
	void *handle;		/* search state for the C runtime */
};

unsigned _dos_findfirst(const char *path, unsigned attributes, struct find_t *buffer);
unsigned _dos_findnext(struct find_t *buffer);
unsigned _dos_findclose(struct find_t *buffer);

void _dos_getdrive(unsigned *drive);
void _dos_setdrive(unsigned drive, unsigned *total);

#ifdef _WIN32
#include <direct.h>			/* struct _diskfree_t */
#ifndef diskfree_t
#define diskfree_t _diskfree_t
#endif
#else
struct diskfree_t {
	unsigned total_clusters;
	unsigned avail_clusters;
	unsigned sectors_per_cluster;
	unsigned bytes_per_sector;
};
#endif
unsigned _dos_getdiskfree(unsigned drive, struct diskfree_t *diskspace);

unsigned _dos_open(const char *path, unsigned mode, int *handle);
unsigned _dos_close(int handle);
unsigned _dos_getftime(int handle, unsigned short *date, unsigned short *time);

#ifdef __cplusplus
}
#endif

#endif
