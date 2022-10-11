/**
 * @file   arguments.h
 * @author Hung Do
 * @date   11/2022
 */
#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

// here write your header

struct args_t {
    const char *base_host;
    const char *dst_filepath;
};

int load_arguments(struct args_t *args, const int argc, char * const *argv);

#endif // _ARGUMENTS_H_
