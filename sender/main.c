/**
 * @brief This is a main file where program starts.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file main.c
 * @date 2022-10-23
 */
#include <string.h>     // strlen
#include <arpa/inet.h>  // inet_pton
#include <unistd.h>     // close
#include <netinet/in.h> // struct sockaddr_in
#include <sys/time.h>   // struct timeval
#include <sys/socket.h>
#include <errno.h>

#include "error.h"
#include "arguments.h"
#include "dns_query.h"
#include "macros.h"

#define DNS_SERVER_INDEX 0
#define SOCKADDR_LEN(socket) (socket.sa_family == AF_INET ? \
                              sizeof(struct sockaddr_in) :  \
                              sizeof(struct sockaddr_in6))

/**
 * @brief Setup socket adress given string IP value.
 *
 * @param sockaddr    Structure for handling internet address.
 * @param upstream_ip String containing upstream DNS IP address.
 * @return int        Error code.
 */
int set_ip_version(struct sockaddr *sockaddr, const char *upstream_ip) {
    // https://stackoverflow.com/a/792016
    // check IPv4 format
    if ( inet_pton(AF_INET, upstream_ip,
                &(( (struct sockaddr_in *) sockaddr)->sin_addr)) ) {
        sockaddr->sa_family = AF_INET;
        struct sockaddr_in *s = (struct sockaddr_in *)sockaddr;
        s->sin_port = ntohs(DNS_PORT);

    // check IPv6 format
    } else if ( inet_pton(AF_INET6, upstream_ip,
                &(( (struct sockaddr_in6 *) sockaddr)->sin6_addr)) ) {
        sockaddr->sa_family = AF_INET6;
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)sockaddr;
        s->sin6_port = ntohs(DNS_PORT);

    } else      // wrong format
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
int set_sockaddr(struct sockaddr *sockaddr, const char *upstream_ip) {
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
                memcpy(dns_servers[counter], token, strlen(token));
                counter++;
            }
        }

        fclose(resolv_f);
        return set_ip_version(sockaddr, dns_servers[DNS_SERVER_INDEX]);
    }

    return set_ip_version(sockaddr, upstream_ip);
}

/**
 * @brief Start communication with DNS server.
 *
 * @param socket_fd Socket file descriptor
 * @param args      Program argument's instance.
 * @param dst       Server's address handler.
 * @return int      0 value when no error occurs; non-zero value otherwise.
 */
static int send_data(int socket_fd, struct args_t *args, struct sockaddr *dst) {
    int err_val;
    if (args == NULL)
        return ERR_OTHER;

    FILE *f = (args->src_filepath != NULL) ? fopen(args->src_filepath, "rb")
                                           : stdin;
    if (f == NULL)
        ERR_MSG(ERR_NO_FILE, "File %s not found\n", args->src_filepath);

    // TODO: option to send using IPv6
    if (dst->sa_family == AF_INET) {
        err_val = send_data_ipv4(socket_fd, (struct sockaddr_in *)dst, f, args);
    } else {
        err_val = send_data_ipv6(socket_fd, (struct sockaddr_in6 *)dst, f, args);
    }

    if (f != stdin)
        fclose(f);

    return err_val;
}

/**
 * @brief Setup socket options.
 *
 * @param fd   Socket file descriptor.
 */
static void socket_setup_opts(int fd) {
    static struct {
        struct timeval timeout;
    } sock_opts = {
        .timeout = {
            .tv_sec = 3,
            .tv_usec = 0
        }
    };

    // settings timeouts
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &(sock_opts.timeout), sizeof(sock_opts.timeout));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &(sock_opts.timeout), sizeof(sock_opts.timeout));
}

int main(int argc, char *argv[]) {
    // load arguments
    int err_val = NO_ERR;
    struct args_t args = { NULL, NULL, NULL, NULL };
    if ((err_val = load_args(&args, argc, argv)) != 0)
        return err_val;

    struct sockaddr server = { 0, };
    if ((err_val = set_sockaddr(&server, args.upstream_dns_ip)) < 0)
        ERR_MSG(err_val, "Invalid IP address\n");

    // create UDP socket
    int socket_fd = socket(server.sa_family, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd < 0)
        ERR_MSG(ERR_SOCKET, "Unable to open socket.\n");

    socket_setup_opts(socket_fd);

    if (connect(socket_fd, &server, SOCKADDR_LEN(server)) != 0) {
        close(socket_fd);
        ERR_MSG(ERR_CONNECT, "UDP socket connection failed\n");
    }

    err_val = send_data(socket_fd, &args, &server);

    close(socket_fd);
    return err_val;
}

/* main.c */
