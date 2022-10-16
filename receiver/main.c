/**
 * @file    main.c
 * @author  Hung Do
 * @date    11/2022
 */
#include <stdio.h>
#include <errno.h>
#include <math.h>       // ceil
#include <arpa/inet.h>  // struct sockaddr_in
#include <unistd.h>     // close
#include <string.h>     // memcpy

#include "arguments.h"
#include "dns_receiver_events.h"
#include "dns_header.h"
#include "error.h"
#include "macros.h"
#include "data_queue.h"

#define MAX_QUERY_SIZE (sizeof(struct dns_header) + 255 + 2 * sizeof(uint16_t))

void socket_setup_opts(int fd) {
    static struct {
        int ipv6_only;
    } sock_opts = { .ipv6_only=0, };

    // https://stackoverflow.com/questions/1618240/how-to-support-both-ipv4-and-ipv6-connections
    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY,
               &sock_opts.ipv6_only, sizeof(sock_opts.ipv6_only));
}

void print_data(uint8_t const *buffer, const char *domain_name,
        struct data_queue_t *q) {
    buffer+=sizeof(struct dns_header);
    static uint8_t a[64] = { 0, };
    uint8_t size = 0;
    for (; buffer[0] != 0; buffer+=size) {
        size = buffer[0];
        buffer++;
        memmove(a, buffer, size);
        a[size] = '\0';
        if (!strncmp((char *)a, domain_name, strlen((char *)a))) {
            break;
        }
        append_data_from_domain(q, a, size);
    }
}

int main(int argc, char *argv[]) {
    struct args_t args = { NULL, NULL };
    if (load_arguments(&args, argc, argv))
        return 1;

    struct sockaddr_in6 client = { 0, }, server = { 0, };
    server.sin6_family = AF_INET6;
    server.sin6_port = htons(DNS_PORT);
    server.sin6_addr = in6addr_any;

    int socket_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (socket_fd < 0)
        return ERR_SOCKET;
    socket_setup_opts(socket_fd);

    if (bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return ERR_BIND;
    }

    // TODO: refactor later
    int n;
    socklen_t socket_size = sizeof(client);
    uint8_t buffer[PACKET_SIZE];
    FILE *file = fopen("data.txt", "wb");
    struct data_queue_t *q = init_queue(file);
    if (q == NULL)
        return ERR_OTHER;

    while ((n = recvfrom(socket_fd, buffer, PACKET_SIZE, 0,
                    (struct sockaddr *) &client, &socket_size)) == MAX_QUERY_SIZE) {
        print_data(buffer, args.base_host, q);
        char *msg = "received";
        memmove(buffer, msg, strlen(msg));
        sendto(socket_fd, buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));
    }
    print_data(buffer, args.base_host, q);
    flush_data_to_file(q);
    char *msg = "received";
    memmove(buffer, msg, strlen(msg));
    sendto(socket_fd, buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));

    if (n == -1)
        printf("%s\n", strerror(errno));

    destroy_queue(q);
    fclose(file);
    close(socket_fd);
    return 0;
}

/* main.c */
