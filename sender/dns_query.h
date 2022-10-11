#ifndef _DNS_QUERY_H_
#define _DNS_QUERY_H_
#include <stdio.h>
#include "arguments.h"

#define CHUNK_SIZE_IPV4 508
#define CHUNK_SIZE_IPV6 1212

int send_data_ipv4(int socket_fd, FILE *f, struct args_t *args);
int send_data_ipv6(int socket_fd, FILE *f, struct args_t *args);

#endif // _DNS_QUERY_H_
