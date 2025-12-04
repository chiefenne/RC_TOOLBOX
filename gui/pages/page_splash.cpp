#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include "gui/images/images.h"
#include "gui/pages/page_splash.h"
#include <cstring>
#include <cstdio>

// Static pointers to manage splash phases
static lv_obj_t* splash_logo = nullptr;
static lv_obj_t* splash_text_container = nullptr;
static lv_timer_t* splash_timer = nullptr;

// Cleanup function - call before destroying splash page
void page_splash_cleanup(void) {
    if (splash_timer) {
        lv_timer_delete(splash_timer);
        splash_timer = nullptr;
    }
    splash_logo = nullptr;
    splash_text_container = nullptr;
}

// Timer callback to transition from logo to text
static void splash_phase_callback(lv_timer_t* timer) {
    (void)timer;

    // Safety check - objects may have been destroyed
    if (!splash_logo || !splash_text_container) {
        if (splash_timer) {
            lv_timer_delete(splash_timer);
            splash_timer = nullptr;
        }
        return;
    }

    // Hide logo, show text
    lv_obj_add_flag(splash_logo, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(splash_text_container, LV_OBJ_FLAG_HIDDEN);

    // Delete the timer (one-shot)
    if (splash_timer) {
        lv_timer_delete(splash_timer);
        splash_timer = nullptr;
    }
}

void page_splash_create(lv_obj_t* parent) {
    // Set up flex layout for vertical centering
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Phase 1: Logo image (shown first for 2 seconds)
    splash_logo = lv_image_create(parent);
    lv_image_set_src(splash_logo, &RC_Toolbox_Logo_150px);

    // Phase 2: Text container (hidden initially, shown after 2 seconds)
    splash_text_container = lv_obj_create(parent);
    lv_obj_remove_style_all(splash_text_container);
    lv_obj_set_size(splash_text_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(splash_text_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(splash_text_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(splash_text_container, LV_OBJ_FLAG_HIDDEN);  // Hidden initially

    // Tagline
    lv_obj_t* tagline = lv_label_create(splash_text_container);
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

    lv_obj_t* features = lv_label_create(splash_text_container);
    lv_label_set_text(features, features_text);
    lv_obj_set_style_text_font(features, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(features, lv_color_hex(GUI_COLOR_SHADES[7]), 0);
    lv_obj_set_style_pad_top(features, 10, 0);  // Space between tagline and features

    // Start timer to switch from logo to text after 3 seconds
    splash_timer = lv_timer_create(splash_phase_callback, 3000, nullptr);
    lv_timer_set_repeat_count(splash_timer, 1);  // One-shot timer
}
