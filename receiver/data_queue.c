/**
 * @brief This file declares `data_queue_t` functions.
 * Some functions that are not used `dns_receiver` project are not defined
 * and returns ERR_OTHER value.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file data_queue.c
 * @date 2022-10-23
 */
#include "recv_queue.h"
#include "arguments.h"
#include "error.h"
#include "base64.h"
#include "macros.h"

#include <assert.h>     // assert
#include <string.h>     // memcpy
#include <stdlib.h>     // calloc, free

struct data_queue_t *init_queue(FILE *f, struct args_t *args) {
    (void)args;
    if (f == NULL)
        return NULL;

    struct data_queue_t *q = calloc(1, sizeof(struct data_queue_t));
    if (q == NULL)
        return NULL;

    q->f = f;
    q->index_to_read = q->file_size = q->encoded_chunk = q->encoded_len = q->chunkID = 0;
    // clear buffer
    return q;
}

int update_data(struct data_queue_t *q) {
    // XXX: not implemented for this part of project
    // DO NOT USE
    (void)q;
    assert(((void)"not implemented", 0));
    return ERR_OTHER;
}

int get_encoded_data_from_file(struct data_queue_t *q, uint8_t *buffer, size_t nof_bytes) {
    // XXX: not implemented for this part of project
    // DO NOT USE
    (void)q;
    (void)buffer;
    (void)nof_bytes;
    assert(((void)"not implemented", 0));
    return ERR_OTHER;
}

int flush_data_to_file(struct data_queue_t *q) {
    if (q == NULL)
        return ERR_OTHER;

    static uint8_t decoded_buffer[DATA_SIZE + 1] = { 0, };
    q->data[q->encoded_len] = '\0';

    int len = Base64decode((char *)decoded_buffer, (char *)q->data);
    fwrite(decoded_buffer, 1, len, q->f);
    q->file_size += len;
    return len;
}

int append_data_from_domain(struct data_queue_t *q, uint8_t *buffer, size_t buffer_size) {
    if (q == NULL || buffer == NULL)
        return ERR_OTHER;

    int used_bytes = 0;
    // fill buffer, decode and flush decoded data into the file
    if (ENCODED_SIZE - q->encoded_len < buffer_size) {
        used_bytes = ENCODED_SIZE - q->encoded_len;
        memcpy(q->data + q->encoded_len, buffer, used_bytes);
        q->encoded_len += used_bytes;
        buffer_size -= used_bytes;
        flush_data_to_file(q);
        q->encoded_len = 0;
    }
    memcpy(q->data + q->encoded_len, buffer+used_bytes, buffer_size);
    q->encoded_chunk += buffer_size;
    q->encoded_len += buffer_size;
    return NO_ERR;
}

void destroy_queue(struct data_queue_t *q) {
    free(q);
}

/* data_queue.c */
