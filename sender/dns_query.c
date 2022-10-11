#include "dns_query.h"
#include "dns_header.h"
#include "error.h"
#include "data_queue.h"

#include <errno.h>
#include <math.h>
#include <string.h>
#include <unistd.h>     // getpid
#include <arpa/inet.h>  // ntohs
                        //
#define QNAME_SIZE 254
#define LABEL_SIZE 63
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static uint8_t *fill_header(uint8_t *buffer, size_t buffer_size) {
    if (buffer == NULL ||
        buffer_size < sizeof(struct dns_header))
        return NULL;

    struct dns_header *header = (struct dns_header *)buffer;
    header->id = ntohs(getpid());
    header->parameters.qresponse = 1;
    header->parameters.opcode = 0;
    header->parameters.auth_ansr = 0;
    header->parameters.rec_desire = 0;
    header->parameters.rec_avail = 0;
    header->parameters.zeros = 0;
    header->parameters.response = 0;
    header->q_count = ntohs(1);
    header->ar_count = header->addit_count = header->ns_count = 0;
    return buffer + sizeof(struct dns_header);
}

/**
 * Converts buffer content into valid query name format.
 */
int convert_qname_to_format(uint8_t *buffer) {
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
    memmove(query, qname_buffer, qname_size);

    // set last four bits of data
    uint16_t *qtype = (uint16_t *)(query + qname_size + 1);
    uint16_t *qclass = (uint16_t *)(qtype + 1);
    // TODO: define constants
    *qtype = ntohs(1);
    *qclass = ntohs(1);

    // return size of packet
    return sizeof(struct dns_header) + qname_size + 2 * sizeof(uint16_t) + 1;
}

int create_query_domain_name(uint8_t *buffer, size_t buffer_size,
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

    // FIXME: add encoded data
    // TODO: append file info
    // cycle to fill labels
    while ((len = get_encoded_data(q, label, MIN(LABEL_SIZE,available_size - total))) == LABEL_SIZE) {
        // move data from label into destination buffer
        memmove(buffer_ptr + total, label, len);
        total += len;
        // append separator
        buffer_ptr[total++] = '.';
    }
    // append last label
    memmove(buffer_ptr + total, label, len);
    total += len;
    //
    // add separator and append domain
    buffer_ptr[total] = '.';
    memmove(buffer + total+2, args->base_host, strlen(args->base_host));
    total+=strlen(args->base_host);

    // convert to format
    return convert_qname_to_format(buffer) ;
}

// TODO: get address
int send_data_ipv4(int socket_fd, FILE *f, struct args_t *args) { 
    if (f == NULL || args == NULL)
        return ERR_OTHER;

    int len, packet_size;
    struct data_queue_t *q = init_queue(f);

    // packet buffer
    uint8_t buffer[CHUNK_SIZE_IPV4] = { 0, };
    // buffer for DNS qname value
    char qname_buffer[QNAME_SIZE] = { 0, };

    // cycle throught whole file and encode data into query and
    // send in to DNS server on the other side of the tunnel
    while ((len = create_query_domain_name((uint8_t *)qname_buffer, QNAME_SIZE, args, q)) == QNAME_SIZE) {
        memset(buffer, 0, CHUNK_SIZE_IPV4);
        packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV4);

        // TODO: check if send was successful
        send(socket_fd, buffer, packet_size, 0);
        packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV4, 0);
    }

    // encode and send last chunk of data
    memset(buffer, 0, CHUNK_SIZE_IPV4);
    packet_size = assemble_query((uint8_t *)qname_buffer, len, buffer, CHUNK_SIZE_IPV4);

    send(socket_fd, buffer, packet_size, 0);
    packet_size = recv(socket_fd, buffer, CHUNK_SIZE_IPV4, 0);

    destroy_queue(q);
    return 0;
}

int send_data_ipv6(int socket_fd, FILE *f, struct args_t *args) { 
    if (f == NULL || args == NULL)
        return ERR_OTHER;

    uint8_t buffer[CHUNK_SIZE_IPV6] = { 0, };
    // TODO:
    return 0;
}


/* dns_query.c */
