#include "lv_port_disp.h"
#include "sgl.h"
#include "lvgl/lvgl.h"

/* Display resolution */
#define DISP_HOR_RES 320
#define DISP_VER_RES 224

/* VDP2 bitmap stride (pixels per row in VRAM, not visible screen width) */
#define VDP2_STRIDE  512

/* Draw buffer: 24 lines of partial rendering, double-buffered */
#define DRAW_BUF_LINES 24
#define DRAW_BUF_SIZE  (DISP_HOR_RES * DRAW_BUF_LINES * sizeof(uint16_t))

static uint8_t draw_buf1[DRAW_BUF_SIZE] __attribute__((aligned(4)));
static uint8_t draw_buf2[DRAW_BUF_SIZE] __attribute__((aligned(4)));

/*
 * Convert RGB565 (LVGL) to Saturn RGB555.
 *
 * LVGL RGB565 (big-endian SH-2):  RRRRRGGG GGGBBBBB
 *   R = bits[15:11], G = bits[10:5], B = bits[4:0]
 *
 * Saturn RGB555:  1BBBBBGG GGGRRRRR
 *   MSB=1 (opaque), B = bits[14:10], G = bits[9:5], R = bits[4:0]
 */
static inline uint16_t rgb565_to_saturn(uint16_t rgb565)
{
    uint16_t r5 = (rgb565 >> 11) & 0x1F;
    uint16_t g5 = (rgb565 >> 6)  & 0x1F;  /* take top 5 of 6 green bits */
    uint16_t b5 =  rgb565        & 0x1F;
    return 0x8000 | (b5 << 10) | (g5 << 5) | r5;
}

/* Flush callback: transfer LVGL draw buffer to VDP2 VRAM */
static void saturn_flush_cb(lv_display_t *disp, const lv_area_t *area,
                            uint8_t *px_map)
{
    uint16_t *src = (uint16_t *)px_map;
    volatile uint16_t *vram = (volatile uint16_t *)VDP2_VRAM_A0;

    int32_t x, y;
    for (y = area->y1; y <= area->y2; y++)
    {
        volatile uint16_t *row = vram + (y * VDP2_STRIDE);
        for (x = area->x1; x <= area->x2; x++)
        {
            row[x] = rgb565_to_saturn(*src);
            src++;
        }
    }

    lv_display_flush_ready(disp);
}

void lv_port_disp_init(void)
{
    lv_display_t *disp = lv_display_create(DISP_HOR_RES, DISP_VER_RES);

    lv_display_set_buffers(disp, draw_buf1, draw_buf2,
                           DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_display_set_flush_cb(disp, saturn_flush_cb);
}
