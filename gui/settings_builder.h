// gui/settings_builder.h - Reusable settings UI builder with collapsible sections

#pragma once
#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"

// Row dimensions
constexpr lv_coord_t SETTINGS_ROW_H = 36;
constexpr lv_coord_t SETTINGS_LABEL_W = 120;
constexpr lv_coord_t SETTINGS_CTRL_W = 100;
constexpr int SETTINGS_MAX_SECTIONS = 10;

class SettingsBuilder {
    lv_obj_t* cont_;           // Main scrollable container
    lv_obj_t* section_cont_;   // Current section's content container
    int section_count_ = 0;

    // Create a row container with label on left, control on right
    lv_obj_t* make_row(const char* label_text);

    // Toggle section visibility
    static void on_header_click(lv_event_t* e);

public:
    explicit SettingsBuilder(lv_obj_t* parent);

    // Begin a collapsible section (all following items go into this section)
    // Returns the header object so it can be added to focus navigation
    lv_obj_t* begin_section(const char* title, bool start_expanded = false);

    // End the current section
    void end_section();

    // Dropdown selector (returns dropdown for further customization)
    lv_obj_t* dropdown(const char* label, const char* options, int selected_idx,
                       lv_event_cb_t on_change = nullptr, void* user_data = nullptr);

    // Slider with value display
    lv_obj_t* slider(const char* label, int min, int max, int value,
                     lv_event_cb_t on_change = nullptr, void* user_data = nullptr);

    // On/Off switch
    lv_obj_t* toggle(const char* label, bool value,
                     lv_event_cb_t on_change = nullptr, void* user_data = nullptr);

    // Action button (opens subpage, keyboard, etc.)
    lv_obj_t* action(const char* label, const char* value_text,
                     lv_event_cb_t on_click, void* user_data = nullptr);

    // Centered button (for reset/action buttons)
    lv_obj_t* button(const char* label, lv_event_cb_t on_click, void* user_data = nullptr);

    // Read-only info label
    void info(const char* label, const char* value);
};
