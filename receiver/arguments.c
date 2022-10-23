/**
 * @file    arguments.c
 * @author  Hung Do
 * @date    11/2022
 */
#include "arguments.h"
#include <stdio.h>

int load_arguments(struct args_t *args, const int argc, char * const *argv) {
    if (args == NULL)
        return -1;

    if (argc < 3) {
        fprintf(stderr, "dns_receiver: Missing arguments.\n"
                        "expected: ./dns_received {BASE_HOST} {DST_DIRPATH}\n");
        return -1;
    }

    args->base_host    = argv[1];
    args->dst_dirpath = argv[2];
    return 0;
}

/* arguments.c */
