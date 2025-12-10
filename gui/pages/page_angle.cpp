#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include "gui/input.h"
#include "gui/gui.h"

// =============================================================================
// Focus Order Configuration
// =============================================================================
enum FocusOrder {
    FO_BTN_HOME     = 0,
    FO_BTN_PREV     = 1,
    FO_BTN_NEXT     = 2,
    FO_BTN_SETTINGS = 3,
};

// Focus group builder for this page
static FocusOrderBuilder focus_builder;

void page_angle_create(lv_obj_t* parent) {
    // Initialize focus builder
    focus_builder.init();

    // Record this page in navigation history
    input_push_page(PAGE_ANGLE);

    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, tr(STR_ANGLE_CONTENT));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(GUI_COLOR_SHADES[7]), 0);
    lv_obj_center(label);

    // Add footer buttons to focus order
    focus_builder.add(gui_get_btn_home(), FO_BTN_HOME);
    focus_builder.add(gui_get_btn_prev(), FO_BTN_PREV);
    focus_builder.add(gui_get_btn_next(), FO_BTN_NEXT);
    focus_builder.add(gui_get_btn_settings(), FO_BTN_SETTINGS);

    // Finalize focus builder
    focus_builder.finalize();
}

void page_angle_destroy() {
    focus_builder.destroy();
}
