#ifndef LV_PORT_TICK_H
#define LV_PORT_TICK_H

#include <stdint.h>

/* Initialize the tick system (registers vblank callback) */
void lv_port_tick_init(void);

/* Returns elapsed milliseconds since init -- used as LVGL tick source */
uint32_t lv_port_tick_get_ms(void);

#endif /* LV_PORT_TICK_H */
