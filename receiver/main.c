/**
 * @file    main.c
 * @author  Hung Do
 * @date    11/2022
 */
#include <stdio.h>
#include <string.h>     // memcpy
#include <errno.h>
#include <unistd.h>     // close
#include <arpa/inet.h>  // struct sockaddr_in
#include <sys/time.h>   // struct timeval
#include <sys/socket.h> 

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
        struct timeval timeout;
    } sock_opts = { 
        .ipv6_only=0,
        .timeout = {
            .tv_sec = 3,
            .tv_usec = 0
        }
    };

    // https://stackoverflow.com/questions/1618240/how-to-support-both-ipv4-and-ipv6-connections
    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY,
               &sock_opts.ipv6_only, sizeof(sock_opts.ipv6_only));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &(sock_opts.timeout), sizeof(sock_opts.timeout));
    // setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &(sock_opts.timeout), sizeof(sock_opts.timeout));
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

    do {
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

        run_communication(socket_fd, &args);

        close(socket_fd);
    } while(CONTINUOUS_RUNNING);
    return 0;
}

/* main.c */
