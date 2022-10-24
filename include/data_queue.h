/**
 * @brief This file declares data_queue_t structure and operations over it.
 *
 * Whether its used in `dns_receiver` or `dns_sender`, this file declares
 * function used in both programs. The structure stores important information
 * that is used later for debugging/logging purposes.
 *
 * Since this header is used in both programs implementation of the structure
 * differs due to different needs. Some functions can and cannot be defined in
 * respective source directories.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file data_queue.h
 * @date 2022-10-23
 */
#ifndef _DATA_QUEUE_H_
#define _DATA_QUEUE_H_

#include <stdio.h>  // FILE
#include <stdint.h> // uint8_t

#include "macros.h"

struct data_queue_t;
struct args_t;

/**
 * @brief Initializes data queue.
 * `args` parameter is needed in `dns_sender` program.
 * Use NULL otherwise.
 *
 * @param f    File to read data from/write data to.
 * @param args Program's arguments.
 * @return (struct data_queue_t *) Initialized data queue structure; NULL when failed.
 */
struct data_queue_t *init_queue(FILE *f, struct args_t *args);

/**
 * Update structure content.
 * Loads data from file and store encoded data into `data_queue_t::data` array.
 * Defined and used in `dns_sender` program.
 *
 * @param q    Data queue instance.
 * @return int Number of bytes read from file.
 */
int update_data(struct data_queue_t *q);

/**
 * @brief Store encoded data from file to given buffer.
 * Defined and used in `dns_sender` program.
 *
 * @param q          Data queue instance.
 * @param buffer     Buffer to store data.
 * @param nof_bytes  Max number of bytes that function can store copy data to buffer.
 * @return int       Number of bytes that function moved into buffer;
 *                   negative value when error occured.
 */
int get_encoded_data_from_file(struct data_queue_t *q, uint8_t *buffer, size_t nof_bytes);

/**
 * @brief Decoded stored data and flush it into desired file.
 * Defined and used in `dns_receiver` program.
 *
 * @param q     Data queue instance.
 * @return int  Number of bytes written into file;
 *              negative value when error occurs.
 */
int flush_data_to_file(struct data_queue_t *q);

/**
 * @brief Load and store query data into encoded buffer,
 * Defined and used in `dns_receiver` program.
 *
 * @param q           Data queue instance.
 * @param buffer      Buffer with data stored in query.
 * @param buffer_size Size of buffer.
 * @return int        Negative value when error occurs; 0 value otherwise.
 */
int append_data_from_domain(struct data_queue_t *q, uint8_t *buffer, size_t buffer_size);

/**
 * @brief Clean up function for data queue structure.
 *
 * @param q Data queue instance.
 */
void destroy_queue(struct data_queue_t *q);

#endif // _DATA_QUEUE_H_
