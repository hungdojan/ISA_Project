/**
 * @brief This file defines program's error codes.
 *
 * This source code serves as submission
 * for a project of class ISA at FIT, BUT 2022/23.
 *
 * @author Hung Do (xdohun00@fit.vutbr.cz)
 * @file error.h
 * @date 2022-10-23
 */
#ifndef _ERROR_H_
#define _ERROR_H_

#define ERR_MSG(code, ...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        return code; \
    } while (0)

/** Enumeration of error codes. */
enum err_code {
    NO_ERR=0,
    ERR_ARGS,              /* arguments error */
    ERR_SOCKET,            /* socket init error */
    ERR_IP_FORMAT,         /* IP format error */
    ERR_CONNECT,           /* connection error */
    ERR_NO_FILE,           /* file not exists */

    ERR_BIND,              /* unable to bind server to socket */
    ERR_OLD_PACKET=-1,      /* received old packet */
    ERR_FNAME_TOO_BIG=-2,  /* file name too big */
    ERR_OTHER=-3,          /* internal error or connection loss */
};

#endif // _ERROR_H_
