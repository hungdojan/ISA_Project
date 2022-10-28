/**
 * @brief This file defines functions declared in `dns_query.h`.
 *
 * This source code serves as submission for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file dns_query.c
 * @date 2022-10-23
 */
#include "dns_query.h"
#include "dns_header.h"
#include "error.h"
#include "macros.h"
#include "dns_sender_events.h"
#include "base64.h"

#include <errno.h>      // errno
#include <string.h>     // memmove, memset, strlen, strerror
#include <unistd.h>     // getpid
#include <arpa/inet.h>  // ntohs
#include <sys/socket.h> // send, receive

#define SEND_IPV4_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q) \
    do { \
        int send_counter = 5;               \
        memset(buffer, 0, CHUNK_SIZE_IPV4); \
        packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV4); \
        \
        do { \
            send(socket_fd, buffer, packet_size, 0);                    \
            if (errno != 0)     continue;                               \
            packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV4, 0);  \
            if (errno != 0)     continue;                               \
            errno = check_received_packet(buffer, q->chunkID - 1);      \
        } while (send_counter-- && errno != 0);   \
        if (errno != 0)             \
            goto error_packet_ipv4; \
    } while (0)

#define SEND_IPV6_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q) \
    do { \
        int send_counter = 5;               \
        memset(buffer, 0, CHUNK_SIZE_IPV6); \
        packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV6); \
        \
        do { \
            send(socket_fd, buffer, packet_size, 0);                    \
            if (errno != 0)     continue;                               \
            packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV6, 0);  \
            if (errno != 0)     continue;                               \
            errno = check_received_packet(buffer, q->chunkID - 1);      \
        } while (send_counter-- && errno != 0);   \
        if (errno != 0)             \
            goto error_packet_ipv6; \
    } while (0)

/**
 * @brief Compare sent chunkID with received one.
 *
 * @param packet        Received packet data.
 * @param sent_chunkID  Expected chunkID.
 * @return int Difference of chunkIDs. Return 0 when both sent and received IDs are the same.
 */
static int check_received_packet(void *packet, size_t sent_chunkID) {
    uint8_t *dns_qname = (uint8_t *)packet + sizeof(struct dns_header);
    char num_buffer[20] = { 0, };
    memcpy(num_buffer, dns_qname+1, dns_qname[0]);
    num_buffer[dns_qname[0]] = '\0';
    return sent_chunkID - strtol(num_buffer, NULL, 10);
}

/**
 * @brief Inserts chunkID value into DNS query name.
 *
 * @param dns_query Address of DNS query section.
 * @param q         Data queue instance.
 * @return size_t   Number of bytes added into dns_query
 */
static size_t append_chunk_id(void *dns_query, struct data_queue_t *q) {
    sprintf((char *)dns_query, "%ld.", q->chunkID++);
    size_t chunk_size = strlen((char *)dns_query);
    return chunk_size;
}

/**
 * @brief Fill buffer with DNS header data.
 * It is expected to send only one query per packet.
 *
 * @param buffer      Data buffer.
 * @param buffer_size Data buffer size.
 * @return uint8_t *  Pointer to where to start filling DNS query data.
 */
static uint8_t *fill_header(uint8_t *buffer, size_t buffer_size) {
    if (buffer == NULL ||
        buffer_size < sizeof(struct dns_header))
        return NULL;

    struct dns_header *header = (struct dns_header *)buffer;
    header->id = ntohs(getpid());
    header->param.query.qresponse = 0;
    header->param.query.opcode = 0;
    header->param.query.auth_ansr = 0;
    header->param.query.rec_desire = 1;
    header->param.query.rec_avail = 0;
    header->param.query.zeros = 0;
    header->param.query.response = 0;
    header->q_count = ntohs(1);
    header->ar_count = header->addit_count = header->ns_count = 0;
    return buffer + sizeof(struct dns_header);
}

/**
 * @brief Converts qname buffer content into valid query name format.
 * Valid query name format is for example 6google3com.
 *
 * @param buffer Output data buffer.
 * @return int   Total length of qname.
 */
