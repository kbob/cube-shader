#ifndef SRC_ARRAY_included
#define SRC_ARRAY_included

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct str_array {
    size_t       count;
    size_t       alloc;
    const char **strings;
    ssize_t     *lengths;
} str_array;

typedef struct str_array_cursor {
    const str_array *array;
    size_t           chunk;
    size_t           offset;
    const char      *ptr;
} str_array_cursor;

extern str_array *create_str_array(void);
extern void destroy_str_array(str_array *);

extern void str_array_append(str_array *, const char *, size_t);
extern void str_array_extend(str_array *, const str_array *);

extern void print_str_array(const str_array *, FILE *);

extern bool str_array_first_line(const str_array *,
                                 str_array_cursor *start,
                                 str_array_cursor *end);
extern bool str_array_next_line(str_array_cursor *start,
                                str_array_cursor *end);

#endif /* !SRC_ARRAY_included */
