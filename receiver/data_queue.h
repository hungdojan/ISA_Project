#ifndef _DATA_QUEUE_H_
#define _DATA_QUEUE_H_

#include <stdio.h>
#include <stdint.h>

struct data_queue_t;

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

int flush_data_to_file(struct data_queue_t *q);

int append_data_from_domain(struct data_queue_t *q, uint8_t *buffer, size_t buffer_size);

/**
 * @brief Return number of bytes read from file.
 * 
 * @param q Data queue instance.
 * @return unsigned int Number of bytes read from file.
 */
size_t get_file_size(const struct data_queue_t *q);

/**
 * @brief Get data chunk value.
 *
 * @param q Data queue instance.
 * @return size_t Current encoded chunk value.
 */
size_t get_encoded_chunk(const struct data_queue_t *q);

/**
 * @brief Clean up function for data queue structure.
 * 
 * @param q Data queue instance.
 */
void destroy_queue(struct data_queue_t *q);

#endif // _DATA_QUEUE_H_
