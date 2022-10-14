#ifndef _DNS_QUERY_H_
#define _DNS_QUERY_H_
#include <stdio.h>
#include "arguments.h"

// #define CHUNK_SIZE_IPV4 508
// #define CHUNK_SIZE_IPV6 1212

/**
 * @brief Send file data to DNS server using IPv4.
 *
 * @param socket_fd Socket file descriptor.
 * @param f         Opened file.
 * @param args      Program's arguments instance.
 * @return int      Error code when something unexpected happens; NO_ERR otherwise.
 */
int send_data_ipv4(int socket_fd, FILE *f, struct args_t *args);

/**
 * @brief Send file data to DNS server using IPv6.
 *
 * @param socket_fd Socket file descriptor.
 * @param f         Opened file.
 * @param args      Program's arguments instance.
 * @return int      Error code when something unexpected happens; NO_ERR otherwise.
 */
int send_data_ipv6(int socket_fd, FILE *f, struct args_t *args);

#endif // _DNS_QUERY_H_
