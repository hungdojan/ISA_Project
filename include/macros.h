#ifndef _MACROS_H_
#define _MACROS_H_

#define CHUNK_SIZE_IPV4 508
#define CHUNK_SIZE_IPV6 1212

#define PACKET_SIZE 512
#define QNAME_SIZE 254
#define LABEL_SIZE 63
#define BASE_ARRAY_SIZE 128
#define DATA_SIZE       3 * BASE_ARRAY_SIZE
#define ENCODED_SIZE    4 * BASE_ARRAY_SIZE

#define DNS_PORT 12345
#define DNS_TYPE 16

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#endif // _MACROS_H_
