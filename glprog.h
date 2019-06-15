#ifndef GLPROG_included
#define GLPROG_included

#include <stddef.h>

// create prog w/ shader
// check ok
// get error
// destroy prog

// register_uniform(name, type, size, setter)
// register_attribute(name, type, size, setter)

// prog_set_uniforms(user_data)

// prog_set_uniform(name, user_data)
// prog_set_attributes(user_data)

// get uniforms
// get attributes

typedef struct glprog glprog;

glprog *create_glprog(const char *frag_shader_source, size_t source_size);

#endif /* !GLPROG_included */
