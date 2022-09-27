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
#include <stdlib.h>     // calloc, exit, free
#include <stdio.h>      // puts
#include <string.h>     // strlen, strncpy
#include <unistd.h>     // getopt

/**
 * @brief Clones string literal.
 *
 * Function allocates memory for given string literal then it copies its content
 * to newly created buffer. If buffer allocation is unsuccessful function
 * returns NULL.
 * @param arg_val String literal from which new buffer will be cloned.
 * @returns New buffer when successful; NULL otherwise.
 */
static char *clone_string(const char *arg_val) {
    size_t arr_size = strlen(arg_val) + 1;
    // buffer allocation and control check
    char *buffer = (char *) calloc(1, arr_size);
    if (buffer == NULL)
        return NULL;
    strncpy(buffer, arg_val, arr_size);
    return buffer;
}

int init_args_t(struct args_t *args, int argc, char * const *argv) {
    if (args == NULL)
        goto error_handling;

    int opt = 1;
    // check options with arguments
    while ((opt = getopt(argc, argv, "b:u:h")) != -1) {
        switch(opt) {
            case 'b':   // set base host
                if ((args->base_host = clone_string(optarg)) == NULL)
                    goto error_alloc;
                break;
            case 'u':   // set upstream DNS IP
                if ((args->upstream_dns_ip = clone_string(optarg)) == NULL)
                    goto error_alloc;
                break;
            case 'h':   // print help
                usage();
                destroy_args_t(args);
                exit(0);
        }
    }
    // missing mandatory argument
    if (argc == optind) {
        fprintf(stderr, "Missing destination path\n");
        goto error_handling;
    }
    
    // alloc dst_filepath
    if ((args->dst_filepath = clone_string(argv[optind++])) == NULL)
        goto error_alloc;

    // alloc src_filepath
    if (optind < argc) {
        if ((args->src_filepath = clone_string(argv[optind])) == NULL)
            goto error_alloc;
    }
    return 0;

// handling errors
error_alloc:
    fprintf(stderr, "Memory allocation failed\n");
error_handling:
    destroy_args_t(args);
    return -1;
}

void usage() {
    // TODO:
    printf("");
}

void destroy_args_t(struct args_t *args) {
    if (args == NULL)
        return;

    free((char *)args->base_host);
    free((char *)args->upstream_dns_ip);
    free((char *)args->dst_filepath);
    free((char *)args->src_filepath);

    // set null to all
    args->base_host = args->upstream_dns_ip = 
        args->dst_filepath = args->src_filepath = NULL;
}

/* arguments.c */
