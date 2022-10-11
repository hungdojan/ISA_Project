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

// #include <sys/socket.h>
// #include <netinet/in.h>
#include "arguments.h"
#include "dns_receiver_events.h"
#include "dns_header.h"
#include "error.h"

#define DNS_PORT 12345
#define PACKET_SIZE 512
#define MAX_QUERY_SIZE (sizeof(struct dns_header) + 255 + 2 * sizeof(uint16_t))

void print_packet(uint8_t *buffer, size_t buffer_size) {
    puts("-------------- Data Dump -----------------");
    const size_t width = 16;
    const size_t height = ceil((double)buffer_size / 16.0);
    size_t top = 0;
    for (; top < height; top++) {
        printf("0x%-7.4zx", top*width);
        // hex code
        for (size_t left = 0; left < width; left++) {
            if (left)       printf(" ");
            if (left == 8)  printf(" ");
            if (top*width + left < buffer_size)
                printf("%.2x", buffer[top*width + left]);
            else
                printf("  ");
        }

        // add padding
        printf("%3s", "");

        // printable characters
        for (size_t left = 0; left < 16 && top*width +left < buffer_size; left++) {
            if (left == 8)  printf(" ");
            if (buffer[top*width + left] >= 32 && buffer[top*width + left] < 128)
                putchar(buffer[top*width + left]);
            else
                putchar('.');
        }
        putchar('\n');
    }
    puts("");
}

void print_data(uint8_t const *buffer, const char *domain_name, FILE *d) {
    buffer+=sizeof(struct dns_header);
    static uint8_t a[64] = { 0, };
    uint8_t size = 0;
    // print_packet(a, 64);
    for (; buffer[0] != 0; buffer+=size) {
        size = buffer[0];
        buffer++;
        memmove(a, buffer, size);
        a[size] = '\0';
        if (!strncmp((char *)a, domain_name, strlen((char *)a))) {
            break;
        }
        fwrite(a, 1, strlen((char *)a), d);
        fflush(d);
    }
}

int main(int argc, char *argv[]) {
    struct args_t args = { NULL, NULL };
    if (load_arguments(&args, argc, argv))
        return 1;

    // TODO:
    struct sockaddr_in client = { 0, }, server = { 0, };
    server.sin_family = AF_INET;
    server.sin_port = ntohs(DNS_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
        return ERR_SOCKET;
    if (bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return ERR_BIND;
    }

    // TODO: refactor later
    int n;
    socklen_t socket_size = sizeof(client);
    uint8_t buffer[PACKET_SIZE];
    FILE *d = fopen("data.txt", "wb");
    while ((n = recvfrom(socket_fd, buffer, PACKET_SIZE, 0,
                    (struct sockaddr *) &client, &socket_size)) == MAX_QUERY_SIZE) {
        print_data(buffer, args.base_host, d);
        char *msg = "received";
        memmove(buffer, msg, strlen(msg));
        sendto(socket_fd, buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));
    }
    print_data(buffer, args.base_host, d);
    char *msg = "received";
    memmove(buffer, msg, strlen(msg));
    sendto(socket_fd, buffer, strlen(msg), 0, (struct sockaddr *)&client, sizeof(client));

    fclose(d);
    if (n == -1)
        printf("%s\n", strerror(errno));

    close(socket_fd);
    return 0;
}

/* main.c */
