#include "leds.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mpsse.h"

#define FRONT_PORCH_BYTES 8
#define BACK_PORCH_BYTES 14

struct LEDs_context {
    size_t led_width;
    size_t led_height;
    size_t best_offset;
    size_t best_row_pitch;
    size_t best_buffer_size;
};

LEDs_context *init_LEDs(size_t led_width, size_t led_height)
{
    int ifnum = 0;
    const char *devstr = NULL;
    bool slow_clock = false;
    mpsse_init(ifnum, devstr, slow_clock);

    LEDs_context *ctx = calloc(1, sizeof *ctx);
    ctx->led_width = led_width;
    ctx->led_height = led_height;
    ctx->best_offset = FRONT_PORCH_BYTES;
    ctx->best_row_pitch = (FRONT_PORCH_BYTES / sizeof (uint16_t) +
                              led_width +
                              BACK_PORCH_BYTES / sizeof (uint16_t));;
    ctx->best_buffer_size = led_height * ctx->best_row_pitch;
    return ctx;
}

size_t LEDs_best_buffer_size(const LEDs_context *ctx)
{
    return ctx->best_buffer_size;
}

size_t LEDs_best_offset(const LEDs_context *ctx)
{
    return ctx->best_offset;
}

size_t LEDs_best_row_pitch(const LEDs_context *ctx)
{
    return ctx->best_row_pitch;
}

static void set_cs(int cs_b)
{
    uint8_t gpio = cs_b ? 0x28 : 0;
    uint8_t direction = 0x2b;
    mpsse_set_gpio(gpio, direction);
}


void LEDs_write_pixels(LEDs_context *ctx,
                       uint16_t *pixels,
                       size_t row_offset,
                       size_t row_pitch)
{
    const size_t ROW_OVERHEAD = 21;
    const size_t ROW_SIZE = ctx->led_width * sizeof *pixels;
    const size_t CB_SIZE = ctx->led_height * (ROW_SIZE + ROW_OVERHEAD) ;
    assert(CB_SIZE < 65536);
    uint8_t cmd_buf[CB_SIZE];
    size_t cmd_idx = 0;
    
    size_t row_count = ctx->led_height;
    for (size_t row = 0; row < row_count; row++) {
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
        size_t y = row;
        memcpy(cmd_buf + cmd_idx, &pixels[y * row_pitch], ROW_SIZE);
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
    set_cs(0);
    cmd_buf[0] = 0x04;
    cmd_buf[1] = 0x00;
    mpsse_send_spi(cmd_buf, 2);
    set_cs(1);
}

void LEDs_await_vsync(LEDs_context *ctx)
{
    uint8_t spi_buf[2];
    do {
        spi_buf[0] = 0x00;
        spi_buf[1] = 0x00;
        set_cs(0);
        mpsse_xfer_spi(spi_buf, 2);
        set_cs(1);
    } while (((spi_buf[0] | spi_buf[1]) & 0x02) != 0x02);
}
