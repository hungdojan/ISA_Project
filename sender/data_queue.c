/**
 * @brief This file declares `data_queue_t` functions.
 * Some functions that are not used `dns_sender` project are not defined
 * and returns ERR_OTHER value.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file data_queue.c
 * @date 2022-10-23
 */
#include "send_queue.h"
#include "error.h"
#include "base64.h"

#include <assert.h>     // assert
#include <string.h>     // memcpy
#include <stdlib.h>     // calloc, free

struct data_queue_t *init_queue(FILE *f, struct args_t *args) {
    if (f == NULL || args == NULL)
        return NULL;

    struct data_queue_t *q = calloc(1, sizeof(struct data_queue_t));
    if (q == NULL)
        return NULL;

    q->f = f;
    q->index_to_read = q->file_size = q->encoded_chunk =
                       q->encoded_len = q->raw_encoded_len = q->chunkID = 0;
    q->args = args;
    // clear buffer
    update_data(q);
    return q;
}

int update_data(struct data_queue_t *q) {
    if (q == NULL)
        return ERR_OTHER;

    // load data from file
    static uint8_t file_data[DATA_SIZE+1] = { 0, };
    int len = fread(file_data, 1, DATA_SIZE, q->f);
    q->data[len] = '\0';
    q->file_size += len;

    q->encoded_len = Base64encode((char *)q->data, (char *)file_data, len);
    // q->encoded_chunk += q->encoded_len;
    q->index_to_read = 0;
    return q->encoded_len;
}

int get_encoded_data_from_file(struct data_queue_t *q, uint8_t *buffer, size_t nof_bytes) {
    if (q == NULL && buffer == NULL)
        return ERR_OTHER;

    int used_bytes = 0;
    size_t available_bytes = q->encoded_len - q->index_to_read-1;
    // encoded data contains not enough bytes
    if (available_bytes < nof_bytes) {
        used_bytes = available_bytes;
        memcpy(buffer, q->data + q->index_to_read, used_bytes);
        nof_bytes -= used_bytes;
        available_bytes = update_data(q);
    }
    // stop when EOF is reached
    if (available_bytes == 1 && q->data[0] == '\0')
        return used_bytes;

    memcpy(buffer+used_bytes, q->data+q->index_to_read, MIN(available_bytes, nof_bytes));
    q->index_to_read += MIN(available_bytes, nof_bytes);
    used_bytes += MIN(available_bytes, nof_bytes);

    return used_bytes;
}

int flush_data_to_file(struct data_queue_t *q) {
    // XXX: not implemented for this part of project
    // DO NOT USE
    (void)q;
    assert(((void)"not implemented", 0));
    return ERR_OTHER;
}

int append_data_from_domain(struct data_queue_t *q, uint8_t *buffer, size_t buffer_size) {
    // XXX: not implemented for this part of project
    // DO NOT USE
    (void)q;
    (void)buffer;
    (void)buffer_size;
    assert(((void)"not implemented", 0));
    return ERR_OTHER;
}

void destroy_queue(struct data_queue_t *q) {
    free(q);
}

/* data_queue.c */
