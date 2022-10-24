/**
 * @brief This file defines DNS header structure.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file dns_header.h
 * @date 2022-10-23
 */
#ifndef _DNS_PACKET_H_
#define _DNS_PACKET_H_

#include <stdlib.h>
#include <stdint.h>

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
    uint16_t q_count;       /**< question count */
    uint16_t ar_count;      /**< answer record count */
    uint16_t ns_count;      /**< name server record count */
    uint16_t addit_count;   /**< additional record count */
};

#endif // _DNS_PACKET_H_
