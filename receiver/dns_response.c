#include <string.h>     // memset
#include <unistd.h>     // close, mkdir
#include <errno.h>
#include <sys/stat.h>   // stat

#include "dns_header.h"
#include "data_queue.h"
#include "dns_response.h"
#include "dns_receiver_events.h"
#include "error.h"
#include "base64.h"
#include "macros.h"

#define MAX_QUERY_SIZE (sizeof(struct dns_header) + 255 + 2 * sizeof(uint16_t))
#define SOCKADDR_SIZE(s) ((s).sa_family == AF_INET ? \
                          sizeof(struct sockaddr_in) : \
                          sizeof(struct sockaddr_in6))
#define PRINT_ERROR() printf("%s %d\n", strerror(errno), errno)
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
 * @brief Validates whether packet passes initialization requirements.
 * 
 * @param packet Buffer containing packet's data.
 * @param packet_size Buffer's size.
 * @param base_host Base host domain name.
 * @return int   Non-zero value when packet passes validation check.
 */
int validate_tunnel_communication(const void *packet, size_t packet_size, const char *base_host) {
    // offset to DNS query name
    char *qname = (char *)packet + sizeof(struct dns_header);
    (void)packet_size;

    // convert base host to DNS query name format
    uint8_t query_base_host[QNAME_SIZE] = { 0, };
    memmove(query_base_host+1, base_host, strlen(base_host) + 1);
    int len = convert_qname_to_format(query_base_host);

    // check if string ends with base domain
    if (strncmp((char *)query_base_host, qname + (strlen(qname) - strlen(base_host))-1, len + 1))
        return 0;
    uint16_t *qtype = (uint16_t *)(qname + strlen(qname) + 1);
    // check for dns query type
    return *qtype == ntohs(DNS_TYPE);
}

/**
 * @brief Extract and decode destination file name from initialization DNS query.
 *
 * @param qname_buffer Query domain name.
 * @param file_name    Buffer to store file name.
 * @param args         Program's arguments.
 * @return int         File name's length.
 */
int get_file_name(uint8_t *qname_buffer, uint8_t *file_name, const struct args_t *args) {
    if (qname_buffer == NULL || file_name == NULL || args == NULL)
        return ERR_OTHER;

    char encoded_data[LABEL_SIZE+1] = { 0, };
    uint8_t size = 0;
    int index = 0;

    // decode data from packet payload
    for (; qname_buffer[0] != 0; qname_buffer+=size) {
        size = qname_buffer[0];
        qname_buffer++;
        // stop at domain name
        if (!strncmp((char *)qname_buffer, args->base_host, size)) {
            break;
        }
        memmove(encoded_data + index, qname_buffer, size);
        index += size;
    }
    encoded_data[index] = '\0';

    // make directory for that
    memmove(file_name, args->dst_filepath, strlen(args->dst_filepath));
    size_t last_index = strlen(args->dst_filepath) - 1;
    if (file_name[last_index] != '/') {
        file_name[++last_index] = '/';
        file_name[++last_index] = '\0';
    }
    // https://stackoverflow.com/a/7430262
    struct stat tmp = { 0, };
    if (stat((char *)file_name, &tmp) == -1) {
        mkdir((char *)file_name, 0744);
    }

    size = Base64decode((char *)file_name + last_index, encoded_data);
    return size;
}

static int create_response(uint8_t *packet) {
    if (packet == NULL)
        return ERR_OTHER;
    struct dns_header *header = (struct dns_header *)packet;
    header->id = ntohs(getpid());
    header->param.response.response      = 1;
    header->param.response.opcode        = 0;
    header->param.response.authoritative = 0;
    header->param.response.truncated     = 0;
    header->param.response.rec_desire    = 1;
    header->param.response.rec_avail     = 1;
    header->param.response.zeros         = 0;
    header->param.response.ans_auth      = 0;
    header->param.response.non_auth_data = 0;
    header->param.response.reply_code    = 2;

    return NO_ERR;
}

