/**
 * @brief This file declares functions used as DNS client for DNS communication.
 * Program reads given file/stdin data, encodes them with base64 encoding,
 * fill DNS query labels with data and sends it to selected DNS server.
 *
 * Client uses UDP protocol for communication with server. Tunneling protocol is
 * defined in documentation. For data encoding and decoding is used base64 encoding.
 * Source code for base64 implementation are taken from https://www.opensource.apple.com
 * under the license Apple Public Source License Version 2.0.
 *
 * This source code serves as submission for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file dns_query.h
 * @date 2022-10-23
 */
#ifndef _DNS_QUERY_H_
#define _DNS_QUERY_H_
#include <stdio.h>          // FILE
#include <netinet/in.h>     // struct sockaddr_in, struct sockaddr_in6

#include "arguments.h"
#include "send_queue.h"

/**
 * @brief Assemble DNS query part of payload.
 *
 * @param qname_buffer Buffer that contains qname (ends with terminating character).
 * @param qname_size   Qname buffer size.
 * @param buffer_out   Destination buffer.
 * @param out_size     Destination buffer size.
 * @return int         Size of whole UDP packet.
 */
int assemble_query(uint8_t *qname_buffer, size_t qname_size,
                                    uint8_t *buffer_out, size_t out_size);

/**
 * @brief Create DNS query domain name.
 * Data are encoded with base64 encoding and split into (max) 63 bytes chunk of
 * data to append into final domain name.
 *
 * @param buffer       Packet buffer.
 * @param buffer_size  Packet buffer size.
 * @param args         Program arguments instance.
 * @param q            Data queue instance.
 * @return int         Output buffer length.
 */
int create_query_domain_name(uint8_t *buffer, size_t buffer_size,
                                    struct args_t *args, struct data_queue_t *q);

/**
 * @brief Create a init DNS query name.
 *
 * @param buffer       Packet buffer.
 * @param buffer_size  Packet buffer size.
 * @param args         Program arguments instance.
 * @param q            Data queue instance.
 * @return int
 */
int create_init_query_domain(uint8_t *buffer, size_t buffer_size,
                                    struct args_t *args, struct data_queue_t *q);
/**
 * @brief Send file data to DNS server using IPv4.
 *
 * @param socket_fd Socket file descriptor.
 * @param dst_addr  Structure handling internet addresses.
 * @param f         Opened file.
 * @param args      Program's arguments instance.
 * @return int      Error code when something unexpected happens; NO_ERR otherwise.
 */
int send_data_ipv4(int socket_fd, struct sockaddr_in *dst_addr, FILE *f, struct args_t *args);

/**
 * @brief Send file data to DNS server using IPv6.
 *
 * @param socket_fd Socket file descriptor.
 * @param dst_addr  Structure handling internet addresses.
 * @param f         Opened file.
 * @param args      Program's arguments instance.
 * @return int      Error code when something unexpected happens; NO_ERR otherwise.
 */
int send_data_ipv6(int socket_fd, struct sockaddr_in6 *dst_addr, FILE *f, struct args_t *args);

#endif // _DNS_QUERY_H_
