/**
 * main.c -- Saturn LVGL Widget Showcase (Pure SGL)
 *
 * Demonstrates 19 LVGL widgets on the Sega Saturn using SGL directly.
 * Cycles through 6 showcase scenes with real-time performance monitoring.
 * Detects 4MB RAM expansion cartridge if present.
 *
 * Scenes:
 *   1. Controls   -- button, checkbox, switch, slider
 *   2. Selectors  -- dropdown, roller, spinbox, button matrix
 *   3. Text       -- textarea, span group, scrolling label
 *   4. Data       -- bar, LED, line, table
 *   5. Lists      -- tabview with list items
 *   6. Containers -- window, tileview, message box
 *
 * Navigation:
 *   D-pad + A button: interact with widgets
 *   L/R triggers: previous/next scene
 *   Auto-advances every 8 seconds
 */

#include "sgl.h"
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_port_tick.h"
#include "saturn_ramcart.h"

/* ---------- showcase config ---------- */
#define SCENE_DURATION_MS  8000
#define NUM_SCENES         6

static const char *scene_names[NUM_SCENES] = {
    "1/6  Controls",
    "2/6  Selectors",
    "3/6  Text",
    "4/6  Data",
    "5/6  Lists",
    "6/6  Containers"
};

/* state */
static int              current_scene   = -1;
static lv_obj_t        *scene_cont      = NULL;
static lv_obj_t        *title_label     = NULL;
static lv_obj_t        *cart_label      = NULL;
static lv_timer_t      *scene_timer     = NULL;
static ramcart_type_t   detected_cart   = RAMCART_NONE;
static uint16_t         prev_pad        = 0xFFFF;
static int              paused          = 0;
static int              sysmon_hidden   = 0;

/* ---------- flat style helper ---------- */

/*
 * Apply flat styling to a widget so it renders correctly
 * with LV_DRAW_SW_COMPLEX=0 (no rounded corners or shadows).
 */
static void apply_flat(lv_obj_t *obj)
{
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, LV_PART_INDICATOR);
    lv_obj_set_style_radius(obj, 0, LV_PART_KNOB);
    lv_obj_set_style_radius(obj, 0, LV_PART_ITEMS);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_width(obj, 0, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(obj, 0, LV_PART_ITEMS);
    lv_obj_set_style_outline_width(obj, 0, 0);
}

/* ---------- animation callbacks ---------- */

static void slider_anim_cb(void *var, int32_t v)
{
    lv_slider_set_value((lv_obj_t *)var, v, LV_ANIM_OFF);
}

static void bar_anim_cb(void *var, int32_t v)
{
    lv_bar_set_value((lv_obj_t *)var, v, LV_ANIM_OFF);
}

/* ---------- scene 0 : Controls ---------- */

