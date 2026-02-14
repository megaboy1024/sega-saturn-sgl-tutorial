#include "lv_port_tick.h"
#include "sgl.h"

/* VBlank counter and millisecond tick */
static volatile uint32_t vblank_count = 0;

static void vblank_tick_handler(void)
{
    vblank_count++;
}

void lv_port_tick_init(void)
{
    vblank_count = 0;
    slIntFunction(vblank_tick_handler);
}

uint32_t lv_port_tick_get_ms(void)
{
    /* NTSC: 60Hz. ms = vblank_count * 1000 / 60.
     * Integer division, exact with no accumulated drift. */
    return (vblank_count * 1000u) / 60u;
}
