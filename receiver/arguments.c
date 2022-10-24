/**
 * @brief This file defines argument structure operations.
 *
 * Data are extracted from `argv` array parsed from main function.
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file arguments.c
 * @date 2022-10-23
 */
#include <stdio.h>
#include "arguments.h"
#include "error.h"

static void usage() {
    printf("DNS receiver\n"
           "dns_receiver {BASE_HOST} {DST_DIRPATH}\n"
           "\n"
           "Positional parameters:\n"
           "   BASE_HOST                           Set base host domain.\n"
           "   DST_DIRPATH                         Path to a directory where data will be stored.\n"
    );

}

int load_arguments(struct args_t *args, const int argc, char * const *argv) {
    if (args == NULL)
        return ERR_OTHER;

    // expected format: ./dns_receiver {BASE_HOST} {DST_DIRPATH}
    // everything after last mandatory parameter DST_DIRPATH will be ignored
    if (argc < 3) {
        fprintf(stderr, "dns_receiver: Missing arguments.\n");
        usage();
        return ERR_ARGS;
    }

    args->base_host    = argv[1];
    args->dst_dirpath = argv[2];
    return NO_ERR;
}

/* arguments.c */
