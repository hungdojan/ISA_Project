// defining this macro in order to enable getopt in -std=c11
#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 2
#endif    

#include "arguments.h"
#include <stdlib.h>     // calloc, exit, free
#include <stdio.h>      // puts
#include <string.h>     // strlen, strncpy
#include <unistd.h>     // getopt

void init_args_t(struct args_t *args, int argc, char * const *argv) {
    if (args == NULL)
        return;

    int opt = 1, arr_size = 0;
    while ((opt = getopt(argc, argv, "b:u:h")) != -1) {
        switch(opt) {
            case 'b':
                arr_size = strlen(optarg) + 1;
                args->base_host = (char *) calloc(1, arr_size);
                strncpy(args->base_host, optarg, arr_size);
                break;
            case 'u':
                arr_size = strlen(optarg) + 1;
                args->upstream_dns_ip = (char *) calloc(1, arr_size);
                strncpy(args->upstream_dns_ip, optarg, arr_size);
                break;
            case 'h':   // print help
                usage();
                destroy_args_t(args);
                exit(0);
        }
    }
    if (argc == optind) {
        // TODO: error
        return;
    }
    
    // alloc dst_filepath
    arr_size = strlen(argv[optind]);
    args->dst_filepath = (char *) calloc(1, arr_size);
    strncpy(args->dst_filepath, argv[optind++], arr_size);

    // alloc src_filepath
    if (optind < argc) {
        arr_size = strlen(argv[optind]);
        args->src_filepath = (char *) calloc(1, arr_size);
        strncpy(args->src_filepath, argv[optind], arr_size);
    }
}

void usage() {
    // TODO:
}

void destroy_args_t(struct args_t *args) {
    if (args == NULL)
        return;

    free(args->base_host);
    free(args->upstream_dns_ip);
    free(args->dst_filepath);
    free(args->src_filepath);
}

/* arguments.c */
