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
#include <unistd.h>     // close
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <string.h>     // strerror
#include <errno.h>      // errno

#include "arguments.h"
#include "error.h"
#include "macros.h"
#include "dns_response.h"

/**
 * @brief Setup socket options.
 *
 * @param fd Socket file descriptor.
 */
void socket_setup_opts(int fd) {
    static struct {
        const int reuse;
        const int ipv6_only;
    } sock_opts = {
        .reuse=1,
        .ipv6_only=0,
    };

    // https://stackoverflow.com/questions/1618240/how-to-support-both-ipv4-and-ipv6-connections
    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY,
               &sock_opts.ipv6_only, sizeof(sock_opts.ipv6_only));
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sock_opts.reuse, sizeof(int));
}

int main(int argc, char *argv[]) {
    struct args_t args = { NULL, NULL };
    int socket_fd, err_value = NO_ERR;
    if (load_arguments(&args, argc, argv))
        return 1;

    // setup server
    struct sockaddr_in6 server = { 0, };
    server.sin6_family = AF_INET6;
    server.sin6_port = htons(DNS_PORT);
    server.sin6_addr = in6addr_any;

    do {
        socket_fd = socket(AF_INET6, SOCK_DGRAM, 0);
        if (socket_fd < 0)
            ERR_MSG(ERR_SOCKET, "Unable to create socket\n");

        socket_setup_opts(socket_fd);

        if (bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
            err_value = ERR_BIND;
            printf("%s\n", strerror(errno));
            goto error_handle;
        }

        // waiting and processing clients queries
        if ((err_value = run_communication(socket_fd, &args)) != NO_ERR)
            goto error_handle;

        close(socket_fd);
    } while(CONTINUOUS_RUNNING);

    return NO_ERR;

error_handle:
    close(socket_fd);
    return err_value;
}

/* main.c */
