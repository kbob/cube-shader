#ifndef LEDS_included
#define LEDS_included

#include <stddef.h>
#include <stdint.h>

typedef struct LEDs_context LEDs_context;

extern LEDs_context *init_LEDs(size_t framebuffer_height,
                              size_t framebuffer_width);
extern void finit_LEDs(LEDs_context *);

extern size_t LEDs_best_buffer_size(const LEDs_context *);
extern size_t LEDs_best_offset(const LEDs_context *);
extern size_t LEDs_best_row_pitch(const LEDs_context *);

extern void LEDs_write_pixels(LEDs_context *,
                              uint16_t *pixel_buf,
                              size_t row_offset,
                              size_t row_pitch);

extern void LEDs_await_vsync(LEDs_context *);

#endif /* !LEDS_included */
