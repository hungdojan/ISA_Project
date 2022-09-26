#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

struct args_t {
    char *base_host;
    char *upstream_dns_ip;
    char *dst_filepath;
    char *src_filepath;
};

void init_args_t(struct args_t *args, int argc, char * const *argv);

void usage();

void destroy_args_t(struct args_t *args);

#endif // _ARGUMENTS_H_
