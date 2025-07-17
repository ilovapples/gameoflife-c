#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#include <stdint.h>

void arg_parse(int32_t argc, char **argv);
void print_usage_msg(void);
void get_dims_from_str(char *s);
void load_bin_file(char *path);

#endif /* ARG_PARSE_H */
