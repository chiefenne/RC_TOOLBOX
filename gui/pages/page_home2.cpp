#include "lvgl.h"
#include "gui/gui.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include "gui/input.h"

// =============================================================================
// Focus Order Configuration
// =============================================================================
// Define focus order here - change these numbers to reorder navigation
// Layout (2 columns x 3 rows), then footer buttons:
//   [0: About]     [1: Reserved]
//   [2: Reserved]  [3: Reserved]
//   [4: Reserved]  [5: Reserved]
//   Footer: [6: Home] [7: Prev] [8: Next] [9: Settings]
enum FocusOrder {
    FO_ABOUT      = 0,
    FO_RESERVED_1 = 1,
    FO_RESERVED_2 = 2,
    FO_RESERVED_3 = 3,
    FO_RESERVED_4 = 4,
    FO_RESERVED_5 = 5,
    FO_BTN_HOME   = 6,
    FO_BTN_PREV   = 7,
    FO_BTN_NEXT   = 8,
    FO_BTN_SETTINGS = 9,
};

// Focus group builder for this page
static FocusOrderBuilder focus_builder;

// =============================================================================
// Callbacks
// =============================================================================
static void btn_about_cb(lv_event_t* e) { LV_UNUSED(e); gui_set_page(PAGE_ABOUT); }

// =============================================================================
// Button Factory
// =============================================================================
static lv_obj_t* create_nav_button(lv_obj_t* parent, const char* text, lv_event_cb_t cb, int focus_order) {
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_size(btn, 140, 40);
    lv_obj_set_style_bg_color(btn, lv_color_hex(GUI_COLOR_MONO[1]), 0);
    lv_obj_set_style_radius(btn, 5, 0);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, FONT_BUTTON_SMMD, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(GUI_COLOR_TINTS[10]), 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(lbl);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);

    // Add to focus builder at specified order position
    focus_builder.add(btn, focus_order);

    return btn;
}

// =============================================================================
// Page Create/Destroy
// =============================================================================
void page_home2_create(lv_obj_t* parent) {
    // Initialize focus builder
    focus_builder.init();

    // Record this page in navigation history
    input_push_page(PAGE_HOME_2);

    // Layout: 2 columns x 3 rows
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(parent, 10, 0);
    lv_obj_set_style_pad_column(parent, 10, 0);

    // Create buttons - visual order (left to right, top to bottom)
    // The focus_order parameter controls encoder navigation order
    create_nav_button(parent, tr(STR_BTN_ABOUT), btn_about_cb, FO_ABOUT);

    // Space for future buttons:
    // create_nav_button(parent, "WiFi\nConfig", btn_wifi_cb, FO_RESERVED_1);
    // create_nav_button(parent, "Calibrate", btn_calibrate_cb, FO_RESERVED_2);
    // create_nav_button(parent, "SD Card", btn_sdcard_cb, FO_RESERVED_3);
    // create_nav_button(parent, "Updates", btn_update_cb, FO_RESERVED_4);
    // create_nav_button(parent, "Backup", btn_backup_cb, FO_RESERVED_5);

    // Add footer buttons to focus order
    focus_builder.add(gui_get_btn_home(), FO_BTN_HOME);
    focus_builder.add(gui_get_btn_prev(), FO_BTN_PREV);
    focus_builder.add(gui_get_btn_next(), FO_BTN_NEXT);
    focus_builder.add(gui_get_btn_settings(), FO_BTN_SETTINGS);
    focus_builder.finalize();
}

void page_home2_destroy() {
    // Nothing to clean up
}
