#ifndef _DNS_PACKET_H_
#define _DNS_PACKET_H_

#include <stdlib.h>
#include <stdint.h>

#define BUFFER_SIZE 1024
#define QNAME_SIZE   254
#define LABEL_SIZE    63

struct dns_query {
    const char *qname;
    uint16_t    qtype;
    uint16_t    qclass;
};

struct dns_answer {
    const char *rname;
    uint16_t    rtype;
    uint16_t    rclass;
    uint32_t    ttl;   /* time-to-live */
    uint16_t    resource_data_len;
    uint8_t    *resource_data;
};

struct dns_param_query {
    uint8_t rec_desire: 1;
    uint8_t truncation: 1;
    uint8_t auth_ansr : 1;
    uint8_t opcode    : 4;
    uint8_t qresponse : 1;

    uint8_t response  : 4;
    uint8_t zeros     : 3;
    uint8_t rec_avail : 1;
};

struct dns_param_response {
    uint8_t rec_desire    : 1;
    uint8_t truncated     : 1;
    uint8_t authoritative : 1;
    uint8_t opcode        : 4;
    uint8_t response      : 1;

    uint8_t reply_code    : 4;
    uint8_t non_auth_data : 1;
    uint8_t ans_auth      : 1;
    uint8_t zeros         : 1;
    uint8_t rec_avail     : 1;
};

struct dns_header {
    uint16_t id;
    union {
        struct dns_param_query query;
        struct dns_param_response response;
    } param;
    uint16_t q_count;       /* question count */
    uint16_t ar_count;      /* answer record count */
    uint16_t ns_count;      /* name server record count */
    uint16_t addit_count;   /* additional record count */
};

void print_packet(uint8_t *buffer, size_t buffer_size);

#endif // _DNS_PACKET_H_
