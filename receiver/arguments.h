/**
 * @brief This file declares argument structure and operation over it.
 *
 * Data are extracted from `argv` array parsed from main function.
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file arguments.h
 * @date 2022-10-23
 */
#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

/**
 * @brief Structure that stores program arguments.
 */
struct args_t {
    const char *base_host;      /**< DNS tunneling identifier. */
    const char *dst_dirpath;    /**< Path to directory to store incoming data. */
};

/**
 * @brief Extracts program arguments.
 * Extracted data are stored in `args` given by user.
 *
 * @param args Structure that holds program arguments.
 * @param argc Number of arguments loaded.
 * @param argv Vector of argument values.
 * @return int Non-zero value when error occurs (defined in include/error.h).
 */
int load_arguments(struct args_t *args, const int argc, char * const *argv);

#endif // _ARGUMENTS_H_
