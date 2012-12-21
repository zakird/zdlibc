#include <unistd.h>
#include <postgresql/libpq-fe.h>
#include "assert.h"
#include <stdlib.h>
#include <stdio.h>
#include "logger.h"

#define MAX_POOLSIZE 16

static int poolsize;

static PGconn* pool[MAX_POOLSIZE];
static int initialized = 0;
static int last_used = -1;

static int pgpool_connect(PGconn **conn, char *connstr)
{
	*conn = PQconnectdb(connstr);
	if (*conn == NULL) {
		log_fatal("pgpool","connection came back as NULL."
				" Probably out of memory.");
		return -1;
	}
	if (PQstatus(*conn) != CONNECTION_OK) {
		log_fatal("pgpool", "unable to connect to database. %s",
				PQerrorMessage(*conn));
		PQfinish(*conn);
		conn = NULL;
		return -1;
	}
	if (PQsetClientEncoding(*conn, "UTF8") < 0) {
		log_fatal("pgpool", "unable to set client"
			       " encoding to UTF8.\n");
		PQfinish(*conn);
		*conn = NULL;
		return -1;
	}
	return 0;
}

int pgpool_init(int numconns, char *connstr)
{
	if (!initialized) {
		poolsize = numconns;
		for (int i=0; i < poolsize; i++) {
			int rc = pgpool_connect(&(pool[i]), connstr);
			if (rc < 0)
				return -1;
		}
		log_info("pgpool", "initiailized with %i connections",
				poolsize);
		initialized = 1;
	}
	return 0;
}

void pgpool_close(void)
{
	for (int i=0; i< poolsize; i++) {
		if (pool[i]) {
			PQfinish(pool[i]);
		}
	}
}

int pgpool_prepare(const char *name, const char *query, 
		int num_params, const Oid *param_types)
{
	assert(initialized);
	for (int i=0; i < poolsize; i++) {
		PGresult *res = PQprepare(
				pool[i], 
				name,
				query,
				num_params, 
				param_types);
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			fprintf(stderr, "ERROR: prepare %s failed: %s", name,
					PQerrorMessage(pool[i]));
			PQclear(res);
			return -1;
		}
		PQclear(res);
	}
	return 0;
}

static int reshandler(PGconn *conn, PGresult *res)
{
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		log_error("pgpool", "async respond failed. %s", 
				PQerrorMessage(conn));
	}
	PQclear(res);
	return 0;
}

PGconn * pgpool_getfreeconn(void)
{
	assert(initialized);
	int id = last_used;
	while (1) {
		id = (id+1) % poolsize;
		PGconn *c = pool[id];
		assert(c);
		PGresult *res;
		int rc;
		// can we get state AND is the server 
		// not blocking and any results?
		while ((rc=PQconsumeInput(c)) && !PQisBusy(c) 
				&& (res=PQgetResult(c))) {
			// we have results. we need to clear those resultsets
			if (reshandler(c, res)) { 
				return NULL;
			}
		}
		if (!rc) {
			log_fatal("pgpool", "Error in PQconsumeInput. %s",
					PQerrorMessage(c));
			return NULL;
		} else if (!PQisBusy(c)) {
			last_used = id;
			return c;
		}
		if (id == last_used) {
			usleep(150000);
		}
	}
}

PGconn *pgpool_getanyconn(void) 
{
	assert(initialized);
	return pool[0];
}

