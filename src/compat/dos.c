/*
 * dos.c - implementations of the DOS services declared in
 * include/compat/dos.h.
 *
 * Hardware access (interrupts, port I/O, the PC speaker) is stubbed out:
 * those calls report failure or do nothing, except the VGA palette ports,
 * which are emulated for the video backend. The file-system calls are
 * mapped onto the C runtime.
 */

#include <string.h>
#include <stdio.h>
#include <time.h>

// The system headers come first: windows.h declares _disable() and
// _enable(), which dos.h redefines as no-op macros.
#ifdef _WIN32
#undef _disable			// defined by the force-included compat.h
#undef _enable
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>
#endif

#include "dos.h"

int int386(int intno, union REGS *in, union REGS *out)
{
	(void)intno;
	if (out != in)
		*out = *in;
	out->x.cflag = 1;			// carry set: call failed
	return out->x.eax;
}

int int386x(int intno, union REGS *in, union REGS *out, struct SREGS *seg)
{
	(void)seg;
	return int386(intno, in, out);
}

void segread(struct SREGS *seg)
{
	memset(seg, 0, sizeof(*seg));
}

unsigned char compat_vga_dac[768];
volatile unsigned compat_vga_dac_version;

static unsigned dac_read_index, dac_write_index;	// in components (color*3)
static int retrace;

int compat_inp(unsigned short port)
{
	switch (port) {
	case 0x3c9: {
		int value = compat_vga_dac[dac_read_index];
		dac_read_index = (dac_read_index + 1) % 768;
		return value;
	}
	case 0x3da:			// input status: bit 3 = vertical retrace, bit 0 = display disabled
		retrace = !retrace;
		return retrace ? 0x09 : 0x00;
	}
	return 0;
}

unsigned short compat_inpw(unsigned short port)
{
	return (unsigned short)compat_inp(port);
}

int compat_outp(unsigned short port, int value)
{
	switch (port) {
	case 0x3c7:			// DAC read index
		dac_read_index = (value & 0xff) * 3;
		break;
	case 0x3c8:			// DAC write index
		dac_write_index = (value & 0xff) * 3;
		break;
	case 0x3c9:			// DAC data
		compat_vga_dac[dac_write_index] = (unsigned char)(value & 0x3f);
		dac_write_index = (dac_write_index + 1) % 768;
		compat_vga_dac_version++;
		break;
	}
	return value;
}

unsigned short compat_outpw(unsigned short port, unsigned short value)
{
	compat_outp(port, value & 0xff);
	return value;
}

compat_isr _dos_getvect(unsigned intno)
{
	(void)intno;
	return NULL;
}

void _dos_setvect(unsigned intno, compat_isr handler)
{
	(void)intno;
	(void)handler;
}

void _chain_intr(compat_isr handler)
{
	(void)handler;
}

void delay(unsigned milliseconds)
{
#ifdef _WIN32
	Sleep(milliseconds);
#else
	(void)milliseconds;
#endif
}

void sound(unsigned frequency)
{
	(void)frequency;
}

void nosound(void)
{
}

void _harderr(int (*handler)(unsigned deverr, unsigned errcode, unsigned *devhdr))
{
	(void)handler;
}

void _hardresume(int result)
{
	(void)result;
}

void _hardretn(int error)
{
	(void)error;
}

#ifdef _WIN32

static void fill_find(struct find_t *buffer, const struct _finddata_t *fd)
{
	struct tm *t = localtime(&fd->time_write);

	buffer->attrib = (char)fd->attrib;
	buffer->size = (unsigned long)fd->size;
	strncpy(buffer->name, fd->name, sizeof(buffer->name) - 1);
	buffer->name[sizeof(buffer->name) - 1] = 0;
	if (t) {
		buffer->wr_time = (unsigned short)((t->tm_hour << 11) | (t->tm_min << 5) | (t->tm_sec / 2));
		buffer->wr_date = (unsigned short)(((t->tm_year - 80) << 9) | ((t->tm_mon + 1) << 5) | t->tm_mday);
	} else {
		buffer->wr_time = buffer->wr_date = 0;
	}
}

// DOS only returns directories, hidden and system files when asked for them
static int attrib_wanted(unsigned attributes, unsigned attrib)
{
	return (attrib & (_A_SUBDIR | _A_HIDDEN | _A_SYSTEM) & ~attributes) == 0;
}

