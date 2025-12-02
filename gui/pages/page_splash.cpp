#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include <cstring>
#include <cstdio>

void page_splash_create(lv_obj_t* parent) {
    // Set up flex layout for vertical centering
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Tagline
    lv_obj_t* tagline = lv_label_create(parent);
    lv_label_set_text(tagline, tr(STR_APP_TAGLINE));
    lv_obj_set_style_text_font(tagline, FONT_FOOTER, 0);
    lv_obj_set_style_text_color(tagline, lv_color_hex(GUI_COLOR_SHADES[10]), 0);

    // Features list - build dynamically
    static char features_text[256];
    snprintf(features_text, sizeof(features_text),
        "- %s\n- %s\n- %s\n- %s\n- %s",
        tr(STR_FEATURE_SERVO),
        tr(STR_FEATURE_LIPO),
        tr(STR_FEATURE_CG_SCALE),
        tr(STR_FEATURE_DEFLECTION),
        tr(STR_FEATURE_ANGLE));

    lv_obj_t* features = lv_label_create(parent);
    lv_label_set_text(features, features_text);
    lv_obj_set_style_text_font(features, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(features, lv_color_hex(GUI_COLOR_SHADES[7]), 0);
    lv_obj_set_style_pad_top(features, 5, 0);  // Space between tagline and features
}
