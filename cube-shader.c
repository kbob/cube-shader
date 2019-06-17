#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <GLES2/gl2.h>

#include "bcm.h"
#include "egl.h"
#include "glprog.h"
#include "leds.h"
#include "strbuf.h"
#include "str-array.h"

// There are four sizes here.
//
// display_height/width is the resolution of the HDMI output.
//     It is set in /boot/config.txt and read from the VideoCore driver.
//
// VIEWPORT_HEIGHT/WIDTH is the area where things are rendered on-screen.
//     It is hard coded here.
//
// FRAMEBUFFER_HEIGHT/WIDTH is an off-screen buffer in system RAM.
//     It is hard coded here.
//
// LED_HEIGHT/WIDTH is the logical size of the LED screen.
//     It is hard coded to six 64x64 panels here.

#define VIEWPORT_WIDTH (6 * 64 * 2)
#define VIEWPORT_HEIGHT (64 * 2)

#define LED_WIDTH (6 * 64)
#define LED_HEIGHT 64

typedef void uniform_setter(GLuint prog, GLuint index, void *user_data);

typedef struct attrib_desc {
    const char    *name;
    GLenum         type;
    GLint          size;
} attrib_desc;

typedef struct uniform_desc {
    const char    *name;
    GLenum         type;
    GLint          size;
    uniform_setter *setter;
} uniform_desc;

// static const attrib_desc attrib_descs[] = {
//     { "vColor", GL_FLOAT_VEC4, 1 },
//     { "vPosition", GL_FLOAT_VEC4, 4 },
// };
// static const size_t attrib_count = (&attrib_descs)[1] - attrib_descs;

// static const uniform_desc uniform_descs[] = {
//     { "m", GL_FLOAT_MAT2, 1, NULL },
//     { "iResolution",        GL_FLOAT_VEC3, 1, NULL },
//     { "iTime",              GL_FLOAT,      1, NULL },
//     { "iTimeDelta",         GL_FLOAT,      1, NULL },
//     { "iFrame",             GL_FLOAT,      1, NULL },
//     { "iChanneltime",       GL_FLOAT,      4, NULL },
//     { "iMouse",             GL_FLOAT_VEC4, 1, NULL },
//     { "iDate",              GL_FLOAT_VEC4, 1, NULL },
//     { "iSampleRate",        GL_FLOAT,      1, NULL },
//     { "iChannelResolution", GL_FLOAT_VEC3, 4, NULL },
//     { "iChannel",           GL_SAMPLER_2D, 1, NULL }
// };
// static const size_t uniform_count = (&uniform_descs)[1] - uniform_descs;

static GLfloat vertices[] = {
    -1.0, -1.0, 0.0,
    +1.0, -1.0, 0.0,
    -1.0, +1.0, 0.0,
    +1.0, +1.0, 0.0,
};
static size_t vertex_count = ((&vertices)[1] - vertices) / 3;

// static const GLchar fssrc[] =   // XXX
//     "#line 1000\n"
//     "vec3 cube_map_to_3d(vec2 pos) {\n"
//     "    vec3 p = vec3(0);\n"
//     "    if (pos.x < 128.0) {\n"
//     "        // top\n"
//     "        p = vec3(1.0 - pos.y / 128.0,\n"
//     "                 1.0,\n"
//     "                 pos.x / 128.0);\n"
//     "    } else if (pos.x < 256.0) {\n"
//     "        // back\n"
//     "        p = vec3(1.0 - pos.y / 128.0,\n"
//     "                 1.0 - (pos.x - 128.0) / 128.0,\n"
//     "                 1.0);\n"
//     "    } else if (pos.x < 384.0) {\n"
//     "        // bottom\n"
//     "        p = vec3(1.0 - pos.y / 128.0,\n"
//     "                 0.0,\n"
//     "                 1.0 - (pos.x - 256.0) / 128.0);\n"
//     "    } else if (pos.x < 512.0) {\n"
//     "        // right\n"
//     "        p = vec3(1.0,\n"
//     "                 1.0 - pos.y / 128.0,\n"
//     "                 1.0 - (pos.x - 384.0) / 128.0);\n"
//     "    } else if (pos.x < 640.0) {\n"
//     "        // front\n"
//     "        p = vec3(1.0 - (pos.x - 512.0) / 128.0,\n"
//     "                 1.0 - pos.y / 128.0,\n"
//     "                 0.0);\n"
//     "    } else if (pos.x < 768.0) {\n"
//     "        // left\n"
//     "        p = vec3(0,\n"
//     "                 1.0 - pos.y / 128.0,\n"
//     "                 (pos.x - 640.0) / 128.0);\n"
//     "    }\n"
//     "    return p - 0.5;\n"
//     "}\n"
//     "\n"
//     "void mainCube(out vec4 fragColor, in vec3 fragCoord) {\n"
//     "        fragColor.rgb = fragCoord.rgb + .5;\n"
//     "}\n"
//     "\n"
//     "#ifndef _EMULATOR\n"
//     "void mainImage(out vec4 fragColor, in vec2 fragCoord) {\n"
//     "    mainCube(fragColor, cube_map_to_3d(fragCoord));\n"
//     "}\n"
//     "#endif\n"
//     ;

