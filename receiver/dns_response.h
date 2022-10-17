#ifndef _DNS_RESPONSE_H_
#define _DNS_RESPONSE_H_

#include <arpa/inet.h>
#include <stdio.h>

#include "arguments.h"

// int get_file_name(uint8_t *qname_buffer, uint8_t *file_name, struct args_t *args);
// int send_response(int socket_fd, struct sockaddr *dst, FILE *f, struct args_t *args);
int normal_dns_response(int socket_fd, uint8_t *packet, size_t packet_size,
        size_t buf_size, struct sockaddr *client, socklen_t c_size);
// int create_response();
int run_communication(int socket_fd, struct sockaddr *dst, struct args_t *args);

#endif // _DNS_RESPONSE_H_
