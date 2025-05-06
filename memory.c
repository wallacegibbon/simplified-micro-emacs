/*
 * This is a dirty HACK.
 * If the size is not stored in the hidden header, or it's stored in unexpected
 * address, it go wrong.
 */

#include "estruct.h"
#include "edef.h"

#if RAMSIZE

/* Undefine macros defined in estruct.h to unshadow `malloc` and `free`. */
#undef malloc
#undef free

void *malloc(unsigned long);
void free(void *);

/* This hack is doing dangerous pointer operations that compiler will warn */
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

static inline size_t allocated_size(void *p)
{
	/* The LSB is a sign, which become 0 when this memory got `free`ed */
	return *((size_t *)p - 1) & ~1;
}

#pragma GCC diagnostic ignored "-Warray-bounds"

void *allocate(unsigned long nbytes)
{
	char *mp = malloc(nbytes);
	if (mp) {
		envram += allocated_size(mp);
	}
	return mp;
}

void release(void *mp)
{
	if (mp) {
		envram -= allocated_size(mp);
		free(mp);
	}
}

#pragma GCC diagnostic pop

#endif
