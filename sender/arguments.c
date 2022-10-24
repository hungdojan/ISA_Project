/**
 * @brief Definition of arguments.h functions.
 * This file defines all functions that operates with 'struct args_t' structure.
 * Structure args_t stores information loaded from program's arguments.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @file arguments.c
 * @author Hung Do
 * @date 11/2022
 */

// defining this macro in order to enable getopt in -std=c11
#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 2
#endif

#include "arguments.h"
#include "error.h"
#include <stdlib.h>     // exit
#include <stdio.h>      // puts
#include <unistd.h>     // getopt

int load_args(struct args_t *args, int argc, char * const *argv) {
    if (args == NULL) {
        ERR_MSG(ERR_OTHER, "\n");
    }

    int opt = 1;
    // check options with arguments
    while ((opt = getopt(argc, argv, "u:h")) != -1) {
        switch(opt) {
            case 'u':   // set upstream DNS IP
                args->upstream_dns_ip = optarg;
                break;
            case 'h':   // print help
                usage();
                exit(NO_ERR);
        }
    }
    // missing mandatory arguments
    if (argc - optind < 2) {
        fprintf(stderr, "dns_sender: Missing arguments\n");
        usage();
        return ERR_ARGS;
    }

    // load base_host and dst_filepath
    args->base_host = argv[optind++];
    args->dst_filepath = argv[optind++];

    // load optional argument
    if (optind < argc)
        args->src_filepath = argv[optind];

    return NO_ERR;
}

void usage() {
    printf("DNS sender\n"
           "dns_sender [-h] [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILENAME} [SRC_FILENAME]\n"
           "\n"
           "Options:\n"
           "   -h                                  Display this help message\n"
           "   -u UPSTREAM_DNS_IP                  Set IP address of server to communicate with.\n"
           "Positional parameters:\n"
           "   BASE_HOST                           Set base host domain.\n"
           "   DST_FILEPATH                        Path to a file where data will be stored on server.\n"
           "   SRC_FILEPATH                        Path to file from which data will be sent. (optional)\n"
           "                                       When not specified program will read data from STDIN\n"
    );
}

/* arguments.c */
