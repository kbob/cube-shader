#include "strbuf.h"

#include <stdlib.h>
#include <string.h>

typedef struct strbuf_impl {
    size_t size; // not counting terminating NUL
    size_t alloc;
    char  *str;
} strbuf_impl;

strbuf create_strbuf(void)
{
    strbuf sb = calloc(1, sizeof *sb);
    sb->size = 0;
    sb->alloc = 1;
    sb->str = strdup("");
    return sb;
}

void destroy_strbuf(strbuf sb)
{
    free(sb->str);
    free(sb);
}

void strbuf_append(strbuf sb, const char *str, ssize_t size)
{
    if (size < 0) {
        size = strlen(str);
    }
    size_t needed = sb->size + size + 1;
    if (needed > sb->alloc) {
        size_t n = 2 * sb->alloc;
        if (n < needed) {
            n = needed;
        }
        sb->str = realloc(sb->str, n);
    }
    memcpy(sb->str + sb->size, str, size);
    sb->size += size;
    sb->str[sb->size] = '\0';
}

const char *strbuf_contents(strbuf buf)
{
    return buf->str;
}

size_t strbuf_size(strbuf buf)
{
    return buf->size;
}

strbuf strbuf_read_file(FILE *f)
{
    
    char bytes[1024];
    size_t n;
    strbuf buf = create_strbuf();
    while ((n = fread(bytes, 1, sizeof bytes, f))) {
        strbuf_append(buf, bytes, n);
    }
    return buf;
}
