#ifndef _DNS_RESPONSE_H_
#define _DNS_RESPONSE_H_

#include <arpa/inet.h>
#include <stdio.h>

#include "arguments.h"

/**
 * @brief Use of Google's public DNS servers to response to query.
 * Received query will be redirected to Google's public DNS servers and
 * its response will be redirected back to client.
 *
 * @param socket_fd   Socket's file descriptor.
 * @param packet      Original packet with DNS query.
 * @param packet_size Packet's size.
 * @param buf_size    Packet buffer's max size.
 * @param client      Info of client's address.
 * @param c_size      Client's sockaddr structure size.
 * @return int        NO_ERR when no error occurs; non-zero value otherwise.
 */
int normal_dns_response(int socket_fd, uint8_t *packet, size_t packet_size,
        size_t buf_size, struct sockaddr *client, socklen_t c_size);

/**
 * @brief Barebone function that handles whole communication between client and server.
 *
 * @param socket_fd Socket's file descriptor.
 * @param args      Program's arguments.
 * @return int      NO_ERR value when no error occurs; non-zero value otherwise.
 */
int run_communication(int socket_fd, struct args_t *args);

#endif // _DNS_RESPONSE_H_
