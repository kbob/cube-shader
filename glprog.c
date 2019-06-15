#include "glprog.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include <GLES2/gl2.h>

struct glprog {
    GLuint     prog;
    GLuint     vshader;
    GLuint     fshader;
    bool       is_ok;
    const char *error_msg;
};

static GLuint vertex_shader = 0;

static const char *vertex_shader_source =
    "attribute vec3 vert;\n"
    "\n"
    "void main(void) {\n"
    "    gl_Position = vec4(vert, 1.0);\n"
    "}\n"
    ;

static GLuint create_shader(GLenum shader_type,
                            const char *source,
                            GLint source_size)
{
    GLuint shader = glCreateShader(shader_type);
    if (shader) {
        glShaderSource(shader, 1, &source, &source_size);
        glCompileShader(shader);
    }
    return shader;
}

static bool shader_is_ok(GLuint shader)
{
    GLint status = ~GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    return status == GL_TRUE;
}

glprog *create_glprog(const char *frag_shader_source, size_t source_size)
{
    glprog *gpg = calloc(1, sizeof *gpg);
    gpg->prog = glCreateProgram();

    if (vertex_shader == 0) {
        vertex_shader = create_shader(GL_VERTEX_SHADER,
                                      vertex_shader_source,
                                      -1); 
        assert(vertex_shader);
        assert(shader_is_ok(vertex_shader));
    }
    glAttachShader(gpg->prog, vertex_shader);

    GLuint frag_shader = create_shader(GL_FRAGMENT_SHADER,
                                       frag_shader_source,
                                       source_size);
    assert(frag_shader);
    if (!shader_is_ok(frag_shader)) {
        glDeleteProgram(gpg->prog);
        free(gpg);
        return NULL;
    }
    if (!frag_shader) {
    }

    return gpg;
}

