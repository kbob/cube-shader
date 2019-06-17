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
     0.000f, +0.500f,  0.0f,
    -0.433f, -0.250f,  0.0f,
    +0.433f, -0.250f,  0.0f
};
static size_t vertex_count = (&vertices)[1] - vertices;

static const GLchar fssrc[] =   // XXX
    "precision mediump float;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    // gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "    gl_FragColor = vec4(gl_FragCoord.xyz, 1.0);\n"
    "}\n"
    ;

int main(int argc, char *argv[])
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
    
    glClearColor(0.0, 0.0, 0.0, 1.0);

    glprog *prog = create_glprog(fssrc, sizeof fssrc - 1);
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

    for (int frame = 0; frame < 120; frame++) {
        glClear(GL_COLOR_BUFFER_BIT);
        for (int vp = 0; vp < 6; vp++) {
            glViewport(128 * vp, 0, 128, 128);
            glDrawArrays(GL_TRIANGLES, 0, vertex_count);
        }

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

    return 0;
}
