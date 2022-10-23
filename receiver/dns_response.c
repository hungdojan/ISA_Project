#include <string.h>     // memset
#include <unistd.h>     // close, mkdir
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>   // stat

#include "dns_header.h"
#include "data_queue.h"
#include "dns_response.h"
#include "dns_receiver_events.h"
#include "error.h"
#include "base64.h"
#include "macros.h"

#define MAX_QUERY_SIZE (sizeof(struct dns_header) + 255 + 2 * sizeof(uint16_t))
#define PRINT_ERROR() printf("%s %d\n", strerror(errno), errno)

#define RECEIVE_IPV4(src_addr, packet_buffer, packet_size, base_host, q, qname_buffer, file_name) \
            do { \
                if (validate_tunnel_communication(packet_buffer, packet_size, base_host)) {         \
                    int chunk_size = process_tunneling_data(packet_buffer, base_host, q);           \
                    if (chunk_size < 0)     continue; \
                    dns_receiver__on_chunk_received(src_addr, file_name, q->chunkID, chunk_size);   \
                    memmove(qname_buffer, packet_buffer + sizeof(struct dns_header),                \
                            strlen((char *)packet_buffer + sizeof(struct dns_header))+1);           \
                    convert_format_to_qname(qname_buffer);                                          \
                    dns_receiver__on_query_parsed(file_name, qname_buffer+1);                       \
                } \
                create_response(packet_buffer);                   \
                sendto(socket_fd, packet_buffer, packet_size, 0,  \
                        (struct sockaddr *)&client, socket_size); \
            } while (0)

#define RECEIVE_IPV6(src_addr, packet_buffer, packet_size, base_host, q, qname_buffer, file_name) \
            do { \
                if (validate_tunnel_communication(packet_buffer, packet_size, base_host)) {         \
                    int chunk_size = process_tunneling_data(packet_buffer, base_host, q);           \
                    if (chunk_size < 0)     continue; \
                    dns_receiver__on_chunk_received6(src_addr, file_name, q->chunkID, chunk_size);  \
                    memmove(qname_buffer, packet_buffer + sizeof(struct dns_header),                \
                            strlen((char *)packet_buffer + sizeof(struct dns_header))+1);           \
                    convert_format_to_qname(qname_buffer);                                          \
                    dns_receiver__on_query_parsed(file_name, qname_buffer+1);                       \
                } \
                create_response(packet_buffer);                   \
                sendto(socket_fd, packet_buffer, packet_size, 0,  \
                        (struct sockaddr *)&client, socket_size); \
            } while (0)

#define PREFIX_SIZE 6
const uint16_t _IPV4_MAPPED_PREFIX[PREFIX_SIZE] = { 0, 0, 0, 0, 0, 0xffff };

static size_t extract_chunk_ID(const uint8_t *packet, size_t *chunk_id_size) {
    char num_buffer[20] = { 0, };
    memcpy(num_buffer, packet+1, packet[0]);
    num_buffer[packet[0]] = '\0';
    *chunk_id_size = strlen(num_buffer);
    return strtol(num_buffer, NULL, 10);
}

/**
 * @brief Checks if sender uses IPv4 communication stored in IPv6 address.
 * Mapped IPv4 uses prefix ::ffff:[IPv4] in IPv6 address.
 * 
 * https://stackoverflow.com/questions/14272656/how-to-resolve-ipv4-address-from-ipv4-mapped-ipv6-address
 *
 * @param addr IPv6 address.
 * @return int Zero when address contains IPv4 mapped prefix.
 */
int is_ipv4_mapped(void *addr) {
    return memcmp(addr, _IPV4_MAPPED_PREFIX, PREFIX_SIZE * sizeof(uint16_t));
}

/**
 * @brief Shift IPv6 address to mapped IPv4 section.
 *
 * @param addr IPv6 address.
 * @return (void *) Beginning of IPv4 section.
 */
void *get_ip_address(struct sockaddr *addr) {
    void *address = &(((struct sockaddr_in6*)addr)->sin6_addr);
    if (!is_ipv4_mapped(address)) {
        return (uint8_t *)address + sizeof(uint16_t) * PREFIX_SIZE;
    }
    return address;
}

/**
 * @brief Converts qname buffer content into valid query name format.
 *
 * @param buffer Output data buffer.
 * @return int   Total length of qname.
 */
