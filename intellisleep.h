#include <time.h>
#include <unistd.h>

#ifndef INTELLISLEEP_H
#define INTELLISLEEP_H

typedef useconds_t isleep_t;

void intellisleep(isleep_t*);

void intellisleep_init(isleep_t*);

void intellisleep_reset(isleep_t*);

#endif//INTELLISLEEP_H
