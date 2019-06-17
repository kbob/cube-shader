#ifndef SRC_ARRAY_included
#define SRC_ARRAY_included

#include <stddef.h>
#include <stdio.h>

typedef struct str_array {
    size_t       count;
    size_t       alloc;
    const char **strings;
    ssize_t     *lengths;
} str_array;

extern str_array *create_str_array(void);
extern void destroy_str_array(str_array *);

extern void str_array_append(str_array *, const char *, size_t);
extern void str_array_extend(str_array *, const str_array *);

extern void print_str_array(const str_array *, FILE *);

#endif /* !SRC_ARRAY_included */
