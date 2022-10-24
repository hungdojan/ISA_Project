/**
 * @brief This file defines `data_queue_t` structure for `dns_receiver` program.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file recv_queue.h
 * @date 2022-10-23
 */
#ifndef _RECV_QUEUE_H_
#define _RECV_QUEUE_H_

#include <stdio.h>  // FILE, size_t
#include <stdint.h> // uint8_t

#include "macros.h"
#include "data_queue.h"

struct data_queue_t {
    uint8_t data[ENCODED_SIZE+1]; /**< Buffer to store encoded data. */
    size_t index_to_read;         /**< Index of byte to be processed. */
    size_t encoded_len;           /**< Number of bytes in encoded data array. */
    FILE *f;                      /**< File descriptor of opened file. */
    size_t file_size;             /**< Current number of bytes read. */
    size_t encoded_chunk;         /**< Current number of encoded bytes. */
    char file_name[QNAME_SIZE];   /**< Output file name. */
    size_t chunkID;               /**< Chunk's ID used while logging. */
};

#endif // _RECV_QUEUE_H_
