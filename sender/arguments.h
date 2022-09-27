/**
 * @brief Declaration of arguments structure and functions.
 * This file contains functions that operates with program's arguments.
 * 
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @file    arguments.h
 * @author  Hung Do
 * @date    11/2022
 */
#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

/**
 * Structure that holds program's arguments.
 */
struct args_t {
    const char *base_host;
    const char *upstream_dns_ip;
    const char *dst_filepath;
    const char *src_filepath;
};

/**
 * @brief Initialization of arguments holder.
 *
 * This function initializes structure that stores program's arguments.
 * Whenever problem occures (missing argument or allocation error)
 * function frees up all allocated memory a returns non-zero value.
 * Program is successfully finished (return code 0) when -h flag is called.
 * @param args  Address of structure's instance to initialize.
 * @param argc  Size of argument vector.
 * @param argv  Argument vector.
 * @returns 0 when no problem occurs; non-zero value otherwise.
 */
int init_args_t(struct args_t *args, int argc, char * const *argv);

/**
 * @brief Prints program usage.
 */
void usage();

/**
 * @brief Destruction of arguments holder.
 *
 * Function frees up all allocated memory in structure.
 * @param args Address of structure's instance to initialize.
 */
void destroy_args_t(struct args_t *args);

#endif // _ARGUMENTS_H_
