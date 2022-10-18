#include "dns_query.h"
#include "dns_header.h"
#include "error.h"
#include "data_queue.h"
#include "macros.h"
#include "dns_sender_events.h"
#include "base64.h"

#include <errno.h>      // errno
#include <string.h>     // memmove, memset, strlen, strerror
#include <unistd.h>     // getpid
#include <arpa/inet.h>  // ntohs

#define SEND_IPV4_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q) \
    do { \
        memset(buffer, 0, CHUNK_SIZE_IPV4); \
        packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV4); \
        \
        send(socket_fd, buffer, packet_size, 0); \
        packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV4, 0); \
    } while (0)

#define SEND_IPV6_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q) \
    do { \
        memset(buffer, 0, CHUNK_SIZE_IPV6); \
        packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV6); \
        \
        send(socket_fd, buffer, packet_size, 0); \
        packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV6, 0); \
    } while (0)

/**
 * @brief Fill buffer with DNS header data.
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

/**
 * @brief Assemble DNS query part of payload.
 *
 * @param qname_buffer Buffer that contains qname (ends with terminating character).
 * @param qname_size   Qname buffer size.
 * @param buffer_out   Destination buffer.
 * @param out_size     Destination buffer size.
 * @return int         Size of whole UDP packet.
 */
static int assemble_query(uint8_t *qname_buffer, size_t qname_size,
        uint8_t *buffer_out, size_t out_size) {
    if (qname_buffer == NULL || buffer_out == NULL)
        return 0;

    // set DNS query header
    uint8_t *query = fill_header(buffer_out, out_size);
    // move query name content into following section of a buffer
    memmove(query, qname_buffer, qname_size);

    // set last four bits of data
    uint16_t *qtype = (uint16_t *)(query + qname_size + 1);
    uint16_t *qclass = (uint16_t *)(qtype + 1);
    *qtype = ntohs(DNS_TYPE);
    *qclass = ntohs(1);

    // return size of packet
    return sizeof(struct dns_header) + qname_size + 2 * sizeof(uint16_t) + 1;
}

/**
 * @brief Create DNS query domain name.
 * Data are encoded with base64 encoding and split into (max) 63 bytes chunk of
 * data to append into final domain name.
 *
 * @param buffer      Destination buffer.
 * @param buffer_size Destination buffer size.
 * @param args        Program's arguments instance.
 * @param q           Data queue instance.
 * @return int        Output buffer length.
 */
static int create_query_domain_name(uint8_t *buffer, size_t buffer_size,
        struct args_t *args, struct data_queue_t *q) {
    if (buffer == NULL || args == NULL || q == NULL)
        return 0;

    size_t len, total = 0;
    // TODO: add label for file name - ?? only once ??
    size_t available_size = buffer_size - strlen(args->base_host) - 2;
    // reset buffer
    memset(buffer, 0, buffer_size);
    uint8_t label[LABEL_SIZE] = { 0, };
    // secondary buffer for pointer arithmetics
    uint8_t *buffer_ptr = buffer+1;
    // static int chunkID = 1;

    // cycle to fill labels
    while ((len = get_encoded_data_from_file(q, label,
                    MIN(LABEL_SIZE,available_size - total))) == LABEL_SIZE) {
        // move data from label into destination buffer
        memmove(buffer_ptr + total, label, len);
        total += len;
        q->raw_encoded_len += len;
        q->encoded_chunk += len;
        // append separator
        buffer_ptr[total++] = '.';
    }
    // append last label
    memmove(buffer_ptr + total, label, len);
    total += len;
    q->raw_encoded_len += len;
    q->encoded_chunk += len;

    // add separator and append domain
    buffer_ptr[total] = '.';
    memmove(buffer + total + 2, args->base_host, strlen(args->base_host));
    total+=strlen(args->base_host);
    buffer_ptr[total+1] = '\0';
    dns_sender__on_chunk_encoded((char *)args->dst_filepath, q->encoded_chunk, (char *)buffer_ptr);

    // convert to format
    return convert_qname_to_format(buffer) ;
}

