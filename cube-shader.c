#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "bcm.h"
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

static const char *egl_error_string(const char *function, EGLint err)
{
    const char *reason;
    switch (err) {

    case EGL_NOT_INITIALIZED:
        reason = "not initialized";
        break;

    case EGL_BAD_ALLOC:
        reason = "bad alloc";
        break;

    case EGL_BAD_ATTRIBUTE:
        reason = "bad attribute";
        break;

    case EGL_BAD_CONFIG:
        reason = "bad config";
        break;

    case EGL_BAD_DISPLAY:
        reason = "bad display";
        break;

    case EGL_BAD_MATCH:
        reason = "bad match";
        break;

    case EGL_BAD_NATIVE_WINDOW:
        reason = "bad native window";
        break;

    case EGL_BAD_PARAMETER:
        reason = "bad parameter";
        break;

    default:
        reason = "unknown error";
        break;
    }
    static __thread char buf[100];
    snprintf(buf, sizeof buf, "%s: %s", function, reason);
    return buf;
}

const char *init_egl(bcm_context bctx)
{
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY) {
        return "eglGetDisplay: %m";
    }

    EGLint major = 0, minor = 0;
    if (eglInitialize(dpy, &major, &minor) == EGL_FALSE) {
        return egl_error_string("eglInitialize", eglGetError());
    }

    static const EGLint attribute_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_SAMPLES, 4,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NATIVE_RENDERABLE, EGL_TRUE,
        EGL_NONE
    };
    EGLConfig config = 0;
    EGLint numconfig = 0;
    if (!eglChooseConfig(dpy, attribute_list, &config, 1, &numconfig)) {
        return egl_error_string("eglChooseConfig", eglGetError());
    }

    static const EGLint context_attributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    EGLContext context =
        eglCreateContext(dpy, config, EGL_NO_CONTEXT, context_attributes);
    if (context == EGL_NO_CONTEXT) {
        return "could not create EGL context";
    }

    int native_window[] = {
        bcm_get_surface(bctx),
        bcm_get_surface_width(bctx),
        bcm_get_surface_height(bctx),
    };
    EGLSurface surface =
        eglCreateWindowSurface(dpy, config, native_window, NULL);
    if (surface == EGL_NO_SURFACE) {
        return egl_error_string("eglCreateWindowSurface", eglGetError());
    }

    if (eglMakeCurrent(dpy, surface, surface, context) == EGL_FALSE)
        perror("eglMakeCurrent"), exit(1);

    // Don't wait for vsync.
    eglSwapInterval(dpy, 0);

    return NULL;
}

void init_gl(void)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void gl_destroy_program(GLuint prog)
{
    // destroy attachments
    // enumerate shaders
    // detach shaders
    // destroy fragment shader
    // destroy program
}

void gl_render(GLuint prog);

static const GLchar fssrc[] =
    "precision mediump float;\n"
    "varying vec4 color;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    // gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "    gl_FragColor = color;\n"
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
    int surface_width = bcm_get_surface_width(bcm);
    int surface_height = bcm_get_surface_height(bcm);
    printf("surface size %dx%d\n", surface_width, surface_height);

    // Initialize LEDs.

    init_LEDs(LED_WIDTH, LED_HEIGHT);

    // Initialize EGL.

    const char *err = init_egl(bcm);
    if (err) {
        fprintf(stderr, "init_egl: %s\n", err);
        exit(1);
    }

    glClearColor(0., 0., 0., 1.);

    glprog *prog = create_glprog(fssrc, sizeof fssrc - 1);
    glUseProgram(glprog_get_program(prog));

    size_t pb_width = surface_width / 2;
    size_t pb_height = surface_height / 2;
    uint16_t pixel_buffer[pb_width * pb_height];
    
    // Get attrib indices
    // Get uniform indices

    for (int frame = 0; frame < 120; frame++) {
        glClear(GL_COLOR_BUFFER_BIT);
        // Use program.
        // Update uniforms
        // Update attributes
        
    }

    return 0;
}
