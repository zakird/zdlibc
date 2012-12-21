#include "intellisleep.h"

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define INTELLISLEEP_MAX 1000000
#define INTELLISLEEP_MIN 50
#define INTELLISLEEP_GROWTHFACTOR 2

void intellisleep_init(isleep_t *s)
{
	*s = INTELLISLEEP_MIN;
}

void intellisleep(isleep_t *s)
{
	if (!(*s)) {
		*s = INTELLISLEEP_MIN;
	} else if (*s < INTELLISLEEP_MAX) {
		isleep_t t = *s * INTELLISLEEP_GROWTHFACTOR;
		if (*s > INTELLISLEEP_MAX)
			*s = INTELLISLEEP_MAX;
		else
			*s = t;
	} else if (*s > INTELLISLEEP_MAX) {
		*s = INTELLISLEEP_MAX;
	}
	usleep(*s);
}


void intellisleep_reset(isleep_t *s)
{
	*s = INTELLISLEEP_MIN;
}