static void scene_controls(void)
{
    /* Two buttons */
    lv_obj_t *btn1 = lv_button_create(scene_cont);
    lv_obj_set_pos(btn1, 10, 24);
    lv_obj_set_size(btn1, 140, 28);
    apply_flat(btn1);
    lv_obj_t *lbl1 = lv_label_create(btn1);
    lv_label_set_text(lbl1, "Press Me");
    lv_obj_center(lbl1);

    lv_obj_t *btn2 = lv_button_create(scene_cont);
    lv_obj_set_pos(btn2, 170, 24);
    lv_obj_set_size(btn2, 140, 28);
    apply_flat(btn2);
    lv_obj_t *lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, "OK");
    lv_obj_center(lbl2);

    /* Two checkboxes */
    lv_obj_t *cb1 = lv_checkbox_create(scene_cont);
    lv_checkbox_set_text(cb1, "Option A");
    lv_obj_set_pos(cb1, 10, 60);
    apply_flat(cb1);

    lv_obj_t *cb2 = lv_checkbox_create(scene_cont);
    lv_checkbox_set_text(cb2, "Option B");
    lv_obj_set_pos(cb2, 170, 60);
    lv_obj_add_state(cb2, LV_STATE_CHECKED);
    apply_flat(cb2);

    /* Two switches */
    lv_obj_t *sw1 = lv_switch_create(scene_cont);
    lv_obj_set_pos(sw1, 10, 88);
    apply_flat(sw1);

    lv_obj_t *sw_lbl1 = lv_label_create(scene_cont);
    lv_label_set_text(sw_lbl1, "OFF");
    lv_obj_set_pos(sw_lbl1, 65, 92);
    lv_obj_set_style_text_color(sw_lbl1, lv_color_hex(0xAAAAAA), 0);

    lv_obj_t *sw2 = lv_switch_create(scene_cont);
    lv_obj_set_pos(sw2, 170, 88);
    lv_obj_add_state(sw2, LV_STATE_CHECKED);
    apply_flat(sw2);

    lv_obj_t *sw_lbl2 = lv_label_create(scene_cont);
    lv_label_set_text(sw_lbl2, "ON");
    lv_obj_set_pos(sw_lbl2, 225, 92);
    lv_obj_set_style_text_color(sw_lbl2, lv_color_hex(0x00FF00), 0);

    /* Animated slider */
    lv_obj_t *sld_lbl = lv_label_create(scene_cont);
    lv_label_set_text(sld_lbl, "Animated:");
    lv_obj_set_pos(sld_lbl, 10, 120);
    lv_obj_set_style_text_color(sld_lbl, lv_color_hex(0xCCCCCC), 0);

    lv_obj_t *sld = lv_slider_create(scene_cont);
    lv_obj_set_size(sld, 200, 14);
    lv_obj_set_pos(sld, 100, 122);
    apply_flat(sld);
    lv_slider_set_range(sld, 0, 100);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, sld);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_playback_time(&a, 2000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, slider_anim_cb);
    lv_anim_start(&a);

    /* Interactive slider */
    lv_obj_t *sld2_lbl = lv_label_create(scene_cont);
    lv_label_set_text(sld2_lbl, "Interactive:");
    lv_obj_set_pos(sld2_lbl, 10, 148);
    lv_obj_set_style_text_color(sld2_lbl, lv_color_hex(0xCCCCCC), 0);

    lv_obj_t *sld2 = lv_slider_create(scene_cont);
    lv_obj_set_size(sld2, 200, 14);
    lv_obj_set_pos(sld2, 100, 150);
    apply_flat(sld2);
    lv_slider_set_value(sld2, 50, LV_ANIM_OFF);
}

/* ---------- scene 1 : Selectors callbacks ---------- */

static void spinbox_dec_cb(lv_event_t *e)
{
    lv_obj_t *sb = (lv_obj_t *)lv_event_get_user_data(e);
    lv_spinbox_decrement(sb);
}

static void spinbox_inc_cb(lv_event_t *e)
{
    lv_obj_t *sb = (lv_obj_t *)lv_event_get_user_data(e);
    lv_spinbox_increment(sb);
}

static void btnm_event_cb(lv_event_t *e)
{
    lv_obj_t *bm = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *result = (lv_obj_t *)lv_event_get_user_data(e);
    uint32_t id = lv_buttonmatrix_get_selected_button(bm);
    if (id == LV_BUTTONMATRIX_BUTTON_NONE) return;
    const char *txt = lv_buttonmatrix_get_button_text(bm, id);
    if (txt && result)
        lv_label_set_text_fmt(result, "Pressed: %s", txt);
}

/* ---------- scene 1 : Selectors ---------- */

