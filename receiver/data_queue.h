#ifndef _DATA_QUEUE_H_
#define _DATA_QUEUE_H_

#include <stdio.h>
#include <stdint.h>

#include "macros.h"

struct data_queue_t {
    uint8_t data[ENCODED_SIZE+1]; /** Buffer to store encoded data. */
    size_t index_to_read;         /** Index of byte to be processed. */
    size_t encoded_len;           /** Number of bytes in encoded data array. */
    FILE *f;                      /** File descriptor of opened file. */
    size_t file_size;             /** Current number of bytes read. */
    size_t encoded_chunk;         /** Current number of encoded bytes. */
    char file_name[QNAME_SIZE];   /** Output file name. */
};

/**
 * Initializes data queue.
 * 
 * @param f File to read data from.
 * @returns Initialized data queue structure; NULL when failed.
 */
struct data_queue_t *init_queue(FILE *f);

/**
 * Update structure content.
 * Loads data from file and store encoded data into data_queue_t::data array.
 *
 * @param q    Data queue instance.
 * @return int Number of bytes read from file.
 */
int update_data(struct data_queue_t *q);

/**
 * @brief Store encoded data from file to given buffer.
 * 
 * @param q          Data queue instance.
 * @param buffer     Buffer to store data.
 * @param nof_bytes  Max number of bytes that function can store copy data to buffer.
 * @return int       Number of bytes that function moved into buffer; -1 when error occured.
 */
int get_encoded_data_from_file(struct data_queue_t *q, uint8_t *buffer, size_t nof_bytes);

/**
 * @brief Decoded stored data and flush it into desired file.
 * 
 * @param q     Data queue instance.
 * @return int  Number of bytes written into file.
 */
int flush_data_to_file(struct data_queue_t *q);

/**
 * @brief Load and store query data into encoded buffer,
 * 'q' and 'buffer' variables cannot be NULL.
 * 
 * @param q           Data queue instance.
 * @param buffer      Buffer with data stored in query.
 * @param buffer_size Size of buffer.
 */
int append_data_from_domain(struct data_queue_t *q, uint8_t *buffer, size_t buffer_size);

/**
 * @brief Clean up function for data queue structure.
 * 
 * @param q Data queue instance.
 */
void destroy_queue(struct data_queue_t *q);

#endif // _DATA_QUEUE_H_