static int convert_qname_to_format(uint8_t *buffer) {
    // split string into two strings with '.' as a separator
    char *token = strtok((char *)buffer+1, ".");

    // splitted string length and index of byte to store this info in
    int index = 0;
    while (token) {
        buffer[index] = (uint8_t) strlen(token);        // store length
        index += strlen(token) + 1;                     // shift index
        token = strtok(NULL, ".");                      // split string again
    }
    buffer[index] = '\0';   // end string with terminating char

    // return total qname length
    return index;
}

int assemble_query(uint8_t *qname_buffer, size_t qname_size,
        uint8_t *buffer_out, size_t out_size) {
    if (qname_buffer == NULL || buffer_out == NULL)
        return 0;

    // set DNS query header
    uint8_t *query = fill_header(buffer_out, out_size);
    // move query name content into following section of a buffer
    memcpy(query, qname_buffer, qname_size);

    // set last four bits of data
    uint16_t *qtype = (uint16_t *)(query + qname_size + 1);
    uint16_t *qclass = (uint16_t *)(qtype + 1);
    *qtype = ntohs(DNS_TYPE);
    *qclass = ntohs(1);

    // return size of packet
    return sizeof(struct dns_header) + qname_size + 2 * sizeof(uint16_t) + 1;
}

int create_query_domain_name(uint8_t *buffer, size_t buffer_size,
        struct args_t *args, struct data_queue_t *q) {
    if (buffer == NULL || args == NULL || q == NULL)
        return ERR_OTHER;

    // reset buffer
    memset(buffer, 0, buffer_size);
    static uint8_t label[LABEL_SIZE] = { 0, };
    // secondary buffer for pointer arithmetics
    uint8_t *buffer_ptr = buffer + 1;

    // append chunk ID
    size_t chunk_size = append_chunk_id(buffer_ptr, q);
    buffer_ptr += chunk_size;

    size_t len, total = 0;
    size_t available_size = buffer_size - strlen(args->base_host) - 2 - buffer[0] - chunk_size;

    // cycle to fill labels
    while ((len = get_encoded_data_from_file(q, label,
                    MIN(LABEL_SIZE,available_size - total))) == LABEL_SIZE) {
        // move data from label into destination buffer
        memcpy(buffer_ptr + total, label, len);
        total += len;
        q->raw_encoded_len += len;
        q->encoded_chunk += len;
        // append separator
        buffer_ptr[total++] = '.';
    }
    // append last label
    memcpy(buffer_ptr + total, label, len);
    total += len;
    q->raw_encoded_len += len;
    q->encoded_chunk += len;

    // add separator and append domain
    buffer_ptr[total] = '.';
    memcpy(buffer + total + 2 + chunk_size, args->base_host, strlen(args->base_host));
    total+=strlen(args->base_host);
    buffer_ptr[total+1] = '\0';
    dns_sender__on_chunk_encoded((char *)args->dst_filepath, q->chunkID-1, (char *)buffer_ptr);

    // convert to format
    return convert_qname_to_format(buffer);
}

int create_init_query_domain(uint8_t *buffer, size_t buffer_size,
        struct args_t *args, struct data_queue_t *q) {
    if (buffer == NULL || args == NULL || q == NULL)
        return ERR_OTHER;

    // clear buffer
    memset(buffer, 0, buffer_size);
    uint8_t *ptr = buffer + 1;  // starting address to insert data

    // append chunk ID
    size_t chunk_size = append_chunk_id(ptr, q);
    ptr += chunk_size;

    char encoded_file_name[256] = { 0, };
    size_t len = Base64encode(encoded_file_name, args->dst_filepath, strlen(args->dst_filepath));

    // check if encoded file name fits into one dns query
    if (len > QNAME_SIZE - 2 - strlen(args->base_host) - 4 - chunk_size)
        ERR_MSG(ERR_FNAME_TOO_BIG, "File name is too large\n");

    // divide encoded file name into labels
    do {
        uint8_t size = MIN(len, LABEL_SIZE);
        memcpy(ptr, encoded_file_name, size);
        len -= size;
        ptr += size;
        *ptr = '.';
        ptr++;
    } while (len > 0);
    // last label is shifted by 2 (terminating char, '.', ptr_addr);
    // we have to move 2 characters to the left (get rid of terminating char)
    ptr -= 2;
    *ptr = '.';
    // append domain name
    memcpy(++ptr, args->base_host, strlen(args->base_host));

    return convert_qname_to_format(buffer);
}