static void scene_selectors(void)
{
    /* Dropdown */
    lv_obj_t *dd_lbl = lv_label_create(scene_cont);
    lv_label_set_text(dd_lbl, "Console:");
    lv_obj_set_pos(dd_lbl, 10, 26);
    lv_obj_set_style_text_color(dd_lbl, lv_color_hex(0xCCCCCC), 0);

    lv_obj_t *dd = lv_dropdown_create(scene_cont);
    lv_dropdown_set_options(dd, "Saturn\nDreamcast\nPS1\nN64");
    lv_obj_set_pos(dd, 10, 44);
    lv_obj_set_width(dd, 140);
    apply_flat(dd);

    /* Spinbox with +/- buttons */
    lv_obj_t *sb_lbl = lv_label_create(scene_cont);
    lv_label_set_text(sb_lbl, "Value:");
    lv_obj_set_pos(sb_lbl, 170, 26);
    lv_obj_set_style_text_color(sb_lbl, lv_color_hex(0xCCCCCC), 0);

    lv_obj_t *sb = lv_spinbox_create(scene_cont);
    lv_spinbox_set_range(sb, 0, 999);
    lv_spinbox_set_digit_format(sb, 3, 0);
    lv_spinbox_set_value(sb, 42);
    lv_obj_set_pos(sb, 198, 44);
    lv_obj_set_width(sb, 70);
    apply_flat(sb);

    lv_obj_t *sb_minus = lv_button_create(scene_cont);
    lv_obj_set_pos(sb_minus, 170, 44);
    lv_obj_set_size(sb_minus, 26, 26);
    apply_flat(sb_minus);
    lv_obj_t *sb_ml = lv_label_create(sb_minus);
    lv_label_set_text(sb_ml, "-");
    lv_obj_center(sb_ml);
    lv_obj_add_event_cb(sb_minus, spinbox_dec_cb, LV_EVENT_CLICKED, sb);

    lv_obj_t *sb_plus = lv_button_create(scene_cont);
    lv_obj_set_pos(sb_plus, 272, 44);
    lv_obj_set_size(sb_plus, 26, 26);
    apply_flat(sb_plus);
    lv_obj_t *sb_pl = lv_label_create(sb_plus);
    lv_label_set_text(sb_pl, "+");
    lv_obj_center(sb_pl);
    lv_obj_add_event_cb(sb_plus, spinbox_inc_cb, LV_EVENT_CLICKED, sb);

    /* Roller */
    lv_obj_t *rl_lbl = lv_label_create(scene_cont);
    lv_label_set_text(rl_lbl, "Chip:");
    lv_obj_set_pos(rl_lbl, 10, 76);
    lv_obj_set_style_text_color(rl_lbl, lv_color_hex(0xCCCCCC), 0);

    lv_obj_t *rl = lv_roller_create(scene_cont);
    lv_roller_set_options(rl, "SH-2\nVDP1\nVDP2\nSCSP\nSMPC", LV_ROLLER_MODE_NORMAL);
    lv_obj_set_pos(rl, 10, 92);
    lv_obj_set_size(rl, 120, 80);
    apply_flat(rl);

    /* Button matrix (3x3 number pad) */
    static const char *btnm_map[] = {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n",
        "7", "8", "9", ""
    };
    lv_obj_t *bm = lv_buttonmatrix_create(scene_cont);
    lv_buttonmatrix_set_map(bm, btnm_map);
    lv_obj_set_pos(bm, 160, 76);
    lv_obj_set_size(bm, 150, 80);
    apply_flat(bm);

    /* Result label shows which numpad button was pressed */
    lv_obj_t *bm_result = lv_label_create(scene_cont);
    lv_label_set_text(bm_result, "Pressed: -");
    lv_obj_set_pos(bm_result, 170, 160);
    lv_obj_set_style_text_color(bm_result, lv_color_hex(0xFFFF00), 0);
    lv_obj_add_event_cb(bm, btnm_event_cb, LV_EVENT_VALUE_CHANGED, bm_result);
}

/* ---------- scene 2 : Text callbacks ---------- */

static void ta_focus_cb(lv_event_t *e)
{
    lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
}

static void kb_hide_cb(lv_event_t *e)
{
    lv_obj_t *kb = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
}

/* ---------- scene 2 : Text ---------- */

