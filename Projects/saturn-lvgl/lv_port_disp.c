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
 * Flush callback: transfer LVGL draw buffer to VDP2 VRAM.
 *
 * Converts RGB565 (LVGL) to Saturn RGB555 during the copy.
 *
 * LVGL RGB565:  RRRRRGGG GGGBBBBB   R[15:11] G[10:5] B[4:0]
 * Saturn:       1BBBBBGG GGGRRRRR   B[14:10] G[9:5]  R[4:0] + opaque bit
 *
 * Optimisations vs. the original per-pixel loop:
 *
 *  1. 16-bit read/write      — safe for VDP2 VRAM (B-bus, 16-bit access)
 *                               and correct for any flush width (even or odd)
 *
 *  2. Row pointer increment  — avoids y*512 multiply per row
 *
 *  3. Fewer ALU ops per pixel — (px>>1)&0x03E0 extracts G5 in one step
 *     instead of the original shift-mask-shift sequence
 */
static void saturn_flush_cb(lv_display_t *disp, const lv_area_t *area,
                            uint8_t *px_map)
{
    const uint16_t *src = (const uint16_t *)px_map;
    int32_t w = area->x2 - area->x1 + 1;
    int32_t y;

    /* VRAM destination row (16-bit pointer for safe VDP2 writes) */
    volatile uint16_t *vrow = (volatile uint16_t *)VDP2_VRAM_A0
                              + area->y1 * VDP2_STRIDE + area->x1;

    for (y = area->y1; y <= area->y2; y++) {
        const uint16_t    *s = src;
        volatile uint16_t *d = vrow;
        int32_t            n = w;

        while (n-- > 0) {
            uint16_t px = *s++;
            *d++ = 0x8000
                 | ((px & 0x001F) << 10)    /* B5 -> [14:10] */
                 | ((px >> 1) & 0x03E0)     /* G5 -> [9:5]   */
                 | (px >> 11);              /* R5 -> [4:0]   */
        }

        src  += w;             /* next source row  (exact pixel count) */
        vrow += VDP2_STRIDE;   /* next VRAM row    (512 pixels)        */
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