int send_data_ipv4(int socket_fd, struct sockaddr_in *dst_addr, FILE *f, struct args_t *args) {
    if (f == NULL || args == NULL)
        return ERR_OTHER;

    int len, packet_size;
    struct data_queue_t *q = init_queue(f, args);

    // packet buffer
    uint8_t buffer[CHUNK_SIZE_IPV4] = { 0, };
    // buffer for DNS qname value
    char qname_buffer[QNAME_SIZE] = { 0, };
    len = create_init_query_domain((uint8_t *)qname_buffer, QNAME_SIZE, args, q);
    if (len < 0)        // error checking
        goto error_packet_ipv4;

    SEND_IPV4_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
    dns_sender__on_transfer_init(&dst_addr->sin_addr);

    // cycle throught whole file and encode data into query and
    // send in to DNS server on the other side of the tunnel
    while ((len = create_query_domain_name((uint8_t *)qname_buffer, QNAME_SIZE, args, q)) == QNAME_SIZE) {
        // TODO: chunkID
        SEND_IPV4_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
        dns_sender__on_chunk_sent(&dst_addr->sin_addr, (char *)args->dst_filepath,
                                  q->chunkID - 1, q->raw_encoded_len);
        q->raw_encoded_len = 0;
    }
    if (len < 0)        // error checking
        goto error_packet_ipv4;

    // encode and send last chunk of data
    SEND_IPV4_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
    dns_sender__on_chunk_sent(&dst_addr->sin_addr, (char *)args->dst_filepath,
                              q->chunkID - 1, q->raw_encoded_len);
    q->raw_encoded_len = 0;

    // finish communication
    dns_sender__on_transfer_completed((char *)args->dst_filepath, q->file_size);

    destroy_queue(q);
    return NO_ERR;

error_packet_ipv4:
    destroy_queue(q);
    ERR_MSG(len, "Connection lost\n");
}

int send_data_ipv6(int socket_fd, struct sockaddr_in6 *dst_addr, FILE *f, struct args_t *args) {
    if (f == NULL || args == NULL)
        return ERR_OTHER;

    int len, packet_size;
    struct data_queue_t *q = init_queue(f, args);

    // creating initialization packet to start tunneling
    uint8_t buffer[CHUNK_SIZE_IPV6] = { 0, };       // packet buffer
    char qname_buffer[QNAME_SIZE] = { 0, };         // buffer for DNS qname value
    len = create_init_query_domain((uint8_t *)qname_buffer, QNAME_SIZE, args, q);
    if (len < 0)        // error checking
        goto error_packet_ipv6;

    SEND_IPV6_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
    dns_sender__on_transfer_init6(&dst_addr->sin6_addr);

    // cycle throught whole file and encode data into query and
    // send in to DNS server on the other side of the tunnel
    while ((len = create_query_domain_name((uint8_t *)qname_buffer, QNAME_SIZE, args, q)) == QNAME_SIZE) {
        SEND_IPV6_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
        dns_sender__on_chunk_sent6(&dst_addr->sin6_addr, (char *)args->dst_filepath,
                                  q->chunkID - 1, q->raw_encoded_len);
        q->raw_encoded_len = 0;
    }
    if (len < 0)        // error checking
        goto error_packet_ipv6;

    // encode and send last chunk of data
    SEND_IPV6_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
    dns_sender__on_chunk_sent6(&dst_addr->sin6_addr, (char *)args->dst_filepath,
                              q->chunkID - 1, q->raw_encoded_len);
    q->raw_encoded_len = 0;

    // finish communication
    dns_sender__on_transfer_completed((char *)args->dst_filepath, q->file_size);

    destroy_queue(q);
    return NO_ERR;

error_packet_ipv6:
    destroy_queue(q);
    ERR_MSG(len, "Connection lost\n");
}


/* dns_query.c */