static void scene_text(void)
{
    /* Textarea */
    lv_obj_t *ta = lv_textarea_create(scene_cont);
    lv_textarea_set_text(ta, "");
    lv_textarea_set_placeholder_text(ta, "Type here...");
    lv_obj_set_pos(ta, 10, 24);
    lv_obj_set_size(ta, 300, 32);
    apply_flat(ta);

    /* On-screen keyboard linked to the textarea (starts hidden) */
    lv_obj_t *kb = lv_keyboard_create(scene_cont);
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_set_size(kb, 320, 110);
    lv_obj_align(kb, LV_ALIGN_TOP_LEFT, 0, 58);  /* override BOTTOM_MID default */
    apply_flat(kb);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    /* Show keyboard when textarea focused, hide on OK / close */
    lv_obj_add_event_cb(ta, ta_focus_cb, LV_EVENT_FOCUSED, kb);
    lv_obj_add_event_cb(kb, kb_hide_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(kb, kb_hide_cb, LV_EVENT_CANCEL, NULL);

    /* Span group (rich colored text) */
    lv_obj_t *sg = lv_spangroup_create(scene_cont);
    lv_obj_set_pos(sg, 10, 172);
    lv_obj_set_size(sg, 300, 24);
    lv_spangroup_set_mode(sg, LV_SPAN_MODE_BREAK);
    apply_flat(sg);

    lv_span_t *sp1 = lv_spangroup_new_span(sg);
    lv_span_set_text(sp1, "SH-2 ");
    lv_style_set_text_color(lv_span_get_style(sp1), lv_color_hex(0xFF4444));

    lv_span_t *sp2 = lv_spangroup_new_span(sg);
    lv_span_set_text(sp2, "28.6 MHz ");
    lv_style_set_text_color(lv_span_get_style(sp2), lv_color_hex(0x44FF44));

    lv_span_t *sp3 = lv_spangroup_new_span(sg);
    lv_span_set_text(sp3, "LVGL v9.2 ");
    lv_style_set_text_color(lv_span_get_style(sp3), lv_color_hex(0x4488FF));

    lv_span_t *sp4 = lv_spangroup_new_span(sg);
    lv_span_set_text(sp4, "on Sega Saturn");
    lv_style_set_text_color(lv_span_get_style(sp4), lv_color_hex(0xFFFF00));

    /* Long scrolling label */
    lv_obj_t *scroll_lbl = lv_label_create(scene_cont);
    lv_label_set_long_mode(scroll_lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(scroll_lbl,
        "19 widgets running on a 28.6 MHz SH-2 with 128KB LVGL heap "
        "-- Pure SGL, no Jo Engine -- VDP2 NBG1 bitmap 320x224 RGB555   ");
    lv_obj_set_pos(scroll_lbl, 10, 198);
    lv_obj_set_width(scroll_lbl, 300);
    lv_obj_set_style_text_color(scroll_lbl, lv_color_hex(0x00FFFF), 0);
}

/* ---------- scene 3 : Data Display ---------- */

static void scene_data(void)
{
    int i;

    /* Two animated bars */
    for (i = 0; i < 2; i++) {
        lv_obj_t *bar = lv_bar_create(scene_cont);
        lv_obj_set_size(bar, 180, 14);
        lv_obj_set_pos(bar, 10, 26 + i * 22);
        apply_flat(bar);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x333333), 0);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(bar,
            lv_color_hex(i == 0 ? 0x00FF00 : 0xFF8000), LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_bar_set_range(bar, 0, 100);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, bar);
        lv_anim_set_values(&a, 0, 100);
        lv_anim_set_time(&a, 1500 + i * 500);
        lv_anim_set_playback_time(&a, 1500 + i * 500);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&a, bar_anim_cb);
        lv_anim_start(&a);
    }

    /* Two LEDs */
    lv_obj_t *led1 = lv_led_create(scene_cont);
    lv_led_set_color(led1, lv_color_hex(0x00FF00));
    lv_led_on(led1);
    lv_obj_set_pos(led1, 210, 26);
    lv_obj_set_size(led1, 24, 24);
    apply_flat(led1);

    lv_obj_t *led_lbl1 = lv_label_create(scene_cont);
    lv_label_set_text(led_lbl1, "ON");
    lv_obj_set_pos(led_lbl1, 240, 30);
    lv_obj_set_style_text_color(led_lbl1, lv_color_hex(0x00FF00), 0);

    lv_obj_t *led2 = lv_led_create(scene_cont);
    lv_led_set_color(led2, lv_color_hex(0xFF0000));
    lv_led_off(led2);
    lv_obj_set_pos(led2, 270, 26);
    lv_obj_set_size(led2, 24, 24);
    apply_flat(led2);

    lv_obj_t *led_lbl2 = lv_label_create(scene_cont);
    lv_label_set_text(led_lbl2, "OFF");
    lv_obj_set_pos(led_lbl2, 296, 30);
    lv_obj_set_style_text_color(led_lbl2, lv_color_hex(0xFF0000), 0);

    /* Line */
    static lv_point_precise_t line_pts[] = {
        {10, 0}, {60, 30}, {110, 5}, {160, 35},
        {210, 10}, {260, 25}, {300, 0}
    };
    lv_obj_t *line = lv_line_create(scene_cont);
    lv_line_set_points(line, line_pts, 7);
    lv_obj_set_pos(line, 0, 68);
    lv_obj_set_style_line_color(line, lv_color_hex(0x00FFFF), 0);
    lv_obj_set_style_line_width(line, 2, 0);

    /* Table */
    lv_obj_t *tbl = lv_table_create(scene_cont);
    lv_table_set_col_cnt(tbl, 3);
    lv_table_set_row_cnt(tbl, 4);
    lv_obj_set_pos(tbl, 10, 110);
    apply_flat(tbl);

    lv_table_set_cell_value(tbl, 0, 0, "Chip");
    lv_table_set_cell_value(tbl, 0, 1, "MHz");
    lv_table_set_cell_value(tbl, 0, 2, "Bits");
    lv_table_set_cell_value(tbl, 1, 0, "SH-2");
    lv_table_set_cell_value(tbl, 1, 1, "28.6");
    lv_table_set_cell_value(tbl, 1, 2, "32");
    lv_table_set_cell_value(tbl, 2, 0, "68EC000");
    lv_table_set_cell_value(tbl, 2, 1, "11.3");
    lv_table_set_cell_value(tbl, 2, 2, "16");
    lv_table_set_cell_value(tbl, 3, 0, "SCSP");
    lv_table_set_cell_value(tbl, 3, 1, "22.6");
    lv_table_set_cell_value(tbl, 3, 2, "32");

    lv_table_set_col_width(tbl, 0, 90);
    lv_table_set_col_width(tbl, 1, 60);
    lv_table_set_col_width(tbl, 2, 50);
}

