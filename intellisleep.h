#include <time.h>
#include <unistd.h>

#ifndef HEADER_ZDLIBC_INTELLISLEEP_H
#define HEADER_ZDLIBC_INTELLISLEEP_H

typedef useconds_t isleep_t;

void intellisleep(isleep_t*);

void intellisleep_init(isleep_t*);

void intellisleep_reset(isleep_t*);

#endif//HEADER_ZDLIBC_INTELLISLEEP_H
