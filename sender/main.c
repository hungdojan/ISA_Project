/**
 * This is the main file from which the program starts.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @file    main.c
 * @author  Hung Do
 * @date    11/2022
 */
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>     // close
#include <arpa/inet.h>  // struct sockaddr_in

#include "error.h"
#include "arguments.h"
#include "dns_query.h"
#include "macros.h"

int set_sockaddr(struct sockaddr_in *sockaddr, const char *upstream_ip) {
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = ntohs(DNS_PORT);
    if (upstream_ip == NULL) {
        // TODO: get from /etc/resolv.conf
    } else {
        // TODO: validate ip format
        // TODO: make ipv6 -> inet_pton
        // https://stackoverflow.com/a/792016
        int ret = inet_pton(AF_INET, upstream_ip, &(sockaddr->sin_addr.s_addr));
        (void)ret; // TODO: check return value
    }
    return NO_ERR;
}

int send_data(int socket_fd, struct args_t *args) {
    if (args == NULL)
        return ERR_OTHER;

    FILE *f = (args->src_filepath != NULL) ? fopen(args->src_filepath, "rb")
                                           : stdin;
    if (f == NULL)
        ERR_MSG(ERR_NO_FILE, "File %s not found\n", args->src_filepath);
    
    // TODO: option to send using IPv6
    send_data_ipv4(socket_fd, f, args);

    if (f != stdin)
        fclose(f);
    
    return 0;
}

int main(int argc, char *argv[]) {
    // load arguments
    int err_val = NO_ERR;
    struct args_t args = { NULL, NULL, NULL, NULL };
    if ((err_val = load_args(&args, argc, argv)) != 0)
        return err_val;

    // create UDP socket
    int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd < 0)
        ERR_MSG(ERR_SOCKET, "Unable to open socket.\n");

    struct sockaddr_in server = { 0, };
    // TODO: create socket address -> use default DNS when no -u is specified
    if ((err_val = set_sockaddr(&server, args.upstream_dns_ip)) < 0)
        ERR_MSG(err_val, "sockaddr\n");

    if (connect(socket_fd, (const struct sockaddr *)&server, sizeof(server)) != 0)
        ERR_MSG(ERR_CONNECT, "UDP socket connection failed\n");

    err_val = send_data(socket_fd, &args);
    // TODO: deal with errors

    close(socket_fd);

    return 0;
}

/* main.c */