static int convert_qname_to_format(char *buffer) {
    // split string into two strings with '.' as a separator
    char *token = strtok(buffer+1, ".");

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
 * @brief Converts query formatted name into human readable format.
 *
 * @param buffer Buffer with data.
 */
static void convert_format_to_qname(char *buffer) {
    uint8_t jump_size = *buffer;
    size_t index = 0;
    while (jump_size != 0) {
        index += jump_size+1;
        jump_size = buffer[index];
        buffer[index] = '.';
    }
    buffer[index] = '\0';
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
    char query_base_host[QNAME_SIZE] = { 0, };
    memmove(query_base_host+1, base_host, strlen(base_host) + 1);
    int len = convert_qname_to_format(query_base_host);

    // check if string ends with base domain
    if (strncmp(query_base_host, qname + (strlen(qname) - strlen(base_host))-1, len + 1))
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
    memmove(file_name, args->dst_dirpath, strlen(args->dst_dirpath));
    size_t last_index = strlen(args->dst_dirpath) - 1;
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

/**
 * @brief Transfor query packet to response packet.
 *
 * @param packet Buffer containing packet data.
 * @return int Zero when no error occurs.
 */
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
        ERR_MSG(ERR_SOCKET, "Cant create socket for communication with 8.8.8.8\n");
    }

    if (connect(dns_socket, (const struct sockaddr *)&dns_server, sizeof(dns_server)) < -1) {
        ERR_MSG(ERR_CONNECT, "Can't connect to 8.8.8.8\n");
    }

    // send and receive from public DNS
    send(dns_socket, packet, packet_size, 0);
    packet_size = recv(dns_socket, packet, buf_size, 0);

    // send back to client
    sendto(socket_fd, packet, packet_size, 0, client, c_size);
    close(dns_socket);
    return NO_ERR;
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
        size_t chunk_id_offset;
        size_t chunk_id_value = extract_chunk_ID(packet_buffer + sizeof(struct dns_header), &chunk_id_offset);
        chunk_id_offset++;
        get_file_name(packet_buffer + sizeof(struct dns_header) + chunk_id_offset, file_name, args);

        // initialize output file and data queue instance for later use
        FILE *file = fopen((char *)file_name, "wb");
        if (file == NULL)
            return ERR_OTHER;
        *q = init_queue(file);
        if (q == NULL) {
            fclose(file);
            return ERR_OTHER;
        }
        (*q)->chunkID = chunk_id_value;
        memmove((*q)->file_name, file_name, strlen((char *)file_name) + 1);

        // start communication between client and server
        if (!is_ipv4_mapped( &((struct sockaddr_in6 *)client)->sin6_addr))
            dns_receiver__on_transfer_init(get_ip_address(client));
        else
            dns_receiver__on_transfer_init6(get_ip_address(client));

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
static int process_tunneling_data(uint8_t const *buffer, const char *domain_name,
        struct data_queue_t *q) {
    // offset to get DNS query's qname
    buffer+=sizeof(struct dns_header);

    // buffer for qname's label
    static uint8_t a[64] = { 0, };
    uint8_t size = 0;
    int chunk_size = 0;

    // checking chunkID of received packet
    // if received packet contains old packet, ignore it 
    size_t chunk_id_offset;
    size_t chunk_id_value = extract_chunk_ID(buffer, &chunk_id_offset);
    if (chunk_id_value <= q->chunkID)
        return ERR_OTHER;
    buffer += chunk_id_offset + 1;
    q->chunkID = chunk_id_value;

    // cycle through qname and extract encoded data only
    // once base host name appears function stops looping
    for (; buffer[0] != 0; buffer+=size) {
        size = buffer[0];
        buffer++;
        if (!strncmp((char *)buffer, domain_name, size)) {
            break;
        }
        chunk_size += size;

        memmove(a, buffer, size);
        a[size] = '\0';
        // move to another buffer that will flush it into desired file
        append_data_from_domain(q, a, size);
    }
    return chunk_size;
}

int run_communication(int socket_fd, struct args_t *args) {
    // TODO: refactor later
    int packet_size;
    struct sockaddr_storage client = { 0, };
    socklen_t socket_size = sizeof(client);
    uint8_t packet_buffer[PACKET_SIZE] = { 0, };
    char qname_buffer[QNAME_SIZE] = { 0, };
    struct data_queue_t *q = NULL;

    // validate init packet
    if ((packet_size = process_init_packet(socket_fd, args, &q, packet_buffer,
                    PACKET_SIZE, (struct sockaddr *)&client, &socket_size)) == 0) {
        // receive and process data
        while ((packet_size = recvfrom(socket_fd, packet_buffer, PACKET_SIZE, 0,
                        (struct sockaddr *)&client, &socket_size)) == MAX_QUERY_SIZE) {
            if (!is_ipv4_mapped( &((struct sockaddr_in6 *)&client)->sin6_addr )) {
                RECEIVE_IPV4(get_ip_address((struct sockaddr *)&client), packet_buffer,
                        packet_size, args->base_host, q, qname_buffer, q->file_name);
            } else {
                RECEIVE_IPV6(get_ip_address((struct sockaddr *)&client), packet_buffer,
                        packet_size, args->base_host, q, qname_buffer, q->file_name);
            }
        }
        // process last chunk of data
        if (!is_ipv4_mapped( &((struct sockaddr_in6 *)&client)->sin6_addr )) {
            RECEIVE_IPV4(get_ip_address((struct sockaddr *)&client), packet_buffer,
                    packet_size, args->base_host, q, qname_buffer, q->file_name);
        } else {
            RECEIVE_IPV6(get_ip_address((struct sockaddr *)&client), packet_buffer,
                    packet_size, args->base_host, q, qname_buffer, q->file_name);
        }
        flush_data_to_file(q);

        // finish log
        dns_receiver__on_transfer_completed(q->file_name, q->file_size);

        if (packet_size == -1)
            printf("%s\n", strerror(errno));

        fclose(q->f);
        destroy_queue(q);
    } else {
        // use Google's public DNS server to answer query
        normal_dns_response(socket_fd, packet_buffer, packet_size,
                PACKET_SIZE, (struct sockaddr *)&client, socket_size);
    }
    return 0;
}

/* dns_response.c */
