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

#define DNS_SERVER_INDEX 0

/**
 * @brief Setup socket adress given string IP value.
 *
 * @param sockaddr Structure for handling internet address.
 * @param upstream_ip String containing upstream DNS IP address.
 * @return int     Error code.
 */
int set_ip_version(struct sockaddr_in *sockaddr, const char *upstream_ip) {
    // https://stackoverflow.com/a/792016
    // check IPv4 format
    if (inet_pton(AF_INET, upstream_ip, &(sockaddr->sin_addr.s_addr)))
        sockaddr->sin_family = AF_INET;
    // check IPv6 format
    else if (inet_pton(AF_INET6, upstream_ip, &(sockaddr->sin_addr.s_addr)))
        sockaddr->sin_family = AF_INET6;
    else       // wrong format
        return ERR_IP_FORMAT;
    return NO_ERR;
}

/**
 * @brief Setup structure for handling internet address.
 *
 * @param sockaddr    Pointer to structure.
 * @param upstream_ip String containing upstream DNS IP address.
 * @return int        Error code.
 */
int set_sockaddr(struct sockaddr_in *sockaddr, const char *upstream_ip) {
    sockaddr->sin_port = ntohs(53);

    if (upstream_ip == NULL) {
        // get system's DNS server locations
        FILE *resolv_f = fopen("/etc/resolv.conf", "r");
        if (resolv_f == NULL)
            return ERR_NO_FILE;
        char buffer[100] = { 0, }, dns_servers[10][50] = { 0, }, *token;
        int counter = 0;

        // store system's DNS server locations
        while (fgets(buffer, 100, resolv_f) != NULL && counter < 10) {
            if (!strncmp(buffer, "nameserver", 10)) {
                // namespace
                token = strtok(buffer, " ");
                // address
                token = strtok(NULL, "\n");
                strncpy(dns_servers[counter], token, strlen(token));
                printf("%s\n", dns_servers[counter]);
                counter++;
            }
        }

        fclose(resolv_f);
        return set_ip_version(sockaddr, dns_servers[DNS_SERVER_INDEX]);
    }

    return set_ip_version(sockaddr, upstream_ip);
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
    if ((err_val = set_sockaddr(&server, args.upstream_dns_ip)) < 0)
        ERR_MSG(err_val, "Invalid IP address\n");

    if (connect(socket_fd, (const struct sockaddr *)&server, sizeof(server)) != 0)
        ERR_MSG(ERR_CONNECT, "UDP socket connection failed\n");

    err_val = send_data(socket_fd, &args);

    close(socket_fd);
    return 0;
}

/* main.c */
