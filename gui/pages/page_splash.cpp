#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"

void page_splash_create(lv_obj_t* parent) {

    // Tagline
    lv_obj_t* tagline = lv_label_create(parent);
    lv_label_set_text(tagline, "Tools for RC Enthusiasts");
    lv_obj_set_style_text_font(tagline, FONT_FOOTER, 0);
    lv_obj_set_style_text_color(tagline, lv_color_hex(GUI_COLOR_SHADES[10]), 0);
    lv_obj_center(tagline);

    // Features list
    lv_obj_t* features = lv_label_create(parent);
    lv_label_set_text(features,
        "• Servo tester\n"
        "• Lipo checker\n"
        "• Flap deflection measurement\n"
        "• Angle calculator\n");
    lv_obj_set_style_text_font(features, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(features, lv_color_hex(GUI_COLOR_SHADES[7]), 0);
    lv_obj_align_to(features, tagline, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
}
