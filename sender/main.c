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
#include <stdio.h>
#include "arguments.h"

int main(int argc, char *argv[]) {
    struct args_t args = { NULL, NULL, NULL, NULL };

    if (init_args_t(&args, argc, argv) != 0) {
        return -1;
    }

    destroy_args_t(&args);
    return 0;
}

/* main.c */
