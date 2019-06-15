#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//#include <bcm_host.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "bcm.h"
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

// typedef struct videocore_context {
//     uint32_t                   display_width;
//     uint32_t                   display_height; 
//     DISPMANX_DISPLAY_HANDLE_T  display;
//     DISPMANX_ELEMENT_HANDLE_T  element;
//     DISPMANX_RESOURCE_HANDLE_T screen_resource;
// } videocore_context;

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

// static const char *vertex_shader_source =
//     "attribute vec3 vert;\n"
//     "\n"
//     "void main(void) {\n"
//     "    gl_Position = vec4(vert, 1.0);\n"
//     "}\n"
//     ;

#if 0
// Return NULL on success; return error message on failure.
// Error message may contain "%m" to reference errno.
const char *init_videocore(videocore_context *ctx)
{
    bcm_host_init();

    uint32_t w, h;
    if (graphics_get_display_size(0, &w, &h) < 0) {
        return "videocore: can't get display 0";
    }
    ctx->display_width = w;
    ctx->display_height = h;
    {                           // XXX XXX
        assert(w == 800 && h == 600);
    }

    ctx->display = vc_dispmanx_display_open(0);
    if (ctx->display == DISPMANX_NO_HANDLE) {
        return "vc_dispmanx_display_open failed";
    }

    DISPMANX_UPDATE_HANDLE_T dispman_update = vc_dispmanx_update_start(0);
    if (dispman_update == DISPMANX_NO_HANDLE) {
        return "vc_dispmanx_update_start failed";
    }

    // I don't know what this does.  But it seems to work.
    const int32_t layer = 0;
    VC_RECT_T src_rect = {
        .x             = 0,
        .y             = 0,
        .width         = VIEWPORT_WIDTH << 16,
        .height        = VIEWPORT_HEIGHT << 16,
    };
    VC_RECT_T dst_rect = {
        .x             = 0,
        .y             = h - VIEWPORT_HEIGHT,
        .width         = VIEWPORT_WIDTH,
        .height        = VIEWPORT_HEIGHT,
    };
    VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_PREMULT, 0, 0 };
    ctx->element = vc_dispmanx_element_add(dispman_update,
                                           ctx->display,
                                           layer, &dst_rect,
                                           0, &src_rect,
                                           DISPMANX_PROTECTION_NONE,
                                           &alpha, 0, 0);
    if (ctx->element == DISPMANX_NO_HANDLE) {
        return "vc_dispmanx_element_add failed";
    }

    if (vc_dispmanx_update_submit_sync(dispman_update) != 0) {
        return "vc_dispmanx_update_submit_sync failed";
    }

    // from fbx2.c
    uint32_t native_image_handle = 0;
    ctx->screen_resource =
        vc_dispmanx_resource_create(VC_IMAGE_RGB565,
                                    FRAMEBUFFER_WIDTH,
                                    FRAMEBUFFER_HEIGHT,
                                    &native_image_handle);

    return NULL;
}
#endif

