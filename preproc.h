#ifndef PREPROC_defined
#define PREPROC_defined

#include <stddef.h>

#include "str-array.h"

typedef struct attachment {
    const char *name;
    const char *desc;
} attachment;

typedef struct shader {
    str_array  *source;
    attachment *attachments;
    size_t      att_count;
} shader;

extern shader *read_shader(FILE *);
extern void    destroy_shader(shader *);

#endif /* !PREPROC_defined */