/* ---------- scene 4 : Lists ---------- */

static void scene_lists(void)
{
    /* Tabview */
    lv_obj_t *tv = lv_tabview_create(scene_cont);
    lv_obj_set_pos(tv, 0, 20);
    lv_obj_set_size(tv, 320, 156);
    apply_flat(tv);
    lv_tabview_set_tab_bar_size(tv, 24);

    /* Tab 1: Hardware list */
    lv_obj_t *t1 = lv_tabview_add_tab(tv, "HW");
    lv_obj_t *list = lv_list_create(t1);
    lv_obj_set_size(list, lv_pct(100), lv_pct(100));
    apply_flat(list);

    lv_list_add_text(list, "Saturn Hardware");
    lv_list_add_button(list, NULL, "Master SH-2");
    lv_list_add_button(list, NULL, "Slave SH-2");
    lv_list_add_button(list, NULL, "VDP1 (sprites)");
    lv_list_add_button(list, NULL, "VDP2 (backgrounds)");
    lv_list_add_button(list, NULL, "SCSP (sound)");

    /* Tab 2: Info */
    lv_obj_t *t2 = lv_tabview_add_tab(tv, "Info");
    lv_obj_t *info = lv_label_create(t2);
    lv_label_set_text(info,
        "LVGL v9.2\n"
        "320x224 RGB555\n"
        "128KB heap\n"
        "19 widgets enabled\n"
        "LV_DRAW_SW_COMPLEX=0");
    lv_obj_set_style_text_color(info, lv_color_hex(0xCCCCCC), 0);

    /* Tab 3: Memory map */
    lv_obj_t *t3 = lv_tabview_add_tab(tv, "Mem");
    lv_obj_t *list2 = lv_list_create(t3);
    lv_obj_set_size(list2, lv_pct(100), lv_pct(100));
    apply_flat(list2);

    lv_list_add_text(list2, "Memory Map");
    lv_list_add_button(list2, NULL, "HWRAM: 1 MB");
    lv_list_add_button(list2, NULL, "LWRAM: 1 MB");
    lv_list_add_button(list2, NULL, "VDP1: 512 KB");
    lv_list_add_button(list2, NULL, "VDP2: 512 KB");
}

/* ---------- scene 5 : Containers ---------- */

