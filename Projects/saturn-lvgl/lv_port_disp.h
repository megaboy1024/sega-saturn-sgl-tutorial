#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

/* Initialize the LVGL display driver for Saturn VDP2 NBG1 bitmap mode.
 * Call AFTER slInitSystem() and lv_init(). */
void lv_port_disp_init(void);

#endif /* LV_PORT_DISP_H */
