#ifndef GLPROG_included
#define GLPROG_included

#include <stdbool.h>
#include <stddef.h>

#include <GLES2/gl2.h>

#include "str-array.h"

typedef struct glprog glprog;

extern glprog     *create_glprog(const str_array *frag_shader_source);
extern void        destroy_glprog(glprog *);
extern bool        glprog_is_ok(const glprog *);

extern const char *glprog_get_info_log(const glprog *);
extern GLuint      glprog_get_program(glprog *);
extern GLuint      glprog_get_vertex_shader(glprog *);
extern GLuint      glprog_get_fragment_shader(glprog *);

#endif /* !GLPROG_included */
