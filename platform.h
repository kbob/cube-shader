#ifndef PLATFORM_included
#define PLATFORM_included

typedef void *platform_context;

extern void init_platform(void);
extern int platform_get_surface_width(void);
extern int platform_get_surface_height(void);
// something to bind a surface

#endif /* !PLATFORM_included */
