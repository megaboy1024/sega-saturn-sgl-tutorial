# Minimal LVGL source files for Saturn (core + label + button only)
# This dramatically reduces binary size vs compiling all 311 files.

# --- Init ---
SRCS += lvgl/src/lv_init.c

# --- Core ---
SRCS += lvgl/src/core/lv_group.c
SRCS += lvgl/src/core/lv_obj.c
SRCS += lvgl/src/core/lv_obj_class.c
SRCS += lvgl/src/core/lv_obj_draw.c
SRCS += lvgl/src/core/lv_obj_event.c
SRCS += lvgl/src/core/lv_obj_id_builtin.c
SRCS += lvgl/src/core/lv_obj_pos.c
SRCS += lvgl/src/core/lv_obj_property.c
SRCS += lvgl/src/core/lv_obj_scroll.c
SRCS += lvgl/src/core/lv_obj_style.c
SRCS += lvgl/src/core/lv_obj_style_gen.c
SRCS += lvgl/src/core/lv_obj_tree.c
SRCS += lvgl/src/core/lv_refr.c

# --- Display ---
SRCS += lvgl/src/display/lv_display.c

# --- Draw core ---
SRCS += lvgl/src/draw/lv_draw.c
SRCS += lvgl/src/draw/lv_draw_arc.c
SRCS += lvgl/src/draw/lv_draw_buf.c
SRCS += lvgl/src/draw/lv_draw_image.c
SRCS += lvgl/src/draw/lv_draw_label.c
SRCS += lvgl/src/draw/lv_draw_line.c
SRCS += lvgl/src/draw/lv_draw_mask.c
SRCS += lvgl/src/draw/lv_draw_rect.c
SRCS += lvgl/src/draw/lv_draw_triangle.c
SRCS += lvgl/src/draw/lv_draw_vector.c
SRCS += lvgl/src/draw/lv_image_decoder.c

# --- Draw SW renderer ---
SRCS += lvgl/src/draw/sw/lv_draw_sw.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_arc.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_border.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_box_shadow.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_fill.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_gradient.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_img.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_letter.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_line.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_mask.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_mask_rect.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_transform.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_triangle.c
SRCS += lvgl/src/draw/sw/lv_draw_sw_vector.c
SRCS += lvgl/src/draw/sw/blend/lv_draw_sw_blend.c
SRCS += lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_rgb565.c

# --- Font ---
SRCS += lvgl/src/font/lv_font.c
SRCS += lvgl/src/font/lv_font_fmt_txt.c
SRCS += lvgl/src/font/lv_font_montserrat_14.c

# --- Input device ---
SRCS += lvgl/src/indev/lv_indev.c
SRCS += lvgl/src/indev/lv_indev_scroll.c

# --- Misc ---
SRCS += lvgl/src/misc/lv_anim.c
SRCS += lvgl/src/misc/lv_anim_timeline.c
SRCS += lvgl/src/misc/lv_area.c
SRCS += lvgl/src/misc/lv_array.c
SRCS += lvgl/src/misc/lv_async.c
SRCS += lvgl/src/misc/lv_color.c
SRCS += lvgl/src/misc/lv_color_op.c
SRCS += lvgl/src/misc/lv_event.c
SRCS += lvgl/src/misc/lv_fs.c
SRCS += lvgl/src/misc/lv_ll.c
SRCS += lvgl/src/misc/lv_log.c
SRCS += lvgl/src/misc/lv_math.c
SRCS += lvgl/src/misc/lv_matrix.c
SRCS += lvgl/src/misc/lv_palette.c
SRCS += lvgl/src/misc/lv_profiler_builtin.c
SRCS += lvgl/src/misc/lv_rb.c
SRCS += lvgl/src/misc/lv_style.c
SRCS += lvgl/src/misc/lv_style_gen.c
SRCS += lvgl/src/misc/lv_text.c
SRCS += lvgl/src/misc/lv_text_ap.c
SRCS += lvgl/src/misc/lv_timer.c
SRCS += lvgl/src/misc/lv_utils.c
SRCS += lvgl/src/misc/cache/lv_cache.c
SRCS += lvgl/src/misc/cache/lv_cache_entry.c
SRCS += lvgl/src/misc/cache/lv_cache_lru_rb.c
SRCS += lvgl/src/misc/cache/lv_image_cache.c
SRCS += lvgl/src/misc/cache/lv_image_header_cache.c

# --- OSAL (no OS) ---
SRCS += lvgl/src/osal/lv_os_none.c
SRCS += lvgl/src/osal/lv_os.c

# --- Stdlib builtin ---
SRCS += lvgl/src/stdlib/lv_mem.c
SRCS += lvgl/src/stdlib/builtin/lv_mem_core_builtin.c
SRCS += lvgl/src/stdlib/builtin/lv_sprintf_builtin.c
SRCS += lvgl/src/stdlib/builtin/lv_string_builtin.c
SRCS += lvgl/src/stdlib/builtin/lv_tlsf.c

# --- Tick ---
SRCS += lvgl/src/tick/lv_tick.c

# --- Themes (need at least one) ---
SRCS += lvgl/src/themes/lv_theme.c
SRCS += lvgl/src/themes/default/lv_theme_default.c
SRCS += lvgl/src/themes/simple/lv_theme_simple.c

# --- Layouts ---
SRCS += lvgl/src/layouts/lv_layout.c
SRCS += lvgl/src/layouts/flex/lv_flex.c

# --- Widgets (label + button + bar for theme) ---
SRCS += lvgl/src/widgets/label/lv_label.c
SRCS += lvgl/src/widgets/button/lv_button.c
SRCS += lvgl/src/widgets/bar/lv_bar.c

# --- Libs: bin decoder (needed for image/font cache) ---
SRCS += lvgl/src/libs/bin_decoder/lv_bin_decoder.c
