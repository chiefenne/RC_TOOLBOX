#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"

void page_servo_create(lv_obj_t* parent) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, "Servo Tester");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(GUI_COLOR_SHADES[7]), 0);
    lv_obj_center(label);
}
