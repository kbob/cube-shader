#include "glprog.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLES2/gl2.h>

struct glprog {
    GLuint  prog;
    GLuint  frag_shader;
    bool    is_ok;
    char   *error_log;
};

static GLuint vertex_shader = 0;

static const char *vertex_shader_source =
    "attribute vec3 vert;\n"
    "\n"
    "void main(void) {\n"
    "    gl_Position = vec4(vert, 1.0);\n"
    "}\n"
    ;

static const char frag_shader_prologue[] =
    "uniform vec3 iResolution;\n"
    "uniform float iTime;\n"
    "uniform float iTimeDelta;\n"
    "uniform float iFrame;\n"
    "uniform float iChannelTime[4];\n"
    "uniform vec4 iMouse;\n"
    "uniform vec4 iDate;\n"
    "uniform float iSampleRate;\n"
    "uniform vec3 iChannelResolution[4];\n"
    ;

static const char frag_shader_epilogue[] =
    "void main() {\n"
    "    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "    mainImage(gl_FragColor, gl_FragCoord.xy);\n"
    "    gl_FragColor.a = 1.0;\n"
    "}\n"
    ;

static str_array *build_frag_source_array(const str_array *source)
{
    str_array *sources = create_str_array();
    str_array_append(sources,
                     frag_shader_prologue,
                     sizeof frag_shader_prologue - 1);
    str_array_extend(sources, source);
    str_array_append(sources,
                     frag_shader_epilogue,
                     sizeof frag_shader_epilogue - 1);
    return sources;
}

static GLuint create_shader(GLenum shader_type,
                            size_t source_count,
                            const char **source_strings,
                            const GLint *source_lengths)
{
    GLuint shader = glCreateShader(shader_type);
    if (shader) {
        glShaderSource(shader, source_count, source_strings, source_lengths);
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

glprog *create_glprog(const str_array *fragment_shader_source)
{
    glprog *gpg = calloc(1, sizeof *gpg);
    gpg->is_ok = true;
    gpg->prog = glCreateProgram();
    if (!gpg->prog) {
        gpg->is_ok = false;
        gpg->error_log = strdup("glCreateProgram failed");
        return gpg;
    }

    if (vertex_shader == 0) {
        GLint lengths[1] = {-1};
        vertex_shader = create_shader(GL_VERTEX_SHADER,
                                      1,
                                      &vertex_shader_source,
                                      lengths); 
        assert(vertex_shader);
        assert(shader_is_ok(vertex_shader));
    }

    str_array *frag_sources = build_frag_source_array(fragment_shader_source);
    printf("----------\n");
    print_str_array(frag_sources, stdout);
    printf("----------\n");

    gpg->frag_shader = create_shader(GL_FRAGMENT_SHADER,
                                     frag_sources->count,
                                     frag_sources->strings,
                                     frag_sources->lengths);
    destroy_str_array(frag_sources);

    assert(gpg->frag_shader);
    if (!shader_is_ok(gpg->frag_shader)) {
        gpg->is_ok = false;
        GLint len;
        glGetShaderiv(gpg->frag_shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            gpg->error_log = malloc(len);
            glGetShaderInfoLog(gpg->frag_shader, len, NULL, gpg->error_log);
        }
        return gpg;
    }

    // Link program.

    glAttachShader(gpg->prog, vertex_shader);
    glAttachShader(gpg->prog, gpg->frag_shader);
    glBindAttribLocation(gpg->prog, 0, "vert");
    glLinkProgram(gpg->prog);
    GLint link_status;
    glGetProgramiv(gpg->prog, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
        gpg->is_ok = false;
        GLint len;
        glGetProgramiv(gpg->prog, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            gpg->error_log = malloc(len);
            glGetProgramInfoLog(gpg->prog, len, NULL, gpg->error_log);
        }
        return gpg;
    }

    // Validate program.

    glValidateProgram(gpg->prog);
    GLint valid_status;
    glGetProgramiv(gpg->prog, GL_VALIDATE_STATUS, &valid_status);
    if (valid_status != GL_TRUE) {
        gpg->is_ok = false;
        GLint len;
        glGetProgramiv(gpg->prog, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            gpg->error_log = malloc(len);
            glGetProgramInfoLog(gpg->prog, len, NULL, gpg->error_log);
        }
        return gpg;
    }
    
// //XXX
// #include <stdio.h>
//     // Look at attributes.

//     GLuint prog = gpg->prog;
//     GLint attr_count;
//     GLint max_attr_len;
//     glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, &attr_count);
//     glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_attr_len);
//     printf("%d attribute%s\n", attr_count, "s" + (attr_count == 1));
//     for (int i = 0; i < attr_count; i++) {
//         char name[max_attr_len];
//         GLint size;
//         GLenum type;
//         glGetActiveAttrib(prog, i, sizeof name, NULL, &size, &type, name);
//         printf("    %d %#x %s\n", size, type, name);
//     }
// //XXX
    return gpg;
}

void destroy_glprog(glprog *gpg)
{
    if (gpg->frag_shader)
        glDeleteShader(gpg->frag_shader);
    if (gpg->prog)
        glDeleteProgram(gpg->prog);
    free(gpg->error_log);
    free(gpg);
}

bool glprog_is_ok(const glprog *gpg)
{
    return gpg->is_ok;
}

const char *glprog_get_error_log(const glprog *gpg)
{
    return gpg->error_log;
}

GLuint glprog_get_program(glprog *gpg)
{
    return gpg->prog;
}

GLuint glprog_get_vertex_shader(glprog *gpg)
{
    return vertex_shader;
}

GLuint glprog_get_fragment_shader(glprog *gpg)
{
    return gpg->frag_shader;
}

