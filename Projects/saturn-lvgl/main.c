/**
 * main.c -- Saturn LVGL Hello World (Pure SGL, no Jo Engine)
 *
 * Demonstrates LVGL running on the Sega Saturn via SGL,
 * rendering to VDP2 NBG1 in bitmap mode.
 */

#include "sgl.h"
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_port_tick.h"

static int click_count = 0;

static void btn_click_cb(lv_event_t *e)
{
    lv_obj_t *btn_label = lv_event_get_user_data(e);
    click_count++;
    static char buf[32];
    lv_snprintf(buf, sizeof(buf), "Clicked: %d", click_count);
    lv_label_set_text(btn_label, buf);
}

/* Create a simple demo UI */
static void create_ui(void)
{
    lv_obj_t *scr = lv_screen_active();

    /* Dark background -- disable scrollbar (its theme style uses LV_RADIUS_CIRCLE
     * which can't render with LV_DRAW_SW_COMPLEX disabled) */
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003050), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    /* Title label */
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Hello Saturn!\nLVGL on SGL");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -40);

    /* Button with click handler (radius=0 required: LV_DRAW_SW_COMPLEX disabled) */
    lv_obj_t *btn = lv_button_create(scr);
    lv_obj_set_size(btn, 120, 40);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_radius(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Press A");
    lv_obj_center(btn_label);

    lv_obj_add_event_cb(btn, btn_click_cb, LV_EVENT_CLICKED, btn_label);
}

int main(void)
{
    /* Initialize SGL (320x224, NTSC, 60fps) */
    slInitSystem(TV_320x224, NULL, 1);

    /* Configure VDP2 NBG1 as bitmap framebuffer */
    slTVOff();
    slBitMapNbg1(COL_TYPE_32768, BM_512x256, (void *)VDP2_VRAM_A0);
    slPriorityNbg1(7);
    slScrAutoDisp(NBG1ON);
    slBack1ColSet((void *)BACK_CRAM, 0x8000);
    slTVOn();

    /* Initialize LVGL */
    lv_init();

    lv_port_tick_init();
    lv_tick_set_cb(lv_port_tick_get_ms);

    lv_port_disp_init();
    lv_port_indev_init();

    create_ui();

    /* Main loop */
    while(1)
    {
        lv_timer_handler();
        slSynch();
    }

    return 0;
}