static int create_init_query_domain(uint8_t *buffer, size_t buffer_size,
        struct args_t *args, struct data_queue_t *q) {
    if (buffer == NULL || args == NULL || q == NULL)
        return 0;
    char encoded_file_name[256] = { 0, };
    size_t len = Base64encode(encoded_file_name, args->dst_filepath, strlen(args->dst_filepath));

    // check if encoded file name fits into one dns query
    if (len > QNAME_SIZE - 2 - strlen(args->base_host) - 4)
        return -1;

    // clear buffer
    memset(buffer, 0, buffer_size);

    // divide encoded file name into labels
    uint8_t *ptr = buffer + 1;
    do {
        uint8_t size = MIN(len, LABEL_SIZE);
        memmove(ptr, encoded_file_name, size);
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
    memmove(++ptr, args->base_host, strlen(args->base_host));

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
    if (len < 0) {
        // TODO file name too big
        return -1;
    }
    SEND_IPV4_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
    dns_sender__on_transfer_init(&dst_addr->sin_addr);

    // cycle throught whole file and encode data into query and
    // send in to DNS server on the other side of the tunnel
    while ((len = create_query_domain_name((uint8_t *)qname_buffer, QNAME_SIZE, args, q)) == QNAME_SIZE) {
        // TODO: chunkID
        SEND_IPV4_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
        dns_sender__on_chunk_sent(&dst_addr->sin_addr, (char *)args->dst_filepath,
                                  q->encoded_chunk, q->raw_encoded_len);
        q->raw_encoded_len = 0;
        // memset(buffer, 0, CHUNK_SIZE_IPV4);
        // packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV4);

        // // TODO: check if send was successful
        // send(socket_fd, buffer, packet_size, 0);
        // dns_sender__on_chunk_sent(&dst_addr->sin_addr, (char *)args->dst_filepath,
        //                           q->file_size, q->raw_encoded_len);
        // packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV4, 0);
        // q->raw_encoded_len = 0;
    }

    // encode and send last chunk of data
    SEND_IPV4_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
    dns_sender__on_chunk_sent(&dst_addr->sin_addr, (char *)args->dst_filepath,
                              q->encoded_chunk, q->raw_encoded_len);
    q->raw_encoded_len = 0;
    // memset(buffer, 0, CHUNK_SIZE_IPV4);
    // packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV4);

    // send(socket_fd, buffer, packet_size, 0);
    // dns_sender__on_chunk_sent(&dst_addr->sin_addr, (char *)args->dst_filepath,
    //                           q->file_size, q->raw_encoded_len);
    // packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV4, 0);
    // q->raw_encoded_len = 0;

    dns_sender__on_transfer_completed((char *)args->dst_filepath, q->file_size);

    destroy_queue(q);
    return 0;
}

int send_data_ipv6(int socket_fd, struct sockaddr_in6 *dst_addr, FILE *f, struct args_t *args) { 
    if (f == NULL || args == NULL)
        return ERR_OTHER;

    // uint8_t buffer[CHUNK_SIZE_IPV6] = { 0, };
    // // TODO:
    // return 0;
    if (f == NULL || args == NULL)
        return ERR_OTHER;

    int len, packet_size;
    struct data_queue_t *q = init_queue(f, args);

    // packet buffer
    uint8_t buffer[CHUNK_SIZE_IPV6] = { 0, };
    // buffer for DNS qname value
    char qname_buffer[QNAME_SIZE] = { 0, };
    len = create_init_query_domain((uint8_t *)qname_buffer, QNAME_SIZE, args, q);
    if (len < 0) {
        // TODO file name too big
        return -1;
    }
    SEND_IPV6_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
    // dns_sender__on_transfer_init(&dst_addr->sin_addr);
    dns_sender__on_transfer_init6(&dst_addr->sin6_addr);

    // cycle throught whole file and encode data into query and
    // send in to DNS server on the other side of the tunnel
    while ((len = create_query_domain_name((uint8_t *)qname_buffer, QNAME_SIZE, args, q)) == QNAME_SIZE) {
        // TODO: chunkID
        SEND_IPV6_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
        // memset(buffer, 0, CHUNK_SIZE_IPV4);
        // packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV4);

        // // TODO: check if send was successful
        // send(socket_fd, buffer, packet_size, 0);
        dns_sender__on_chunk_sent6(&dst_addr->sin6_addr, (char *)args->dst_filepath,
                                  q->file_size, q->raw_encoded_len);
        // packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV4, 0);
        q->raw_encoded_len = 0;
    }

    // encode and send last chunk of data
    SEND_IPV6_PACKET(buffer, qname_buffer, len, socket_fd, dst_addr, args, q);
    // memset(buffer, 0, CHUNK_SIZE_IPV4);
    // packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV4);

    // send(socket_fd, buffer, packet_size, 0);
    dns_sender__on_chunk_sent6(&dst_addr->sin6_addr, (char *)args->dst_filepath,
                              q->file_size, q->raw_encoded_len);
    // packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV4, 0);
    q->raw_encoded_len = 0;

    dns_sender__on_transfer_completed((char *)args->dst_filepath, q->file_size);

    destroy_queue(q);
    return 0;
}


/* dns_query.c */
