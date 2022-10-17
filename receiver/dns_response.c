#include <string.h>     // memset
#include <unistd.h>     // close
#include <errno.h>

#include "dns_header.h"
#include "data_queue.h"
#include "dns_response.h"
#include "dns_receiver_events.h"
#include "error.h"
#include "base64.h"
#include "macros.h"

#define MAX_QUERY_SIZE (sizeof(struct dns_header) + 255 + 2 * sizeof(uint16_t))

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

int validate_tunnel_communication(void *packet, size_t packet_size, const char *base_host) {
    char *qname = (char *)packet + sizeof(struct dns_header);

    // convert base host to DNS query name format
    uint8_t query_base_host[QNAME_SIZE] = { 0, };
    memmove(query_base_host+1, base_host, strlen(base_host) + 1);
    int len = convert_qname_to_format(query_base_host);

    if (strncmp((char *)query_base_host, qname + (strlen(qname) - strlen(base_host))-1, len + 1))
        return 0;
    uint16_t *qtype = (uint16_t *)(qname + strlen(qname) + 1);
    return *qtype == ntohs(DNS_TYPE);
}

int get_file_name(uint8_t *qname_buffer, uint8_t *file_name, struct args_t *args) {
    if (qname_buffer == NULL || file_name == NULL || args == NULL)
        return ERR_OTHER;

    char encoded_data[LABEL_SIZE+1] = { 0, };
    uint8_t size = 0;
    int index = 0;
    for (; qname_buffer[0] != 0; qname_buffer+=size) {
        size = qname_buffer[0];
        qname_buffer++;
        if (!strncmp((char *)qname_buffer, args->base_host, size)) {
            break;
        }
        memmove(encoded_data + index, qname_buffer, size);
        index += size;
    }
    encoded_data[index] = '\0';
    size = Base64decode((char *)file_name, encoded_data);
    return size;
}

static int send_response(int socket_fd, struct sockaddr *dst, FILE *f, struct args_t *args) {
    return 0;
}

int normal_dns_response(int socket_fd, uint8_t *packet, size_t packet_size,
        size_t buf_size, struct sockaddr *client, socklen_t c_size) {
    // TODO: test
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

    send(dns_socket, packet, packet_size, 0);
    packet_size = recv(dns_socket, packet, buf_size, 0);

    // FIXME: not working for some reason...
    sendto(socket_fd, packet, packet_size, 0, client, c_size);
    close(dns_socket);
    return 0;
}

static int process_init_packet(int socket_fd, struct args_t *args, FILE **file,
        struct data_queue_t **q, uint8_t *packet_buffer, size_t buffer_size) {
    // size of received packet
    int packet_size;
    struct sockaddr_in6 client = { 0, };
    socklen_t socket_size = sizeof(client);
    uint8_t file_name[QNAME_SIZE] = { 0, };

    // receiving packet
    packet_size = recvfrom(socket_fd, packet_buffer, buffer_size, 0,
            (struct sockaddr *) &client, &socket_size);

    if (validate_tunnel_communication(packet_buffer, packet_size, args->base_host)) {
        get_file_name(packet_buffer + sizeof(struct dns_header), file_name, args);
        printf("filename: %s\n", file_name);

        // send response
        // TODO: debug info: init

        char *msg = "received";
        memmove(packet_buffer, msg, strlen(msg));
        sendto(socket_fd, packet_buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));

        *file = fopen((char *)file_name, "wb");
        if (file == NULL)
            return -1;
        *q = init_queue(*file);
        if (q == NULL) {
            fclose(*file);
            return -1;
        }
        return 0;
    }
    return packet_size;
}

static void process_tunneling_data(uint8_t const *buffer, const char *domain_name,
        struct data_queue_t *q) {
    buffer+=sizeof(struct dns_header);
    static uint8_t a[64] = { 0, };
    uint8_t size = 0;
    for (; buffer[0] != 0; buffer+=size) {
        size = buffer[0];
        buffer++;
        if (!strncmp((char *)buffer, domain_name, size)) {
            break;
        }

        memmove(a, buffer, size);
        a[size] = '\0';
        append_data_from_domain(q, a, size);
    }
}

int run_communication(int socket_fd, struct sockaddr *dst, struct args_t *args) {
    // TODO: refactor later
    int packet_size;
    struct sockaddr_in6 client = { 0, };
    socklen_t socket_size = sizeof(client);
    uint8_t packet_buffer[PACKET_SIZE] = { 0, };
    FILE *f = NULL;
    struct data_queue_t *q = NULL;
    char *msg = "received";

    // TODO: while true??
    while (1) {
    if ((packet_size = process_init_packet(socket_fd, args, &f, &q, packet_buffer, PACKET_SIZE)) == 0) {
        // send recieved
        // TODO: process tunneling
        while ((packet_size = recvfrom(socket_fd, packet_buffer, PACKET_SIZE, 0,
                        (struct sockaddr *) &client, &socket_size)) == MAX_QUERY_SIZE) {
            process_tunneling_data(packet_buffer, args->base_host, q);
            // TODO: data
            memmove(packet_buffer, msg, strlen(msg));
            sendto(socket_fd, packet_buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));
        }
        process_tunneling_data(packet_buffer, args->base_host, q);
        flush_data_to_file(q);
        memmove(packet_buffer, msg, strlen(msg));
        sendto(socket_fd, packet_buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));

        if (packet_size == -1)
            printf("%s\n", strerror(errno));

        fclose(f);
        destroy_queue(q);
    } else {
        normal_dns_response(socket_fd, packet_buffer, packet_size,
                            PACKET_SIZE, (struct sockaddr *)&client, sizeof(client));
        // TODO: send valid response using 8.8.8.8
    }
    }

    // init packet
    // packet_size = recvfrom(socket_fd, packet_buffer, PACKET_SIZE, 0, (struct sockaddr *) &client, &socket_size);
    // get_file_name(packet_buffer + sizeof(struct dns_header), file_name, &args);
    // printf("filename: %s\n", file_name);
    // char *msg = "received";
    // memmove(packet_buffer, msg, strlen(msg));
    // sendto(socket_fd, packet_buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));
    // // return 0;
    // // TODO: check for file validity
    // FILE *file = fopen((char *)file_name, "wb");
    // struct data_queue_t *q = init_queue(file);
    // if (q == NULL)
    //     return ERR_OTHER;

    // while ((packet_size = recvfrom(socket_fd, packet_buffer, PACKET_SIZE, 0,
    //                 (struct sockaddr *) &client, &socket_size)) == MAX_QUERY_SIZE) {
    //     print_data(packet_buffer, args->base_host, q);
    //     char *msg = "received";
    //     memmove(packet_buffer, msg, strlen(msg));
    //     sendto(socket_fd, packet_buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));
    // }
    // print_data(packet_buffer, args->base_host, q);
    // flush_data_to_file(q);
    // // char *msg = "received";
    // memmove(packet_buffer, msg, strlen(msg));
    // sendto(socket_fd, packet_buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));

    // if (packet_size == -1)
    //     printf("%s\n", strerror(errno));

    // destroy_queue(q);
    // fclose(file);
}

/* dns_response.c */
