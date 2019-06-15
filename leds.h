#ifndef LEDS_included
#define LEDS_included

#include <stdint.h>

typedef void *LED_context;

extern LED_context init_LEDs(int framebuffer_height, int framebuffer_width);
extern void finit_LEDs(LED_context);

extern size_t LEDs_best_buffer_size(LED_context);
extern size_t LEDs_best_row_pitch(LED_context);
extern size_t LEDs_best_row_offset(LED_context);

extern void LEDs_put_pixels(LED_context,
                            uint16_t *pixel_buf,
                            size_t row_offset,
                            size_t row_pitch);

extern void LEDs_await_vsync(LED_context);

#endif /* !LEDS_included */
