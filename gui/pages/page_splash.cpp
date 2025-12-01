#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include <cstring>
#include <cstdio>

void page_splash_create(lv_obj_t* parent) {

    // Tagline
    lv_obj_t* tagline = lv_label_create(parent);
    lv_label_set_text(tagline, tr(STR_APP_TAGLINE));
    lv_obj_set_style_text_font(tagline, FONT_FOOTER, 0);
    lv_obj_set_style_text_color(tagline, lv_color_hex(GUI_COLOR_SHADES[10]), 0);
    lv_obj_center(tagline);

    // Features list - build dynamically
    static char features_text[256];
    snprintf(features_text, sizeof(features_text),
        "• %s\n• %s\n• %s\n• %s\n• %s",
        tr(STR_FEATURE_SERVO),
        tr(STR_FEATURE_LIPO),
        tr(STR_FEATURE_CG_SCALE),
        tr(STR_FEATURE_DEFLECTION),
        tr(STR_FEATURE_ANGLE));

    lv_obj_t* features = lv_label_create(parent);
    lv_label_set_text(features, features_text);
    lv_obj_set_style_text_font(features, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(features, lv_color_hex(GUI_COLOR_SHADES[7]), 0);
    lv_obj_align_to(features, tagline, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
}