static void scene_containers(void)
{
    /* Window */
    lv_obj_t *win = lv_win_create(scene_cont);
    lv_obj_set_pos(win, 5, 22);
    lv_obj_set_size(win, 148, 130);
    apply_flat(win);
    lv_win_add_title(win, "Saturn");

    lv_obj_t *win_cont = lv_win_get_content(win);
    lv_obj_set_style_pad_all(win_cont, 4, 0);
    lv_obj_t *win_lbl = lv_label_create(win_cont);
    lv_label_set_text(win_lbl,
        "LVGL Widget\n"
        "Showcase\n"
        "\n"
        "19 widgets\n"
        "Pure SGL");
    lv_obj_set_style_text_color(win_lbl, lv_color_hex(0xCCCCCC), 0);

    /* Tileview */
    lv_obj_t *tv = lv_tileview_create(scene_cont);
    lv_obj_set_pos(tv, 165, 22);
    lv_obj_set_size(tv, 150, 130);
    apply_flat(tv);

    lv_obj_t *tile1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_RIGHT);
    lv_obj_t *t1lbl = lv_label_create(tile1);
    lv_label_set_text(t1lbl, "Tile 1\nSwipe ->");
    lv_obj_set_style_text_color(t1lbl, lv_color_hex(0x00FF00), 0);
    lv_obj_center(t1lbl);

    lv_obj_t *tile2 = lv_tileview_add_tile(tv, 1, 0, LV_DIR_LEFT);
    lv_obj_t *t2lbl = lv_label_create(tile2);
    lv_label_set_text(t2lbl, "Tile 2\n<- Swipe");
    lv_obj_set_style_text_color(t2lbl, lv_color_hex(0xFFFF00), 0);
    lv_obj_center(t2lbl);

    /* Message box (inline, non-modal) */
    lv_obj_t *mbox = lv_msgbox_create(scene_cont);
    lv_msgbox_add_title(mbox, "Alert");
    lv_msgbox_add_text(mbox, "19 widgets on Saturn!");
    lv_msgbox_add_footer_button(mbox, "OK");
    lv_obj_set_pos(mbox, 30, 158);
    lv_obj_set_size(mbox, 260, 36);
    apply_flat(mbox);
}

/* ---------- scene management ---------- */

static void cleanup_scene(void)
{
    if (scene_cont) {
        lv_obj_delete(scene_cont);
        scene_cont = NULL;
    }
}

static void next_scene(lv_timer_t *t)
{
    (void)t;

    current_scene++;
    if (current_scene >= NUM_SCENES)
        current_scene = 0;

    cleanup_scene();

    /* update title */
    lv_label_set_text(title_label, scene_names[current_scene]);

    /* fresh container (transparent, full-screen) */
    lv_obj_t *scr = lv_screen_active();
    scene_cont = lv_obj_create(scr);
    lv_obj_remove_style_all(scene_cont);
    lv_obj_set_size(scene_cont, 320, 224);
    lv_obj_set_pos(scene_cont, 0, 0);
    lv_obj_set_scrollbar_mode(scene_cont, LV_SCROLLBAR_MODE_OFF);

    /* bring title + cart label to front */
    {
        lv_obj_t *parent = lv_obj_get_parent(title_label);
        int32_t last = (int32_t)lv_obj_get_child_count(parent) - 1;
        lv_obj_move_to_index(title_label, last);
        if (cart_label)
            lv_obj_move_to_index(cart_label, last);
    }

    switch (current_scene) {
        case 0: scene_controls();   break;
        case 1: scene_selectors();  break;
        case 2: scene_text();       break;
        case 3: scene_data();       break;
        case 4: scene_lists();      break;
        case 5: scene_containers(); break;
    }
}

static void prev_scene_wrap(void)
{
    current_scene -= 2;  /* next_scene() will +1 */
    if (current_scene < -1)
        current_scene = NUM_SCENES - 2;
    next_scene(NULL);
    lv_timer_reset(scene_timer);
}

static void next_scene_wrap(void)
{
    next_scene(NULL);
    lv_timer_reset(scene_timer);
}

/* ---------- entry point ---------- */

