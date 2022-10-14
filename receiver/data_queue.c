#include "data_queue.h"
#include "error.h"
#include "base64.h"
#include "macros.h"

#include <assert.h>
#include <string.h>     // memset
#include <stdlib.h>

struct data_queue_t {
    uint8_t data[ENCODED_SIZE+1]; /** Buffer to store encoded data. */
    size_t index_to_read;         /** Index of byte to be processed. */
    size_t encoded_len;           /** Number of bytes in encoded data array. */
    FILE *f;                      /** File descriptor of opened file. */
    size_t file_size;             /** Current number of bytes read. */
    size_t encoded_chunk;         /** Current number of encoded bytes. */
};

struct data_queue_t *init_queue(FILE *f) {
    if (f == NULL)
        return NULL;

    struct data_queue_t *q = calloc(1, sizeof(struct data_queue_t));
    if (q == NULL)
        return NULL;

    q->f = f;
    q->index_to_read = q->file_size = q->encoded_chunk = q->encoded_len = 0;
    // clear buffer
    // update_data(q);
    return q;
}

int update_data(struct data_queue_t *q) {
    if (q == NULL)
        return -1;
    // load data from file
    static uint8_t file_data[DATA_SIZE+1];
    int len = fread(file_data, 1, DATA_SIZE, q->f);
    q->data[len] = '\0';
    q->file_size += len;

    q->encoded_len = Base64encode((char *)q->data, (char *)file_data, len);
    q->encoded_chunk += q->encoded_len;
    q->index_to_read = 0;
    return q->encoded_len;
}

int get_encoded_data_from_file(struct data_queue_t *q, uint8_t *buffer, size_t nof_bytes) {
    if (q == NULL || buffer == NULL)
        return -1;

    int used_bytes = 0;
    size_t available_bytes = q->encoded_len - q->index_to_read-1;
    // encoded data contains not enough bytes
    if (available_bytes < nof_bytes) {
        used_bytes = available_bytes;
        memmove(buffer, q->data + q->index_to_read, used_bytes);
        nof_bytes -= used_bytes;
        available_bytes = update_data(q);
    }
    if (available_bytes == 1 && q->data[0] == '\0')
        return used_bytes;
    memmove(buffer+used_bytes, q->data+q->index_to_read, MIN(available_bytes, nof_bytes));
    q->index_to_read += MIN(available_bytes, nof_bytes);
    used_bytes += MIN(available_bytes, nof_bytes);

    return used_bytes;
}

int flush_data_to_file(struct data_queue_t *q) {
    assert(q);
    static uint8_t decoded_buffer[DATA_SIZE + 1] = { 0, };
    q->data[q->encoded_len] = '\0';
    
    int len = Base64decode((char *)decoded_buffer, (char *)q->data);
    fwrite(decoded_buffer, 1, len, q->f);
    q->file_size += len;
    return len;
}

int append_data_from_domain(struct data_queue_t *q, uint8_t *buffer, size_t buffer_size) {
    assert(q && buffer);
    int used_bytes = 0;
    // fill buffer, decode and flush decoded data into the file
    if (ENCODED_SIZE - q->encoded_len < buffer_size) {
        used_bytes = ENCODED_SIZE - q->encoded_len;
        memmove(q->data + q->encoded_len, buffer, used_bytes);
        q->encoded_len += used_bytes;
        buffer_size -= used_bytes;
        flush_data_to_file(q);
        q->encoded_len = 0;
    }
    memmove(q->data + q->encoded_len, buffer+used_bytes, buffer_size);
    q->encoded_chunk += buffer_size;
    q->encoded_len += buffer_size;
    return buffer_size;
}

size_t get_file_size(const struct data_queue_t *q) {
    assert(q);  // q parameter must not be NULL
    return q->file_size;
}

size_t get_encoded_chunk(const struct data_queue_t *q) {
    assert(q);  // q parameter must not be NULL
    return q->encoded_chunk;
}

void destroy_queue(struct data_queue_t *q) {
    free(q);
}

/* data_queue.c */