#if 0
// Returns zero or positive on success.
int videocore_read_pixels(videocore_context *ctx,
                           uint16_t pixel_buf[],
                           size_t word_pitch)
{

    VC_RECT_T rect;
    vc_dispmanx_rect_set(&rect, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    int r = vc_dispmanx_snapshot(ctx->display, ctx->screen_resource, 0);
    if (r >= 0) {
        r = vc_dispmanx_resource_read_data(ctx->screen_resource,
                                           &rect,
                                           pixel_buf,
                                           word_pitch * sizeof *pixel_buf);
    }
    return r;
}
#endif

#if 0
void init_mpsse(void)
{
    int ifnum = 0;
    const char *devstr = NULL;
    bool slow_clock = false;
    mpsse_init(ifnum, devstr, slow_clock);
}

static void mpsse_set_cs(int cs_b)
{
    uint8_t gpio = cs_b ? 0x28 : 0;
    uint8_t direction = 0x2b;
    mpsse_set_gpio(gpio, direction);
}

void mpsse_send_pixels(uint16_t pixel_buf[], size_t word_pitch)
{
    const size_t ROW_OVERHEAD = 21;
    const size_t ROW_SIZE = LED_WIDTH * sizeof *pixel_buf;
    const size_t CB_SIZE = LED_HEIGHT * (ROW_SIZE + ROW_OVERHEAD) ;
    uint8_t cmd_buf[CB_SIZE];
    size_t cmd_idx = 0;
    
    for (size_t row = 0; row < LED_HEIGHT; row++) {
        // Set CS low
        cmd_buf[cmd_idx++] = 0x80; // MC_SETB_LOW
        cmd_buf[cmd_idx++] = 0x00; // gpio
        cmd_buf[cmd_idx++] = 0x2b; // dir

        // SPI packet header
        cmd_buf[cmd_idx++] = 0x11;
        cmd_buf[cmd_idx++] = (ROW_SIZE + 1 - 1) & 0xFF;
        cmd_buf[cmd_idx++] = (ROW_SIZE + 1 - 1) >> 8;

        // SPI payload
        cmd_buf[cmd_idx++] = 0x80;
        size_t y = FRAMEBUFFER_HEIGHT - VIEWPORT_HEIGHT + row;
        memcpy(cmd_buf + cmd_idx, &pixel_buf[y * word_pitch], ROW_SIZE);
        cmd_idx += ROW_SIZE;

        // Set CS high
        cmd_buf[cmd_idx++] = 0x80; // MZC_SETB_LOW
        cmd_buf[cmd_idx++] = 0x28; // gpio
        cmd_buf[cmd_idx++] = 0x2b;  // dir

        // Set CS low
        cmd_buf[cmd_idx++] = 0x80; // MC_SETB_LOW
        cmd_buf[cmd_idx++] = 0x00; // gpio
        cmd_buf[cmd_idx++] = 0x2b; // dir

        // SPI header
        cmd_buf[cmd_idx++] = 0x11; // MC_DATA_OUT | MC_DATA_OCN
        cmd_buf[cmd_idx++] = 2-1;
        cmd_buf[cmd_idx++] = 0;

        // SPI payload
        cmd_buf[cmd_idx++] = 0x03;
        cmd_buf[cmd_idx++] = row;

        // Set CS high
        cmd_buf[cmd_idx++] = 0x80; // MC_SETB_LOW
        cmd_buf[cmd_idx++] = 0x28; // gpio
        cmd_buf[cmd_idx++] = 0x2b; // dir
    }
    assert(cmd_idx == CB_SIZE);
    mpsse_send_raw(cmd_buf, cmd_idx);

    // Swap
    mpsse_set_cs(0);
    cmd_buf[0] = 0x04;
    cmd_buf[1] = 0x00;
    mpsse_send_spi(cmd_buf, 2);
    mpsse_set_cs(1);
}

void mpsse_await_vsync(void)
{
    uint8_t spi_buf[2];
    do {
        spi_buf[0] = 0x00;
        spi_buf[1] = 0x00;
        mpsse_set_cs(0);
        mpsse_xfer_spi(spi_buf, 2);
        mpsse_set_cs(1);
        //printf("%d\n", cmd_buf[0] | cmd_buf[1]);
    } while (((spi_buf[0] | spi_buf[1]) & 0x02) != 0x02);
}
#endif

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
        // EGL_DEPTH_SIZE, 8,      // XXX
        // EGL_STENCIL_SIZE, 8,    // XXX
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

// GLuint gl_create_program(const char *fssrc, size_t fs_size)
// {
// }

void gl_destroy_program(GLuint prog)
{
    // destroy attachments
    // enumerate shaders
    // detach shaders
    // destroy fragment shader
    // destroy program
}

void gl_render(GLuint prog);

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
    printf("display size %dx%d\n", surface_width, surface_height);

    // Initialize MPSSE.
    
    init_LEDs(LED_WIDTH, LED_HEIGHT);

    // Initialize EGL.

    // err = init_egl(&vc_ctx);
    // if (err) {
    //     fprintf(stderr, "init_egl: %s\n", err);
    //     exit(1);
    // }

    return 0;
}
