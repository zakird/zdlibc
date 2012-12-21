#include <postgresql/libpq-fe.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef HEADER_ZDLIBC_DATABASE_H

int pg_exec_cmd(PGconn *conn, char *cmd);
int pg_async_exec(PGconn *conn, const char *command);
int pg_check_tuples(PGresult *res);
int pg_escapestr(char *dest, char *source, size_t len, PGconn *conn);
int pg_bulkloadcsv(PGconn *conn, const char *tabledef, char *buf);
int pg_droptable(PGconn *conn, const char *table);
int pg_set_work_mem(PGconn *conn, char *size);

#endif