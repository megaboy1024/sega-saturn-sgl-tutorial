#include "lv_port_indev.h"
#include "sgl.h"
#include "lvgl/lvgl.h"

/* Virtual cursor position (start at center of 320x224) */
static int32_t cursor_x = 160;
static int32_t cursor_y = 112;

/* Cursor movement speed (pixels per read) */
#define CURSOR_SPEED 3

static void saturn_indev_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    Uint16 pad = Smpc_Peripheral[0].data;

    (void)indev;

    /* Update cursor from D-pad (active-low: bit=0 means pressed) */
    if (!(pad & PER_DGT_KL)) cursor_x -= CURSOR_SPEED;
    if (!(pad & PER_DGT_KR)) cursor_x += CURSOR_SPEED;
    if (!(pad & PER_DGT_KU)) cursor_y -= CURSOR_SPEED;
    if (!(pad & PER_DGT_KD)) cursor_y += CURSOR_SPEED;

    /* Clamp to screen bounds */
    if (cursor_x < 0)   cursor_x = 0;
    if (cursor_x > 319) cursor_x = 319;
    if (cursor_y < 0)   cursor_y = 0;
    if (cursor_y > 223) cursor_y = 223;

    data->point.x = cursor_x;
    data->point.y = cursor_y;

    /* A button = press (active-low) */
    data->state = !(pad & PER_DGT_TA)
                  ? LV_INDEV_STATE_PRESSED
                  : LV_INDEV_STATE_RELEASED;

    data->continue_reading = false;
}

void lv_port_indev_init(void)
{
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, saturn_indev_read_cb);

    /* Create a visible cursor (small square -- must use radius=0 because
     * LV_DRAW_SW_COMPLEX is disabled on Saturn for code-size reasons) */
    lv_obj_t *cursor = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(cursor);          /* strip theme defaults (card style has radius) */
    lv_obj_set_size(cursor, 8, 8);
    lv_obj_set_style_radius(cursor, 0, 0);
    lv_obj_set_style_bg_color(cursor, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(cursor, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cursor, 0, 0);
    lv_obj_set_style_pad_all(cursor, 0, 0);
    lv_obj_set_scrollbar_mode(cursor, LV_SCROLLBAR_MODE_OFF);
    lv_indev_set_cursor(indev, cursor);
}
