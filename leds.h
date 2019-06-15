#ifndef LEDS_included
#define LEDS_included

#include <stddef.h>
#include <stdint.h>

typedef struct LED_context LED_context;

extern LED_context *init_LEDs(size_t framebuffer_height,
                              size_t framebuffer_width);
extern void finit_LEDs(LED_context *);

extern size_t LEDs_best_buffer_size(const LED_context *);
extern size_t LEDs_best_offset(const LED_context *);
extern size_t LEDs_best_row_pitch(const LED_context *);

extern void LEDs_put_pixels(LED_context *,
                            uint16_t *pixel_buf,
                            size_t row_offset,
                            size_t row_pitch);

extern void LEDs_await_vsync(LED_context);

#endif /* !LEDS_included */
