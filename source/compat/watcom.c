/*
 * watcom.c - replacements for Watcom C runtime functions that have no
 * equivalent in other C runtimes. Declared in include/compat/compat.h.
 */

#include <stddef.h>

/* Returns the number of bytes of stack space left. The game only uses it
   to guard against runaway recursion with a 1K threshold; the stack is
   no longer a small fixed DOS/4GW stack, so report plenty. */
size_t stackavail(void)
{
	return 64*1024;
}
