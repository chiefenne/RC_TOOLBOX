#include "lvgl.h"
#include "gui/gui.h"
#include "gui/color_palette.h"
#include "gui/images/images.h"
#include "gui/pages/page_splash.h"

// Cleanup function - nothing to clean up now
void page_splash_cleanup(void) {
    // No timers or dynamic resources to clean up
}

void page_splash_create(lv_obj_t* parent) {
    // White background for logo (index 0 is default white)
    lv_obj_set_style_bg_color(parent, lv_color_hex(GUI_COLOR_BG[0]), 0);

    // Set up flex layout for centering
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Logo image centered
    lv_obj_t* logo = lv_image_create(parent);
    lv_image_set_src(logo, &RC_Toolbox_Logo_150px);
}
