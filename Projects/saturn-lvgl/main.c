/**
 * main.c -- Saturn LVGL Benchmark (Pure SGL)
 *
 * Custom performance benchmark for LVGL on Sega Saturn.
 * Cycles through 5 test scenes using available widgets
 * (label, button, bar) within the Saturn's 48KB heap.
 *
 * Scenes:
 *   1. Fill   -- 16 colored rectangles with color animation
 *   2. Text   -- 8 labels rendered simultaneously
 *   3. Bars   -- 6 animated progress bars
 *   4. Churn  -- rapid object create/delete stress
 *   5. Mixed  -- rectangles + labels + bars combined
 *
 * Performance monitor (FPS, CPU%, render/flush time) is
 * shown at bottom-right via LVGL sysmon.  Memory usage
 * is shown at bottom-left.
 */

#include "sgl.h"
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_port_tick.h"

/* ---------- benchmark config ---------- */
#define SCENE_DURATION_MS  6000
#define NUM_SCENES         5

static const char *scene_names[NUM_SCENES] = {
    "1/5  Fill",
    "2/5  Text",
    "3/5  Bars",
    "4/5  Churn",
    "5/5  Mixed"
};

/* 16-entry colour palette */
static const uint32_t colors[16] = {
    0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00,
    0xFF00FF, 0x00FFFF, 0xFF8000, 0x8000FF,
    0x008080, 0x808000, 0x800080, 0x008000,
    0x000080, 0x800000, 0x404040, 0xC0C0C0
};

/* state */
static int          current_scene = -1;
static lv_obj_t    *scene_cont    = NULL;   /* container for scene objects    */
static lv_obj_t    *title_label   = NULL;   /* persistent scene-name label    */
static lv_timer_t  *scene_timer   = NULL;   /* advances scenes every 6 s     */
static lv_timer_t  *churn_timer   = NULL;   /* used by churn scene only      */

/* ---------- helpers ---------- */

/* Create a flat rectangle (no radius / shadow / border). */
static lv_obj_t *make_rect(lv_obj_t *parent,
                            int x, int y, int w, int h, uint32_t c)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, lv_color_hex(c), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(o, 0, 0);
    lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
    return o;
}

/* Create a positioned, coloured label. */
static lv_obj_t *make_label(lv_obj_t *parent,
                             int x, int y, const char *txt, uint32_t c)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_pos(l, x, y);
    lv_obj_set_style_text_color(l, lv_color_hex(c), 0);
    return l;
}

/* ---------- animation callbacks ---------- */

/* HSV-like hue sweep (integer-only, no FPU). */
static void color_anim_cb(void *var, int32_t v)
{
    uint8_t r, g, b;
    int h = v % 360;
    int s = h / 60;
    int f = h % 60;
    switch (s) {
        case 0: r = 255; g = (uint8_t)((f * 255) / 60); b = 0;   break;
        case 1: r = (uint8_t)(255 - (f * 255) / 60); g = 255; b = 0;   break;
        case 2: r = 0;   g = 255; b = (uint8_t)((f * 255) / 60); break;
        case 3: r = 0;   g = (uint8_t)(255 - (f * 255) / 60); b = 255; break;
        case 4: r = (uint8_t)((f * 255) / 60); g = 0;   b = 255; break;
        default:r = 255; g = 0;   b = (uint8_t)(255 - (f * 255) / 60); break;
    }
    lv_obj_set_style_bg_color((lv_obj_t *)var, lv_color_make(r, g, b), 0);
}

/* Bar value setter (wraps 3-arg API into 2-arg anim callback). */
static void bar_anim_cb(void *var, int32_t v)
{
    lv_bar_set_value((lv_obj_t *)var, v, LV_ANIM_OFF);
}

/* ---------- scene 0 : Fill ---------- */

static void scene_fill(void)
{
    int i, j;
    int rw = 80;   /* 320 / 4 */
    int rh = 40;   /* leave room for title (top) + monitors (bottom) */

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            lv_obj_t *r = make_rect(scene_cont,
                                    j * rw, 24 + i * rh,
                                    rw, rh, colors[idx]);

            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, r);
            lv_anim_set_values(&a, idx * 22, idx * 22 + 360);
            lv_anim_set_time(&a, 2000);
            lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
            lv_anim_set_exec_cb(&a, color_anim_cb);
            lv_anim_start(&a);
        }
    }
}

/* ---------- scene 1 : Text ---------- */

static void scene_text(void)
{
    static const char *texts[] = {
        "LVGL on Saturn!",  "SH-2 28.6 MHz",
        "VDP2 NBG1 Bitmap", "320x224 RGB555",
        "48 KB LVGL Heap",  "No FPU, No OS",
        "Pure SGL Port",    "Benchmark v1.0"
    };
    int i;
    for (i = 0; i < 8; i++) {
        int col = i / 4;
        int row = i % 4;
        make_label(scene_cont,
                   10 + col * 160, 26 + row * 38,
                   texts[i], colors[i * 2]);
    }
}

/* ---------- scene 2 : Bars ---------- */