static void main_loop(const str_array *fragment_shader_array)
{
    // Initialize BCM
    bcm_context bcm = init_bcm();
    if (!bcm) {
        fprintf(stderr, "bcm: %s\n", bcm_last_error());
        exit(1);
    }
    uint32_t bcm_surface = bcm_get_surface(bcm);
    uint32_t surface_width = bcm_get_surface_width(bcm);
    uint32_t surface_height = bcm_get_surface_height(bcm);
    printf("surface size %dx%d\n", surface_width, surface_height);

    // Initialize EGL.

    EGL_context *egl = init_EGL(bcm_surface, surface_width, surface_height);
    if (!egl) {
        fprintf(stderr, "init_EGL: %s\n", EGL_last_error());
        exit(1);
    }

    // Initialize LEDs.

    LEDs_context *leds = init_LEDs(LED_WIDTH, LED_HEIGHT);
    leds = leds;                // XXX

    size_t pb_width = surface_width / 2;
    size_t pb_height = surface_height / 2;
    uint16_t pixel_buffer[pb_width * pb_height];
    
    glClearColor(0.5, 0.0, 1.0, 1.0);

    glprog *prog = create_glprog(fragment_shader_array);
    if (!glprog_is_ok(prog)) {
        fprintf(stderr, "Program is not OK\n");
        fprintf(stderr, "%s\n", glprog_get_error_log(prog));
        exit(1);
    }

    // Get attrib indices
    GLint vert_index = glGetAttribLocation(glprog_get_program(prog), "vert");
    GLint vert_size;
    GLenum vert_type;
    GLchar vert_name[5] = {"1234"};
    glGetActiveAttrib(glprog_get_program(prog),
                      vert_index,
                      sizeof vert_name,
                      NULL,
                      &vert_size,
                      &vert_type,
                      vert_name);
    assert(vert_size == 1 && vert_type == GL_FLOAT_VEC3);

    // Get uniform indices

    glUseProgram(glprog_get_program(prog));
    glVertexAttribPointer(vert_index, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

    for (int frame = 0; frame < 1200 * 10; frame++) {
     glViewport(0, 0, 128 * 6, 128);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);

        // Update uniforms
        // Update attributes
        
        EGL_swap_buffers(egl);

        if (bcm_read_pixels(bcm, pixel_buffer, pb_width) != 0) {
            fprintf(stderr, "bcm_read_pixels failed\n");
            exit(1);
        }

        size_t offset = pb_width * (pb_height - LED_HEIGHT);
        LEDs_write_pixels(leds, pixel_buffer + offset, 0, pb_width);
    }
}

static const char *prog_name;

void usage(FILE *f)
{
    fprintf(f, "Use: %s [shader]\n", prog_name);
    exit(f == stderr);
}


str_array *str_array_from_strbuf(strbuf buf)
{
    str_array *sa = create_str_array();
    str_array_append(sa, strbuf_contents(buf), strbuf_size(buf));
    return sa;
}

int main(int argc, char *argv[])
{
    prog_name = argv[0];
    strbuf shader_buf = NULL;
    if (argc == 2) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Can't open ");
            perror(argv[1]);
            exit(1);
        }
        shader_buf = strbuf_read_file(f);
        fclose(f);
    } else if (argc == 1) {
        shader_buf = strbuf_read_file(stdin);
    } else {
        usage(stderr);
    }
    str_array *shader_array = str_array_from_strbuf(shader_buf);
    main_loop(shader_array);
    destroy_str_array(shader_array);
    destroy_strbuf(shader_buf);
    return 0;
}