unsigned _dos_findfirst(const char *path, unsigned attributes, struct find_t *buffer)
{
	struct _finddata_t fd;
	intptr_t h = _findfirst(path, &fd);

	buffer->handle = NULL;
	if (h == -1)
		return 1;

	while (!attrib_wanted(attributes, fd.attrib)) {
		if (_findnext(h, &fd) != 0) {
			_findclose(h);
			return 1;
		}
	}

	buffer->handle = (void *)h;
	buffer->reserved[0] = (char)attributes;
	fill_find(buffer, &fd);
	return 0;
}

unsigned _dos_findnext(struct find_t *buffer)
{
	struct _finddata_t fd;
	intptr_t h = (intptr_t)buffer->handle;
	unsigned attributes = (unsigned char)buffer->reserved[0];

	if (!buffer->handle)
		return 1;

	do {
		if (_findnext(h, &fd) != 0) {
			_findclose(h);
			buffer->handle = NULL;
			return 1;
		}
	} while (!attrib_wanted(attributes, fd.attrib));

	fill_find(buffer, &fd);
	return 0;
}

unsigned _dos_findclose(struct find_t *buffer)
{
	if (buffer->handle)
		_findclose((intptr_t)buffer->handle);
	buffer->handle = NULL;
	return 0;
}

void _dos_getdrive(unsigned *drive)
{
	*drive = (unsigned)_getdrive();
}

void _dos_setdrive(unsigned drive, unsigned *total)
{
	_chdrive((int)drive);
	if (total)
		*total = 26;
}

unsigned _dos_getdiskfree(unsigned drive, struct diskfree_t *diskspace)
{
	char root[4] = "A:\\";
	DWORD spc, bps, free_clusters, total_clusters;

	root[0] = (char)('A' + drive - 1);
	if (!GetDiskFreeSpaceA(drive ? root : NULL, &spc, &bps, &free_clusters, &total_clusters))
		return 1;

	diskspace->sectors_per_cluster = spc;
	diskspace->bytes_per_sector = bps;
	diskspace->avail_clusters = free_clusters;
	diskspace->total_clusters = total_clusters;
	return 0;
}

unsigned _dos_open(const char *path, unsigned mode, int *handle)
{
	*handle = _open(path, (int)mode | _O_BINARY);
	return *handle == -1 ? 1 : 0;
}

unsigned _dos_close(int handle)
{
	return _close(handle) == 0 ? 0 : 1;
}

unsigned _dos_getftime(int handle, unsigned short *date, unsigned short *time)
{
	struct _stat st;
	struct tm *t;

	if (_fstat(handle, &st) != 0 || (t = localtime(&st.st_mtime)) == NULL)
		return 1;

	*time = (unsigned short)((t->tm_hour << 11) | (t->tm_min << 5) | (t->tm_sec / 2));
	*date = (unsigned short)(((t->tm_year - 80) << 9) | ((t->tm_mon + 1) << 5) | t->tm_mday);
	return 0;
}

#else	// not _WIN32: no DOS-style file services

unsigned _dos_findfirst(const char *path, unsigned attributes, struct find_t *buffer)
{
	(void)path; (void)attributes;
	buffer->handle = NULL;
	return 1;
}

unsigned _dos_findnext(struct find_t *buffer)
{
	(void)buffer;
	return 1;
}

unsigned _dos_findclose(struct find_t *buffer)
{
	(void)buffer;
	return 0;
}

void _dos_getdrive(unsigned *drive)
{
	*drive = 3;
}

void _dos_setdrive(unsigned drive, unsigned *total)
{
	(void)drive;
	if (total)
		*total = 26;
}

unsigned _dos_getdiskfree(unsigned drive, struct diskfree_t *diskspace)
{
	(void)drive;
	memset(diskspace, 0, sizeof(*diskspace));
	return 1;
}

unsigned _dos_open(const char *path, unsigned mode, int *handle)
{
	(void)path; (void)mode;
	*handle = -1;
	return 1;
}

unsigned _dos_close(int handle)
{
	(void)handle;
	return 1;
}

unsigned _dos_getftime(int handle, unsigned short *date, unsigned short *time)
{
	(void)handle;
	*date = *time = 0;
	return 1;
}

#endif
