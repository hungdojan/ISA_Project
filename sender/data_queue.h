#ifndef _DATA_QUEUE_H_
#define _DATA_QUEUE_H_

#include <stdio.h>
#include <stdint.h>
#define QUEUE_SIZE 256

struct data_queue_t;

/**
 * Initializes data queue.
 */
struct data_queue_t *init_queue(FILE *f);
int get_encoded_data(const struct data_queue_t *q, uint8_t *buffer, size_t nof_bytes);
unsigned int get_file_size(const struct data_queue_t *q);
void destroy_queue(struct data_queue_t *q);

#endif // _DATA_QUEUE_H_
