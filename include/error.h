#ifndef _ERROR_H_
#define _ERROR_H_

#define ERR_MSG(code, ...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        return code; \
    } while (0)

enum err_code {
    NO_ERR=0,
    ERR_ARGS,           /* arguments error */
    ERR_SOCKET,         /* socket init error */
    ERR_IP_FORMAT,      /* IP format error */
    ERR_CONNECT,        /* connection error */
    ERR_NO_FILE,        /* file not exists */

    ERR_BIND,           /* unable to bind server to socket */
    ERR_OTHER=-1
};

#endif // _ERROR_H_
