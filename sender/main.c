/**
 * @file    main.c
 * @author  Hung Do
 * @date    11/2022
 */
#include <stdio.h>
#include "arguments.h"

int main(int argc, char *argv[]) {
    struct args_t args = { NULL, NULL, NULL, NULL };
    init_args_t(&args, argc, argv);

    // TODO:

    destroy_args_t(&args);
    return 0;
}

/* main.c */
