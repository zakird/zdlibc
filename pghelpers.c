#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <postgresql/libpq-fe.h>

// execute a command (no result set returned) synchronously
int pg_exec_cmd(PGconn *conn, char *cmd)
{
	PGresult *res = PQexec(conn, cmd);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		printf("ERROR: unable to execute command: %s. %s.\n",
				cmd, PQerrorMessage(conn));
		return -1;
	}
	PQclear(res);
	return 0;
}

int pg_async_exec(PGconn *conn, const char *command)
{
	int rc = PQsendQuery(conn, command);
	if (rc == 0) {
		fprintf(stderr, "ERROR: async postgres call failed. %s\n", 
				PQerrorMessage(conn));
		return -1;
	}
	return 0;
}

int pg_check_tuples(PGresult *res)
{
	if (PQresultStatus(res) != PGRES_TUPLES_OK) { 
		fprintf(stderr, "ERROR: postgres call failed. %s\n", 
				PQresultErrorMessage(res)); 
		PQclear(res);
		return -1;
	}
	return 0;
}

/* escapes a strong to make it safe to insert into a sql statement */
int pg_escapestr(char *dest, char *source, size_t len, PGconn *conn)
{
	int err;
	PQescapeStringConn(conn, dest, source, len, &err);
	if (err) {
		fprintf(stderr, "ERROR: an error occured in "
				"PQescapeStringConn. %s\n",
				PQerrorMessage(conn));
		return -1;	
	}
	return 0;
}

int pg_bulkloadcsv(PGconn *conn, const char *tabledef, char *buf)
{
	assert(init);
	PGresult *res;
	int rc;
	// enter bulk data mode
	char cmd[strlen(tabledef) + 100];
	sprintf(cmd, "COPY %s FROM STDIN WITH CSV", tabledef);
	res = PQexec(conn, cmd);
	if (PQresultStatus(res) != PGRES_COPY_IN) {
		fprintf(stderr, "ERROR: begin copy mode failed. %s\n", 
				PQerrorMessage(conn));
		return -1;
	}
	PQclear(res);
	rc = PQputCopyData(conn, (const char *) buf, strlen(buf));
	if (rc == 0) {
		fprintf(stderr, "ERROR: unable to send data:"
				" attempt would block\n");
		return -1;
	} else if (rc == -1) {
		fprintf(stderr, "ERROR: unable to send data:"
				" error occured\n");
		return -1;
	}
	rc = PQputCopyEnd(conn, (const char*) NULL);
	if (rc == -1) {
		fprintf(stderr, "ERROR: PQputCopyEnd failed. %s\n", 
				PQerrorMessage(conn));
		return -1; 
	} else if (rc == 0) {
		fprintf(stderr, "ERROR: attempt would block.\n");
		return -1;
	}
	res = PQgetResult(conn);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "ERROR: post copy result failed. %s\n", 
				PQerrorMessage(conn));
		return -1;
	}
	return 0;
}

int pg_droptable(PGconn *conn, const char *table)
{
	assert(init);
	char cmd[strlen(table) + 50];
	sprintf(cmd, "DROP TABLE IF EXISTS \"%s\";", table);
	return pg_exec_cmd(conn, cmd);
}

int pg_set_work_mem(PGconn *conn, char *size)
{
	char cmd[100];
	sprintf(cmd, "set work_mem='%s'", size);
	return pg_exec_cmd(conn, cmd);
}
