#include <stdint.h>

#ifndef HEADER_ZDLIBC_BLACKLIST_H
#define HEADER_ZDLIBC_BLACKLIST_H

int blacklist_init(char*);
int blacklist_is_blacklisted(uint32_t s_addr);
void blacklist_add(char *ip, int prefix_len);

int whitelist_init(char*);
int whitelist_is_whitelisted(uint32_t s_addr);
void whitelist_add(char *ip, int prefix_len);


#endif
