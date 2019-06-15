#define _GNU_SOURCE
#include "bcm.h"

#include <assert.h>
#include <stdio.h>

#include <bcm_host.h>

#define VIEWPORT_WIDTH (6 * 64 * 2)
#define VIEWPORT_HEIGHT (64 * 2)

// #define FRAMEBUFFER_WIDTH (6 * 64)
// #define FRAMEBUFFER_HEIGHT 64
#define FRAMEBUFFER_WIDTH  (800 / 2) // XXX
#define FRAMEBUFFER_HEIGHT (600 / 2) // XXX

#define LED_WIDTH (6 * 64)
#define LED_HEIGHT 64

static __thread char *last_error;

typedef struct videocore_context {
    uint32_t                   surface_width;
    uint32_t                   surface_height; 
    DISPMANX_DISPLAY_HANDLE_T  display;
    DISPMANX_ELEMENT_HANDLE_T  element;
    DISPMANX_RESOURCE_HANDLE_T screen_resource;
} videocore_context;

// Return NULL on success; return error message on failure.
// Error message may contain "%m" to reference errno.
static const char *init_videocore(videocore_context *ctx)
{
    bcm_host_init();

    uint32_t w, h;
    if (graphics_get_display_size(0, &w, &h) < 0) {
        return "can't get display 0";
    }
    ctx->surface_width = w;
    ctx->surface_height = h;
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

// Returns zero or positive on success.
static int videocore_read_pixels(videocore_context *ctx,
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

bcm_context init_bcm(void)
{
    // Initialize VideoCore.
    videocore_context *vctx = calloc(1, sizeof vctx);
    const char *err = init_videocore(vctx);
    if (err) {
        free(vctx);
        free(last_error);
        asprintf(&last_error, "init_videocore: %s\n", err);
        return NULL;
    }
    return (bcm_context)vctx;
}

void finit_bcm(bcm_context bctx)
{
    assert(0 && "This function has not been tested.");
    videocore_context *vctx = bctx;
    vc_dispmanx_resource_delete(vctx->screen_resource);
    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    if (update != DISPMANX_NO_HANDLE) {
        if (vc_dispmanx_element_remove(update, vctx->element) == 0) {
            (void)vc_dispmanx_update_submit_sync(update);
        }        
    }
    (void)vc_dispmanx_display_close(vctx->display);
    free(vctx);
}

const char *bcm_last_error(void)
{
    return last_error;
}

int bcm_get_surface_width(bcm_context bctx)
{
    return ((videocore_context *)bctx)->surface_width;
}

int bcm_get_surface_height(bcm_context bctx)
{
    return ((videocore_context *)bctx)->surface_height;
}

int bcm_get_framebuffer_width(bcm_context bctx)
{
    return FRAMEBUFFER_WIDTH;
}

int bcm_get_framebuffer_height(bcm_context bctx)
{
    return FRAMEBUFFER_HEIGHT;
}

int bcm_get_surface(bcm_context bctx)
{
    return ((videocore_context *)bctx)->element;
}

// returns zero on success
int bcm_read_pixels(bcm_context bctx,
                    uint16_t *pixels,
                    uint16_t row_pitch)
{
    videocore_context *vctx = bctx;
    return videocore_read_pixels(vctx, pixels, row_pitch);
}
