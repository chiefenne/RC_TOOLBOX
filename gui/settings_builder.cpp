// gui/settings_builder.cpp - Reusable settings UI builder with collapsible sections

#include "gui/settings_builder.h"
#include "gui/lang.h"

// Colors
namespace {
    const lv_color_t COL_HEADER_BG = lv_color_hex(GUI_COLOR_MONO[1]);
    const lv_color_t COL_HEADER_TEXT = lv_color_hex(GUI_COLOR_TINTS[10]);
    const lv_color_t COL_TEXT = lv_color_hex(GUI_COLOR_SHADES[7]);
    const lv_color_t COL_CTRL_BG = lv_color_hex(GUI_COLOR_GRAYS[8]);
    const lv_color_t COL_ACTIVE = lv_color_hex(GUI_COLOR_TRIAD[1]);
    const lv_color_t COL_INACTIVE = lv_color_hex(GUI_COLOR_GRAYS[6]);
}

SettingsBuilder::SettingsBuilder(lv_obj_t* parent)
    : cont_(parent), section_cont_(nullptr), section_count_(0) {
    // Set up scrollable column layout
    lv_obj_set_flex_flow(cont_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont_, 4, 0);
    lv_obj_set_style_pad_hor(cont_, 8, 0);
    lv_obj_set_style_pad_top(cont_, 8, 0);
    lv_obj_set_scrollbar_mode(cont_, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_add_flag(cont_, LV_OBJ_FLAG_SCROLLABLE);
}

void SettingsBuilder::on_header_click(lv_event_t* e) {
    lv_obj_t* header = lv_event_get_target_obj(e);

    // Arrow is first child (index 0), title is second (index 1)
    lv_obj_t* arrow = lv_obj_get_child(header, 0);
    // Content container is the next sibling of the header
    lv_obj_t* content = lv_obj_get_sibling(header, 1);

    if (!arrow || !content) return;

    // Check current state by looking at content visibility
    bool is_hidden = lv_obj_has_flag(content, LV_OBJ_FLAG_HIDDEN);

    // Toggle
    if (is_hidden) {
        lv_obj_clear_flag(content, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(arrow, SYM_DOWN);
    } else {
        lv_obj_add_flag(content, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(arrow, SYM_RIGHT);
    }
}

lv_obj_t* SettingsBuilder::begin_section(const char* title, bool start_expanded) {
    if (section_count_ >= SETTINGS_MAX_SECTIONS) return nullptr;

    // Create clickable header
    lv_obj_t* header = lv_obj_create(cont_);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, COL_HEADER_BG, 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 4, 0);
    lv_obj_set_style_pad_left(header, 10, 0);
    lv_obj_set_style_pad_right(header, 10, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(header, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(header, lv_color_hex(GUI_COLOR_MONO[2]), LV_STATE_PRESSED);

    // Arrow on left (must be first child - index 0)
    lv_obj_t* arrow = lv_label_create(header);
    lv_label_set_text(arrow, start_expanded ? SYM_DOWN : SYM_RIGHT);
    lv_obj_set_style_text_font(arrow, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(arrow, COL_HEADER_TEXT, 0);
    lv_obj_align(arrow, LV_ALIGN_LEFT_MID, 0, 0);

    // Title label (second child - index 1)
    lv_obj_t* lbl = lv_label_create(header);
    lv_label_set_text(lbl, title);
    lv_obj_set_style_text_font(lbl, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(lbl, COL_HEADER_TEXT, 0);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 18, 0);  // Offset for arrow

    // Add click handler (no user_data needed - we find content via sibling)
    lv_obj_add_event_cb(header, on_header_click, LV_EVENT_CLICKED, nullptr);

    // Create content container (holds all rows for this section)
    // Must be created immediately after header so it's the next sibling
    lv_obj_t* content = lv_obj_create(cont_);
    lv_obj_set_size(content, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_style_pad_row(content, 2, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    // Start hidden if not expanded
    if (!start_expanded) {
        lv_obj_add_flag(content, LV_OBJ_FLAG_HIDDEN);
    }

    // Set as current container for rows
    section_cont_ = content;
    section_count_++;

    return header;  // Return header for focus navigation
}

void SettingsBuilder::end_section() {
    section_cont_ = nullptr;  // Reset to main container
}

lv_obj_t* SettingsBuilder::make_row(const char* label_text) {
    lv_obj_t* parent = section_cont_ ? section_cont_ : cont_;

    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), SETTINGS_ROW_H);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_left(row, 20, 0);  // Indent for section content
    lv_obj_set_style_pad_right(row, 5, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    // Label on left
    lv_obj_t* lbl = lv_label_create(row);
    lv_label_set_text(lbl, label_text);
    lv_obj_set_style_text_font(lbl, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(lbl, COL_TEXT, 0);

    return row;
}

lv_obj_t* SettingsBuilder::dropdown(const char* label, const char* options, int selected_idx,
                                     lv_event_cb_t on_change, void* user_data) {
    lv_obj_t* row = make_row(label);

    lv_obj_t* dd = lv_dropdown_create(row);
    lv_dropdown_set_options(dd, options);
    lv_dropdown_set_selected(dd, selected_idx);
    lv_dropdown_set_symbol(dd, SYM_DOWN);  // ▼ down arrow
    lv_obj_set_width(dd, SETTINGS_CTRL_W);
    lv_obj_set_style_text_font(dd, FONT_DEFAULT, 0);
    lv_obj_set_style_text_font(dd, FONT_DEFAULT, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(dd, 5, 0);

    // Style the dropdown list
    lv_obj_t* list = lv_dropdown_get_list(dd);
    if (list) {
        lv_obj_set_style_text_font(list, FONT_DEFAULT, 0);
    }

    if (on_change) {
        lv_obj_add_event_cb(dd, on_change, LV_EVENT_VALUE_CHANGED, user_data);
    }

    return dd;
}

lv_obj_t* SettingsBuilder::slider(const char* label, int min, int max, int value,
                                   lv_event_cb_t on_change, void* user_data) {
    lv_obj_t* row = make_row(label);

    // Container for slider + value label
    lv_obj_t* ctrl_cont = lv_obj_create(row);
    lv_obj_set_size(ctrl_cont, SETTINGS_CTRL_W + 50, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(ctrl_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_cont, 0, 0);
    lv_obj_set_style_pad_all(ctrl_cont, 0, 0);
    lv_obj_set_flex_flow(ctrl_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_cont, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(ctrl_cont, 5, 0);
    lv_obj_clear_flag(ctrl_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* sl = lv_slider_create(ctrl_cont);
    lv_obj_set_width(sl, SETTINGS_CTRL_W - 30);
    lv_slider_set_range(sl, min, max);
    lv_slider_set_value(sl, value, LV_ANIM_OFF);

    // Make slider track thinner and knob smaller to fit in row
    lv_obj_set_style_height(sl, 6, LV_PART_MAIN);           // Thin track (6px)
    lv_obj_set_style_radius(sl, 3, LV_PART_MAIN);           // Rounded track
    lv_obj_set_style_radius(sl, 3, LV_PART_INDICATOR);      // Rounded indicator
    // Knob: set explicit size (12x12) and round
    lv_obj_set_style_height(sl, 12, LV_PART_KNOB);
    lv_obj_set_style_width(sl, 12, LV_PART_KNOB);
    lv_obj_set_style_radius(sl, 6, LV_PART_KNOB);           // Circular knob
    lv_obj_set_style_pad_all(sl, 0, LV_PART_KNOB);          // No extra padding

    lv_obj_set_style_bg_color(sl, COL_CTRL_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sl, COL_ACTIVE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(sl, COL_ACTIVE, LV_PART_KNOB);

    // Value label - wide enough for "XXXX µs" format (monospace bold for stable width)
    lv_obj_t* val_lbl = lv_label_create(ctrl_cont);
    lv_label_set_text_fmt(val_lbl, "%4d", value);
    lv_obj_set_style_text_font(val_lbl, FONT_MONO_BOLD_SM, 0);
    lv_obj_set_style_text_color(val_lbl, COL_TEXT, 0);
    lv_obj_set_style_min_width(val_lbl, 50, 0);  // Wide enough for "XXXX µs"
    lv_obj_set_style_text_align(val_lbl, LV_TEXT_ALIGN_RIGHT, 0);

    // Update value label on change
    lv_obj_add_event_cb(sl, [](lv_event_t* e) {
        lv_obj_t* slider = lv_event_get_target_obj(e);
        lv_obj_t* container = lv_obj_get_parent(slider);
        lv_obj_t* val_label = lv_obj_get_child(container, 1);
        if (val_label) {
            lv_label_set_text_fmt(val_label, "%4d", lv_slider_get_value(slider));
        }
    }, LV_EVENT_VALUE_CHANGED, nullptr);

    if (on_change) {
        lv_obj_add_event_cb(sl, on_change, LV_EVENT_VALUE_CHANGED, user_data);
    }

    return sl;
}

lv_obj_t* SettingsBuilder::toggle(const char* label, bool value,
                                   lv_event_cb_t on_change, void* user_data) {
    lv_obj_t* row = make_row(label);

    lv_obj_t* sw = lv_switch_create(row);
    if (value) {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
    lv_obj_set_style_bg_color(sw, COL_INACTIVE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, COL_ACTIVE, LV_PART_INDICATOR | LV_STATE_CHECKED);

    if (on_change) {
        lv_obj_add_event_cb(sw, on_change, LV_EVENT_VALUE_CHANGED, user_data);
    }

    return sw;
}

lv_obj_t* SettingsBuilder::action(const char* label, const char* value_text,
                                   lv_event_cb_t on_click, void* user_data) {
    lv_obj_t* row = make_row(label);
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(row, LV_OPA_50, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(row, COL_ACTIVE, LV_STATE_PRESSED);

    // Value + arrow on right
    lv_obj_t* right_cont = lv_obj_create(row);
    lv_obj_set_size(right_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(right_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right_cont, 0, 0);
    lv_obj_set_style_pad_all(right_cont, 0, 0);
    lv_obj_set_flex_flow(right_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(right_cont, 5, 0);
    lv_obj_clear_flag(right_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(right_cont, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* val_lbl = lv_label_create(right_cont);
    lv_label_set_text(val_lbl, value_text);
    lv_obj_set_style_text_font(val_lbl, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(val_lbl, COL_TEXT, 0);

    lv_obj_t* arrow = lv_label_create(right_cont);
    lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(arrow, COL_TEXT, 0);

    if (on_click) {
        lv_obj_add_event_cb(row, on_click, LV_EVENT_CLICKED, user_data);
    }

    return row;
}

lv_obj_t* SettingsBuilder::button(const char* label, lv_event_cb_t on_click, void* user_data) {
    lv_obj_t* parent = section_cont_ ? section_cont_ : cont_;

    // Row container (centered)
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), SETTINGS_ROW_H);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    // Create button
    lv_obj_t* btn = lv_button_create(row);
    lv_obj_set_size(btn, LV_SIZE_CONTENT, 28);
    lv_obj_set_style_pad_hor(btn, 20, 0);
    lv_obj_set_style_bg_color(btn, COL_ACTIVE, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(GUI_COLOR_TRIAD[0]), LV_STATE_PRESSED);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, label);
    lv_obj_set_style_text_font(lbl, FONT_DEFAULT, 0);
    lv_obj_center(lbl);

    if (on_click) {
        lv_obj_add_event_cb(btn, on_click, LV_EVENT_CLICKED, user_data);
    }

    return btn;
}

lv_obj_t* SettingsBuilder::toggle_button(const char* label, const char* btn_text,
                                          lv_event_cb_t on_click, void* user_data) {
    lv_obj_t* row = make_row(label);

    // Create button on the right side
    lv_obj_t* btn = lv_button_create(row);
    lv_obj_set_size(btn, LV_SIZE_CONTENT, 26);
    lv_obj_set_style_pad_hor(btn, 12, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(GUI_COLOR_GRAYS[6]), LV_STATE_DEFAULT);  // Gray default
    lv_obj_set_style_bg_color(btn, lv_color_hex(GUI_COLOR_GRAYS[4]), LV_STATE_PRESSED);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, btn_text);
    lv_obj_set_style_text_font(lbl, FONT_DEFAULT, 0);
    lv_obj_center(lbl);

    if (on_click) {
        lv_obj_add_event_cb(btn, on_click, LV_EVENT_CLICKED, user_data);
    }

    return btn;
}

void SettingsBuilder::info(const char* label, const char* value) {
    lv_obj_t* row = make_row(label);

    lv_obj_t* val_lbl = lv_label_create(row);
    lv_label_set_text(val_lbl, value);
    lv_obj_set_style_text_font(val_lbl, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(val_lbl, lv_color_hex(GUI_COLOR_SHADES[10]), 0);
}
