#include "random.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define RANDSRC "/dev/urandom"

int random_bytes(void *dst, size_t n)
{
	FILE *f = fopen(RANDSRC, "rb");
	assert(f);
	size_t r = fread(dst, n, 1, f);
	fclose(f);
	if (r < 1) {
		return 0;
	}
	return 1;
}
