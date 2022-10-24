/**
 * @brief This file defines `data_queue_t` structure for `dns_sender` program.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file send_queue.h
 * @date 2022-10-23
 */
#ifndef _SEND_QUEUE_H_
#define _SEND_QUEUE_H_

#include <stdio.h>   // FILE, size_t
#include <stdint.h>  // uint8_t

#include "arguments.h"
#include "macros.h"
#include "data_queue.h"

struct data_queue_t {
    uint8_t data[ENCODED_SIZE+1]; /**< Buffer to store encoded data. */
    size_t index_to_read;         /**< Index of byte to be processed. */
    size_t encoded_len;           /**< Number of bytes in encoded data array. */
    FILE *f;                      /**< File descriptor of opened file. */
    size_t file_size;             /**< Current number of bytes read. */
    size_t encoded_chunk;         /**< Current number of encoded bytes. */
    size_t raw_encoded_len;       /**< Length of encoded data in payload. */
    struct args_t *args;          /**< Program's arguments values. */
    size_t chunkID;               /**< Chunk's ID used while logging. */
};

#endif // _SEND_QUEUE_H_