int normal_dns_response(int socket_fd, uint8_t *packet, size_t packet_size,
        size_t buf_size, struct sockaddr *client, socklen_t c_size) {

    // init communication with Google DNS server.
    struct sockaddr_in dns_server = { 0, };
    dns_server.sin_family = AF_INET;
    inet_pton(AF_INET, "8.8.8.8", &dns_server.sin_addr.s_addr);
    dns_server.sin_port = ntohs(53);

    int dns_socket;
    if ((dns_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        // TODO: do sth abt it
    }

    if (connect(dns_socket, (const struct sockaddr *)&dns_server, sizeof(dns_server)) < -1) {
        ERR_MSG(ERR_CONNECT, "cant connect to 8.8.8.8");
    }

    // send and receive from public DNS
    send(dns_socket, packet, packet_size, 0);
    packet_size = recv(dns_socket, packet, buf_size, 0);

    // send back to client
    sendto(socket_fd, packet, packet_size, 0, client, c_size);
    close(dns_socket);
    return 0;
}

/**
 * @brief Processes initialization packet.
 *
 * @param socket_fd     Socket's file descriptor.
 * @param args          Program's arguments.
 * @param q             Pointer to data queue instance (might be initialized).
 * @param packet_buffer Buffer with packet's content.
 * @param buffer_size   Packet buffer size.
 * @param client        Info of client's address.
 * @return int          Zero when server received initialization packet;
 *                      packet's size when valid DNS query was received.
 */
static int process_init_packet(const int socket_fd, const struct args_t *args,
                               struct data_queue_t **q, uint8_t *packet_buffer,
                               size_t buffer_size, struct sockaddr *client, socklen_t *len) {
    // size of received packet
    int packet_size;
    uint8_t file_name[QNAME_SIZE] = { 0, };

    // receiving packet
    packet_size = recvfrom(socket_fd, packet_buffer, buffer_size, 0, (struct sockaddr *)client, len);

    if (validate_tunnel_communication(packet_buffer, packet_size, args->base_host)) {
        get_file_name(packet_buffer + sizeof(struct dns_header), file_name, args);

        FILE *file = fopen((char *)file_name, "wb");
        if (file == NULL)
            return ERR_OTHER;
        *q = init_queue(file);
        if (q == NULL) {
            fclose(file);
            return ERR_OTHER;
        }
        memmove((*q)->file_name, file_name, strlen((char *)file_name) + 1);

        // TODO: get what version of IP protocol is used in this communication
        if (client->sa_family == AF_INET)
            dns_receiver__on_transfer_init( &((struct sockaddr_in *)client)->sin_addr);
        else
            dns_receiver__on_transfer_init6( &((struct sockaddr_in6 *)client)->sin6_addr);

        create_response(packet_buffer);
        sendto(socket_fd, packet_buffer, packet_size, 0, client, *len);
        return 0;
    }
    return packet_size;
}

/**
 * @brief Extract data from dns queries.
 *
 * @param buffer Packet buffer.
 * @param domain_name Base host domain name.
 * @param q      Data queue instance.
 */
static void process_tunneling_data(uint8_t const *buffer, const char *domain_name,
        struct data_queue_t *q) {
    // offset to get DNS query's qname
    buffer+=sizeof(struct dns_header);

    // buffer for qname's label
    static uint8_t a[64] = { 0, };
    uint8_t size = 0;

    // cycle through qname and extract encoded data only
    // once base host name appears function stops looping
    for (; buffer[0] != 0; buffer+=size) {
        size = buffer[0];
        buffer++;
        if (!strncmp((char *)buffer, domain_name, size)) {
            break;
        }

        memmove(a, buffer, size);
        a[size] = '\0';
        // move to another buffer that will flush it into desired file
        append_data_from_domain(q, a, size);
    }
}

int run_communication(int socket_fd, struct sockaddr *dst, struct args_t *args) {
    // TODO: refactor later
    int packet_size;
    struct sockaddr_storage client = { 0, };
    socklen_t socket_size = sizeof(client);
    uint8_t packet_buffer[PACKET_SIZE] = { 0, };
    struct data_queue_t *q = NULL;

    while (1) {
        // validate init packet
        if ((packet_size = process_init_packet(socket_fd, args, &q, packet_buffer,
                        PACKET_SIZE, (struct sockaddr *)&client, &socket_size)) == 0) {
            // receive and process data
            while ((packet_size = recvfrom(socket_fd, packet_buffer, PACKET_SIZE, 0,
                            (struct sockaddr *)&client, &socket_size)) == MAX_QUERY_SIZE) {
                process_tunneling_data(packet_buffer, args->base_host, q);
                // TODO: data
                create_response(packet_buffer);
                sendto(socket_fd, packet_buffer, packet_size, 0,
                        (struct sockaddr *)&client, socket_size);
            }
            // process last chunk of data
            process_tunneling_data(packet_buffer, args->base_host, q);
            flush_data_to_file(q);
            create_response(packet_buffer);
            sendto(socket_fd, packet_buffer, packet_size, 0, (struct sockaddr *)&client, socket_size);

            // finish log
            dns_receiver__on_transfer_completed(q->file_name, q->file_size);

            if (packet_size == -1)
                printf("%s\n", strerror(errno));

            fclose(q->f);
            destroy_queue(q);
        } else {
            normal_dns_response(socket_fd, packet_buffer, packet_size,
                    PACKET_SIZE, (struct sockaddr *)&client, socket_size);
        }
    }
}

/* dns_response.c */
