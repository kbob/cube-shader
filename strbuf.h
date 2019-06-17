#ifndef STRBUF_included
#define STRBUF_included

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

typedef struct strbuf_impl *strbuf;

extern strbuf create_strbuf(void);
extern void destroy_strbuf(strbuf);

// size = -1 for NUL-terminated
extern void strbuf_append(strbuf, const char *str, ssize_t size);

extern const char *strbuf_contents(const strbuf);
extern size_t strbuf_size(const strbuf);

extern strbuf strbuf_read_file(FILE *f);

#endif /* !STRBUF_included */