static void scene_bars(void)
{
    int i;
    for (i = 0; i < 6; i++) {
        lv_obj_t *bar = lv_bar_create(scene_cont);
        lv_obj_set_size(bar, 280, 16);
        lv_obj_set_pos(bar, 20, 26 + i * 26);

        /* Flat style */
        lv_obj_set_style_radius(bar, 0, 0);
        lv_obj_set_style_radius(bar, 0, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x333333), 0);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(bar, lv_color_hex(colors[i * 2]), LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);

        lv_bar_set_range(bar, 0, 100);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, bar);
        lv_anim_set_values(&a, 0, 100);
        lv_anim_set_time(&a, 1200 + i * 300);
        lv_anim_set_playback_time(&a, 1200 + i * 300);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&a, bar_anim_cb);
        lv_anim_start(&a);
    }
}

/* ---------- scene 3 : Churn ---------- */

#define CHURN_COUNT 10
static lv_obj_t *churn_objs[CHURN_COUNT];
static int churn_cycle;

static void churn_timer_cb(lv_timer_t *t)
{
    int i;
    (void)t;

    /* delete previous batch */
    for (i = 0; i < CHURN_COUNT; i++) {
        if (churn_objs[i]) {
            lv_obj_delete(churn_objs[i]);
            churn_objs[i] = NULL;
        }
    }

    /* create new batch (2 columns x 5 rows, below the header line) */
    churn_cycle++;
    for (i = 0; i < CHURN_COUNT; i++) {
        static char buf[20];
        lv_snprintf(buf, sizeof(buf), "C%d #%d", churn_cycle, i);
        churn_objs[i] = make_label(scene_cont,
                                   10 + (i % 2) * 155,
                                   46 + (i / 2) * 28,
                                   buf,
                                   colors[(churn_cycle + i) & 0xF]);
    }
}

static void scene_churn(void)
{
    int i;
    churn_cycle = 0;
    for (i = 0; i < CHURN_COUNT; i++) churn_objs[i] = NULL;

    make_label(scene_cont, 10, 26, "Creating / deleting objects...", 0xFFFFFF);
    churn_timer = lv_timer_create(churn_timer_cb, 250, NULL);
}

/* ---------- scene 4 : Mixed ---------- */

static void scene_mixed(void)
{
    int i;

    /* 4 animated background rectangles */
    for (i = 0; i < 4; i++) {
        lv_obj_t *r = make_rect(scene_cont, i * 80, 24, 80, 80, colors[i]);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, r);
        lv_anim_set_values(&a, i * 90, i * 90 + 360);
        lv_anim_set_time(&a, 3000);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&a, color_anim_cb);
        lv_anim_start(&a);
    }

    /* 4 labels overlaid on rectangles */
    make_label(scene_cont,  10, 30, "Saturn", 0xFFFFFF);
    make_label(scene_cont, 170, 30, "LVGL",   0xFFFFFF);
    make_label(scene_cont,  10, 65, "SGL",    0xFFFFFF);
    make_label(scene_cont, 170, 65, "Bench",  0xFFFFFF);

    /* 3 animated bars below */
    for (i = 0; i < 3; i++) {
        lv_obj_t *bar = lv_bar_create(scene_cont);
        lv_obj_set_size(bar, 280, 14);
        lv_obj_set_pos(bar, 20, 112 + i * 24);

        lv_obj_set_style_radius(bar, 0, 0);
        lv_obj_set_style_radius(bar, 0, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x222222), 0);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(bar, lv_color_hex(colors[i * 4 + 4]), LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);

        lv_bar_set_range(bar, 0, 100);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, bar);
        lv_anim_set_values(&a, 0, 100);
        lv_anim_set_time(&a, 1000 + i * 500);
        lv_anim_set_playback_time(&a, 1000 + i * 500);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&a, bar_anim_cb);
        lv_anim_start(&a);
    }
}

/* ---------- scene management ---------- */

static void cleanup_scene(void)
{
    if (churn_timer) {
        lv_timer_delete(churn_timer);
        churn_timer = NULL;
    }
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

    /* bring title to front so scene objects don't cover it */
    {
        lv_obj_t *parent = lv_obj_get_parent(title_label);
        lv_obj_move_to_index(title_label,
                             lv_obj_get_child_count(parent) - 1);
    }

    switch (current_scene) {
        case 0: scene_fill();  break;
        case 1: scene_text();  break;
        case 2: scene_bars();  break;
        case 3: scene_churn(); break;
        case 4: scene_mixed(); break;
    }
}

/* ---------- entry point ---------- */

static void create_benchmark_ui(void)
{
    lv_obj_t *scr = lv_screen_active();

    /* dark background */
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x001020), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    /* persistent title at top */
    title_label = lv_label_create(scr);
    lv_label_set_text(title_label, "Saturn LVGL Benchmark");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFF00), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 4);

    /* show sysmon overlays */
    lv_sysmon_show_performance(NULL);
    lv_sysmon_show_memory(NULL);

    /* Fix sysmon label backgrounds: the default LV_OPA_50 (semi-transparent)
     * leaves gray ghost artifacts with LV_DRAW_SW_COMPLEX=0.
     * Override all system-layer labels to fully opaque black. */
    {
        lv_obj_t *sys_layer = lv_display_get_layer_sys(NULL);
        uint32_t cnt = lv_obj_get_child_count(sys_layer);
        uint32_t ci;
        for (ci = 0; ci < cnt; ci++) {
            lv_obj_t *child = lv_obj_get_child(sys_layer, ci);
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

    create_benchmark_ui();

    /* main loop */
    while (1) {
        lv_timer_handler();
        slSynch();
    }

    return 0;
}
