/**
 * @brief This header file contains all macro variables used in this project.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file macros.h
 * @date 2022-10-23
 */
#ifndef _MACROS_H_
#define _MACROS_H_

#define CHUNK_SIZE_IPV4 508
#define CHUNK_SIZE_IPV6 1212

#define PACKET_SIZE     512
#define QNAME_SIZE      254
#define LABEL_SIZE      63
#define BASE_ARRAY_SIZE 128
#define DATA_SIZE       3 * BASE_ARRAY_SIZE
#define ENCODED_SIZE    4 * BASE_ARRAY_SIZE

#define DNS_PORT 53
#define DNS_TYPE 16

// number of double octets defined for IPv4 mapped in IPv6
#define PREFIX_SIZE 6

#define MIN(a,b) ((a) < (b) ? (a) : (b))
// variable sets receiver mode
// if value is non-zero receiver continues listening
// after finishing previous communication
#define CONTINUOUS_RUNNING 1

#endif // _MACROS_H_
