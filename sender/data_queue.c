#include "data_queue.h"
#include "error.h"

#include <string.h>     // memset
#include <stdlib.h>

struct data_queue_t {
    uint8_t data[QUEUE_SIZE];
    int index_to_read;
    FILE *f;
    unsigned int file_size;
};

struct data_queue_t *init_queue(FILE *f) {
    if (f == NULL)
        return NULL;
    struct data_queue_t *q = calloc(1, sizeof(struct data_queue_t));
    if (q == NULL)
        return 0;
    q->f = f;
    q->index_to_read = q->file_size = 0;
    memset(q->data, 0, QUEUE_SIZE);
    return q;
}

int get_encoded_data(const struct data_queue_t *q, uint8_t *buffer, size_t nof_bytes) {
    if (q == NULL || buffer == NULL)
        return 0;
    // FIXME: will be fixed later
    // data has to be encoded
    size_t len = fread(buffer, 1, nof_bytes, q->f);
    return len;
}

unsigned int get_file_size(const struct data_queue_t *q) {
    return q->file_size;
}

void destroy_queue(struct data_queue_t *q) {
    free(q);
}

/* data_queue.c */
