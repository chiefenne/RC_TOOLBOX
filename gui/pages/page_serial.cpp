#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include "gui/input.h"
#include "gui/gui.h"
#include <cstring>

// =============================================================================
// Focus Order Configuration
// =============================================================================
enum FocusOrder {
    FO_BTN_CLEAR    = 0,
    FO_BTN_HOME     = 1,
    FO_BTN_PREV     = 2,
    FO_BTN_NEXT     = 3,
    FO_BTN_SETTINGS = 4,
};

// Focus group builder for this page
static FocusOrderBuilder focus_builder;

// =============================================================================
// Serial Monitor - Display debug messages
// =============================================================================

static lv_obj_t* log_textarea = nullptr;

// Circular buffer for messages
#define MAX_LOG_MESSAGES 50
#define MAX_MSG_LENGTH 128
static char log_buffer[MAX_LOG_MESSAGES][MAX_MSG_LENGTH];
static int log_write_idx = 0;
static int log_count = 0;

// Update the text area with all messages
static void update_display() {
    if (!log_textarea) return;

    // Build text from circular buffer
    static char full_text[MAX_LOG_MESSAGES * MAX_MSG_LENGTH];
    full_text[0] = '\0';

    int start = (log_count >= MAX_LOG_MESSAGES) ? log_write_idx : 0;
    int num_msgs = (log_count < MAX_LOG_MESSAGES) ? log_count : MAX_LOG_MESSAGES;

    for (int i = 0; i < num_msgs; i++) {
        int idx = (start + i) % MAX_LOG_MESSAGES;
        strcat(full_text, log_buffer[idx]);
        if (i < num_msgs - 1) {
            strcat(full_text, "\n");
        }
    }

    lv_textarea_set_text(log_textarea, full_text);

    // Scroll to bottom
    lv_obj_scroll_to_y(log_textarea, LV_COORD_MAX, LV_ANIM_OFF);
}

void page_serial_add_message(const char* msg) {
    if (!msg) return;

    // Copy message to buffer
    strncpy(log_buffer[log_write_idx], msg, MAX_MSG_LENGTH - 1);
    log_buffer[log_write_idx][MAX_MSG_LENGTH - 1] = '\0';

    log_write_idx = (log_write_idx + 1) % MAX_LOG_MESSAGES;
    log_count++;

    update_display();
}

void page_serial_clear() {
    log_write_idx = 0;
    log_count = 0;
    if (log_textarea) {
        lv_textarea_set_text(log_textarea, "");
    }
}

static void btn_clear_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        page_serial_clear();
    }
}

void page_serial_create(lv_obj_t* parent) {
    // Initialize focus builder
    focus_builder.init();

    // Record this page in navigation history
    input_push_page(PAGE_SERIAL);

    // Main container with flex layout
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_style_pad_row(parent, 8, 0);

    // Clear button
    lv_obj_t* btn_clear = lv_button_create(parent);
    lv_obj_set_size(btn_clear, 100, 30);
    lv_obj_add_event_cb(btn_clear, btn_clear_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_set_style_bg_color(btn_clear, lv_color_hex(GUI_COLOR_TRIAD[2]), 0);  // Orange
    lv_obj_set_style_text_color(btn_clear, lv_color_white(), 0);

    lv_obj_t* btn_clear_label = lv_label_create(btn_clear);
    lv_label_set_text(btn_clear_label, "CLEAR");
    lv_obj_set_style_text_font(btn_clear_label, FONT_DEFAULT, 0);
    lv_obj_center(btn_clear_label);

    // Text area for log display
    log_textarea = lv_textarea_create(parent);
    lv_obj_set_size(log_textarea, LV_PCT(100), LV_PCT(100));
    lv_textarea_set_text(log_textarea, "");
    lv_obj_set_style_text_font(log_textarea, &courier_new_14, 0);  // Larger monospace font
    lv_obj_set_style_bg_color(log_textarea, lv_color_hex(0x000000), 0);  // Black background
    lv_obj_set_style_text_color(log_textarea, lv_color_hex(0x00FF00), 0);  // Green text
    lv_obj_set_style_border_width(log_textarea, 1, 0);
    lv_obj_set_style_border_color(log_textarea, lv_color_hex(GUI_COLOR_GRAYS[4]), 0);
    lv_textarea_set_cursor_click_pos(log_textarea, false);  // Disable cursor

    // Show any buffered messages
    update_display();

    // Add buttons to focus order
    focus_builder.add(btn_clear, FO_BTN_CLEAR);
    focus_builder.add(gui_get_btn_home(), FO_BTN_HOME);
    focus_builder.add(gui_get_btn_prev(), FO_BTN_PREV);
    focus_builder.add(gui_get_btn_next(), FO_BTN_NEXT);
    focus_builder.add(gui_get_btn_settings(), FO_BTN_SETTINGS);
    focus_builder.finalize();
}

void page_serial_destroy() {
    log_textarea = nullptr;
}
