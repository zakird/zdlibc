#include "blacklist.h"

#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <patricia.h>
#include <logger.h>

static patricia_tree_t *blacklist = NULL;
static patricia_tree_t *whitelist = NULL;

static prefix_t check;

static prefix_t *make_prefix(unsigned long addr, int width)
{
	prefix_t *sn = malloc(sizeof(prefix_t));
	sn->add.sin.s_addr = addr;
	sn->family = AF_INET;
	sn->bitlen = width;
	sn->ref_count = 1;
	return sn;
}

static int is_blacklisted(patricia_tree_t* tree, uint32_t s_addr, int default_)
{
	if (tree) {
		check.add.sin.s_addr = s_addr;
		if (patricia_search_best(tree, &check)) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return default_;
	}
}

// check whether a single IP address is blacklisted.
//		1 => is blacklisted
//		0 => is not blacklisted
int blacklist_is_blacklisted(uint32_t s_addr) {
	return is_blacklisted(blacklist, s_addr, 0);
}

int whitelist_is_whitelisted(uint32_t s_addr) {
	int retval = is_blacklisted(whitelist, s_addr, 1);
	if (retval) {
	}
	return retval;
}

// add a CIDR network allocation to blacklist
// e.g. blacklist_add("128.255.134.0", 24)
static void add(patricia_tree_t *tree, char *ip, int prefix_len)
{
	assert(ip);
	assert((0 <= prefix_len) && (prefix_len <= 32));

	assert(tree);
	prefix_t *subnet = make_prefix(inet_addr(ip), prefix_len);
	patricia_lookup(tree, subnet);
	Deref_Prefix(subnet);
}


void blacklist_add(char *ip, int prefix_len)
{
	add(blacklist, ip, prefix_len);
}

void whitelist_add(char *ip, int prefix_len)
{
	add(whitelist, ip, prefix_len);
}

static int init(patricia_tree_t **t, char *file, const char *name)
{
	patricia_tree_t* tree = New_Patricia(32);
	*t = tree;
	memset(&check, 0, sizeof(prefix_t));
	check.family = AF_INET;
	check.bitlen = 32;
	check.ref_count = 1;

	FILE *fp;
	char line[1000];
	int blocked = 0;

	fp = fopen(file, "r");
	if (fp == NULL) {
		log_fatal(name, "Unable to open %s file: %s: %s", 
				name, file, strerror(errno));
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		char *comment = strchr(line, '#');
		if (comment) {
			*comment = '\0';
		}
		char ip[33];
		if ((sscanf(line, "%32s", ip)) == EOF) {
			continue;
		}
		int prefix_len;
		log_info(name, "'%s'", ip);
		char *slash = strchr(ip, '/');
		if (slash == NULL) {
			log_fatal(name,
				"Unable to parse %s file: %s",
				name, file);
		}
		// split apart network and prefix length 
		*slash = '\0';
		prefix_len = atoi(&slash[1]);
		add(tree, ip, prefix_len);
		log_debug(name, "%s/%i added to %s",
				ip, prefix_len, name);
		blocked++;
	}
	fclose(fp);
	return 0;

}

int blacklist_init(char *file)
{
	return init(&blacklist, file, "blacklist");
}

int whitelist_init(char *file)
{
	return init(&whitelist, file, "whitelist");
}
