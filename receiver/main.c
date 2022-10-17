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
#include "dns_response.h"

void socket_setup_opts(int fd) {
    static struct {
        int ipv6_only;
    } sock_opts = { .ipv6_only=0, };

    // https://stackoverflow.com/questions/1618240/how-to-support-both-ipv4-and-ipv6-connections
    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY,
               &sock_opts.ipv6_only, sizeof(sock_opts.ipv6_only));
}

int main(int argc, char *argv[]) {
    struct args_t args = { NULL, NULL };
    if (load_arguments(&args, argc, argv))
        return 1;

    // setup server
    struct sockaddr_in6 server = { 0, };
    server.sin6_family = AF_INET6;
    server.sin6_port = htons(DNS_PORT);
    server.sin6_addr = in6addr_any;

    int socket_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        ERR_MSG(ERR_SOCKET, "Unable to create socket\n");
        return ERR_SOCKET;
    }
    socket_setup_opts(socket_fd);

    if (bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return ERR_BIND;
    }

    run_communication(socket_fd, (struct sockaddr *)&server, &args);

    close(socket_fd);
    return 0;
}

/* main.c */
