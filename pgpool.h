#include <postgresql/libpq-fe.h>

#ifndef HEADER_ZDLIBC_PGPOOL_H
#define HEADER_ZDLIBC_PGPOOL_H

int pgpool_init(int, char*);

void pgpool_close(void);

int pgpool_prepare(const char*, const char*, int, const Oid*);

PGconn * pgpool_getfreeconn(void);

PGconn *pgpool_getanyconn(void); 

#endif //HEADER_ZDLIBC_PGPOOL_H
