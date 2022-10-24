/**
 * @brief This file declares functions used as DNS server for DNS communication.
 * Server is capable of answering basic DNS queries by rerouting query to
 * Google's public DNS server (8.8.8.8).
 *
 * Server uses UDP protocol for communication with client. Tunneling protocol is
 * defined in documentation. For data encoding and decoding is used base64 encoding.
 * Source code for base64 implementation are taken from https://www.opensource.apple.com
 * under the license Apple Public Source License Version 2.0.
 *
 * This source code serves as submission for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file dns_response.h
 * @date 2022-10-23
 */
#ifndef _DNS_RESPONSE_H_
#define _DNS_RESPONSE_H_

#include <netinet/in.h>  // struct sockaddr

#include "arguments.h"
#include "recv_queue.h"

#define MAX_QUERY_SIZE (sizeof(struct dns_header) + 255 + 2 * sizeof(uint16_t))

/**
 * @brief Validates whether packet passes initialization requirements.
 *
 * @param packet Buffer containing packet's data.
 * @param packet_size Buffer's size.
 * @param base_host Base host domain name.
 * @return int   Non-zero value when packet passes validation check.
 */
int validate_tunnel_communication(const void *packet, size_t packet_size, const char *base_host);

/**
 * @brief Extract and decode destination file name from initialization DNS query.
 *
 * @param qname_buffer Query domain name.
 * @param file_name    Buffer to store file name.
 * @param args         Program's arguments.
 * @return int         File name's length; negative value when error occurs
 */
int get_file_name(uint8_t *qname_buffer, uint8_t *file_name, const struct args_t *args);

/**
 * @brief Transfor query packet to response packet.
 *
 * @param packet Buffer containing packet data.
 * @return int Zero when no error occurs.
 */
int create_response(uint8_t *packet);

/**
 * @brief Processes initialization packet.
 * Function checks whether received packet is initialization packet.
 * If yes, function initializes given `data_queue_t` instance, extract initialization
 * data from packet, logs starting communication and update socket settings SOL_RCVTIMEO.
 *
 * Function can return 3 types of value:
 *  - <0 = error occurs
 *  - =0 = received packet was indeed initialization packet
 *  - >0 = received packet is normal DNS query; returns packet size
 *
 * @param socket_fd     Socket's file descriptor.
 * @param args          Program's arguments.
 * @param q             Pointer to data queue instance (might be initialized).
 * @param packet_buffer Buffer with packet's content.
 * @param buffer_size   Packet buffer size.
 * @param client        Info of client's address.
 * @return int          Zero when server received initialization packet;
 *                      negative value when error occurs, otherwise
 *                      packet's size when valid DNS query was received.
 */
int process_init_packet(const int socket_fd, const struct args_t *args,
        struct data_queue_t **q, uint8_t *packet_buffer,
        size_t buffer_size, struct sockaddr *client, socklen_t *len);

/**
 * @brief Extract data from dns queries.
 * Ignores DNS header part and only processes DNS query name.
 * Each label is stored in data buffer (defined in `data_queue_t` structure).
 * If program has already received same packet before, processing is halted and
 * function returns ERR_OLD_PACKET value.
 *
 * @param buffer      Whole packet buffer.
 * @param domain_name Base host domain name defined by user.
 * @param q           Data queue instance.
 * @return int        Number of bytes used for storing data;
 *                    Negative value when error occurs.
 */
int process_tunneling_data(uint8_t const *buffer, const char *domain_name, struct data_queue_t *q);

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