static void create_showcase_ui(void)
{
    lv_obj_t *scr = lv_screen_active();

    /* dark background */
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x001020), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    /* persistent title at top-left */
    title_label = lv_label_create(scr);
    lv_label_set_text(title_label, "Saturn LVGL Showcase");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFF00), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 4, 4);

    /* RAM cart status at top-right */
    cart_label = lv_label_create(scr);
    {
        const char *cart_text = "No cart";
        uint32_t cart_color = 0x666666;
        if (detected_cart == RAMCART_4MB) {
            cart_text = "4MB cart";
            cart_color = 0x00FF00;
        } else if (detected_cart == RAMCART_1MB) {
            cart_text = "1MB cart";
            cart_color = 0x00CC00;
        }
        lv_label_set_text(cart_label, cart_text);
        lv_obj_set_style_text_color(cart_label, lv_color_hex(cart_color), 0);
    }
    lv_obj_align(cart_label, LV_ALIGN_TOP_RIGHT, -4, 4);

    /* show sysmon overlays */
    {
        lv_sysmon_show_performance(NULL);
        lv_sysmon_show_memory(NULL);

        /* Fix backgrounds (LV_OPA_50 leaves gray ghosts with COMPLEX=0).
         * Iterate sys_layer children; only touch labels (sysmon),
         * skip lv_obj (cursor) regardless of child order. */
        lv_obj_t *sys = lv_display_get_layer_sys(NULL);
        uint32_t cnt = lv_obj_get_child_count(sys);
        uint32_t ci;
        for (ci = 0; ci < cnt; ci++) {
            lv_obj_t *child = lv_obj_get_child(sys, ci);
            if (lv_obj_check_type(child, &lv_label_class))
                lv_obj_set_style_bg_opa(child, LV_OPA_COVER, 0);
        }
    }

    /* start scene timer + run first scene immediately */
    scene_timer = lv_timer_create(next_scene, SCENE_DURATION_MS, NULL);
    next_scene(NULL);
}

int main(void)
{
    /* SGL init (320x224 NTSC, 60 fps) */
    slInitSystem(TV_320x224, NULL, 1);

    /* Detect and enable RAM expansion cartridge */
    detected_cart = ramcart_init();

    slTVOff();
    slBitMapNbg1(COL_TYPE_32768, BM_512x256, (void *)VDP2_VRAM_A0);
    slPriorityNbg1(7);
    slScrAutoDisp(NBG1ON);
    slBack1ColSet((void *)BACK_CRAM, 0x8000);
    slTVOn();

    /* LVGL init */
    lv_init();

    lv_port_tick_init();
    lv_tick_set_cb(lv_port_tick_get_ms);

    lv_port_disp_init();
    lv_port_indev_init();

    create_showcase_ui();

    /* main loop */
    while (1) {
        lv_timer_handler();

        /* Button handling (edge-detect) */
        {
            uint16_t pad = Smpc_Peripheral[0].data;
            uint16_t pressed = ~pad & prev_pad;  /* rising edge */
            prev_pad = pad;

            /* Z button: toggle pause/resume auto-advance */
            if (pressed & PER_DGT_TZ) {
                paused = !paused;
                if (paused)
                    lv_timer_pause(scene_timer);
                else
                    lv_timer_resume(scene_timer);
            }

            /* X button: toggle sysmon overlay visibility.
             * Only toggle labels (sysmon); skip lv_obj (cursor). */
            if (pressed & PER_DGT_TX) {
                lv_obj_t *sys = lv_display_get_layer_sys(NULL);
                uint32_t cnt = lv_obj_get_child_count(sys);
                uint32_t ci;
                sysmon_hidden = !sysmon_hidden;
                for (ci = 0; ci < cnt; ci++) {
                    lv_obj_t *child = lv_obj_get_child(sys, ci);
                    if (!lv_obj_check_type(child, &lv_label_class))
                        continue;
                    if (sysmon_hidden)
                        lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
                    else
                        lv_obj_remove_flag(child, LV_OBJ_FLAG_HIDDEN);
                }
            }

            /* L/R triggers: manual scene navigation */
            if (pressed & PER_DGT_TR)
                next_scene_wrap();
            if (pressed & PER_DGT_TL)
                prev_scene_wrap();
        }

        slSynch();
    }

    return 0;
}
