// gui/input.cpp - LVGL encoder input device integration
// Platform-specific reading is in src/input_hw.cpp (ESP32) and simulator/input_sim.cpp

#include "gui/input.h"
#include "gui/color_palette.h"
#include "gui/gui.h"

// =============================================================================
// Encoder State (fed by platform-specific code)
// =============================================================================
static int encoder_diff = 0;           // Accumulated rotation delta
static InputEvent pending_gesture = INPUT_NONE;  // Button gesture
static uint32_t last_rotation_time = 0;
static int rotation_count = 0;         // Rotations in time window

// =============================================================================
// Navigation Focus Hint (preserve footer button focus across page transitions)
// =============================================================================
static NavFocusHint nav_focus_hint = NAV_FOCUS_NONE;

void input_set_nav_focus_hint(NavFocusHint hint) {
    nav_focus_hint = hint;
}

NavFocusHint input_get_nav_focus_hint() {
    NavFocusHint hint = nav_focus_hint;
    nav_focus_hint = NAV_FOCUS_NONE;  // Clear after reading
    return hint;
}

// =============================================================================
// Navigation History
// =============================================================================
static constexpr int NAV_HISTORY_SIZE = 8;
static int nav_history[NAV_HISTORY_SIZE] = {-1};
static int nav_history_idx = 0;

// =============================================================================
// LVGL Encoder Input Device
// =============================================================================
static lv_indev_t* encoder_indev = nullptr;
static lv_group_t* default_group = nullptr;

// Active focus builder (set by FocusOrderBuilder::finalize)
static FocusOrderBuilder* active_focus_builder = nullptr;

// Focus style - not used, each widget gets its own style
static lv_style_t style_focus;
static bool style_initialized = false;

// Get active focus builder
FocusOrderBuilder* input_get_active_focus_builder() {
    return active_focus_builder;
}

// LVGL encoder read callback - only used for minimal LVGL integration
// We handle everything ourselves
static void encoder_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    (void)indev;

    // Never pass rotation to LVGL - we manage focus completely ourselves
    data->enc_diff = 0;

    // Don't let LVGL handle buttons either - we do it ourselves
    data->state = LV_INDEV_STATE_RELEASED;
    data->key = 0;
}

// Handle button press - send click event directly to focused widget
static void handle_button_press(InputEvent gesture) {
    switch (gesture) {
        case INPUT_ENC_PRESS:
            // Short press - click the focused widget or exit edit mode
            if (active_focus_builder) {
                // If in edit mode, exit it
                if (active_focus_builder->is_edit_mode()) {
                    active_focus_builder->set_edit_mode(false);
                    // Close dropdown if open
                    lv_obj_t* focused = active_focus_builder->get_focused_widget();
                    if (focused && lv_obj_check_type(focused, &lv_dropdown_class)) {
                        lv_dropdown_close(focused);
                    }
                    break;
                }

                lv_obj_t* focused = active_focus_builder->get_focused_widget();
                if (focused) {
                    // Check widget type to determine behavior
                    if (lv_obj_check_type(focused, &lv_dropdown_class)) {
                        // Enter edit mode for dropdowns
                        active_focus_builder->set_edit_mode(true);
                        lv_dropdown_open(focused);
                    } else if (lv_obj_check_type(focused, &lv_slider_class)) {
                        // Toggle edit mode for sliders
                        active_focus_builder->set_edit_mode(true);
                        lv_obj_add_state(focused, LV_STATE_EDITED);
                    } else {
                        // Regular click for other widgets (buttons, section headers)
                        lv_obj_send_event(focused, LV_EVENT_CLICKED, nullptr);
                    }
                }
            }
            break;

        case INPUT_ENC_LONG_PRESS:
            // Long press - call page-specific callback if set
            if (active_focus_builder && active_focus_builder->on_long_press) {
                active_focus_builder->on_long_press();
            }
            break;

        case INPUT_ENC_DOUBLE_CLICK:
            // Double click - exit edit mode first, then let page handle or go back
            if (active_focus_builder) {
                if (active_focus_builder->is_edit_mode()) {
                    active_focus_builder->set_edit_mode(false);
                    // Close dropdown if open
                    lv_obj_t* focused = active_focus_builder->get_focused_widget();
                    if (focused) {
                        if (lv_obj_check_type(focused, &lv_dropdown_class)) {
                            lv_dropdown_close(focused);
                        } else if (lv_obj_check_type(focused, &lv_slider_class)) {
                            lv_obj_clear_state(focused, LV_STATE_EDITED);
                        }
                    }
                    break;
                }
                if (active_focus_builder->on_double_click) {
                    if (active_focus_builder->on_double_click()) {
                        break;  // Page handled it
                    }
                }
            }
            gui_go_back();
            break;

        case INPUT_ENC_TRIPLE_CLICK:
            // Triple click - go to home page
            gui_set_page(PAGE_HOME);
            break;

        default:
            break;
    }
}

// =============================================================================
// Public API
// =============================================================================

// Weak definitions for platforms without hardware encoder (e.g., simulator)
__attribute__((weak)) void input_hw_init() {}
__attribute__((weak)) void input_hw_poll() {}

void input_init() {
    encoder_diff = 0;
    pending_gesture = INPUT_NONE;
    rotation_count = 0;

    // Create LVGL encoder input device - only for button press handling
    encoder_indev = lv_indev_create();
    lv_indev_set_type(encoder_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(encoder_indev, encoder_read_cb);

    // Create default focus group
    default_group = lv_group_create();
    lv_indev_set_group(encoder_indev, default_group);

    // Initialize focus style
    input_add_focus_style();

    // Initialize platform-specific hardware (ESP32 encoder, etc.)
    input_hw_init();
}

void input_add_focus_style() {
    if (style_initialized) return;
    style_initialized = true;

    lv_style_init(&style_focus);

    // NOTE: Focus style is now applied per-widget in apply_focus_style()
    // This global style is kept for backward compatibility but not actively used
}

lv_indev_t* input_get_encoder_indev() {
    return encoder_indev;
}

lv_group_t* input_get_default_group() {
    return default_group;
}

lv_group_t* input_create_group() {
    lv_group_t* group = lv_group_create();
    return group;
}

void input_set_group(lv_group_t* group) {
    if (encoder_indev && group) {
        lv_indev_set_group(encoder_indev, group);
    }
}

// =============================================================================
// Platform Interface (called by input_hw.cpp / input_sim.cpp)
// =============================================================================

void input_feed_encoder(int delta) {
    // First, check if we're in edit mode (adjusting a widget value)
    if (active_focus_builder && active_focus_builder->is_edit_mode()) {
        lv_obj_t* focused = active_focus_builder->get_focused_widget();
        if (focused) {
            if (lv_obj_check_type(focused, &lv_dropdown_class)) {
                // Navigate dropdown options
                uint32_t opt_cnt = lv_dropdown_get_option_cnt(focused);
                int32_t sel = lv_dropdown_get_selected(focused);
                sel += delta;
                if (sel < 0) sel = opt_cnt - 1;
                if (sel >= (int32_t)opt_cnt) sel = 0;
                lv_dropdown_set_selected(focused, sel);
                // Trigger value changed event
                lv_obj_send_event(focused, LV_EVENT_VALUE_CHANGED, nullptr);
            } else if (lv_obj_check_type(focused, &lv_slider_class)) {
                // Adjust slider value
                int32_t val = lv_slider_get_value(focused);
                int32_t min = lv_slider_get_min_value(focused);
                int32_t max = lv_slider_get_max_value(focused);
                // Scale delta based on range
                int step = (max - min > 100) ? 10 : 1;
                val += delta * step;
                if (val < min) val = min;
                if (val > max) val = max;
                lv_slider_set_value(focused, val, LV_ANIM_ON);
                // Trigger value changed event
                lv_obj_send_event(focused, LV_EVENT_VALUE_CHANGED, nullptr);
            }
        }
        // Update speed tracking
        uint32_t now = lv_tick_get();
        if (now - last_rotation_time < 100) {
            rotation_count++;
        } else {
            rotation_count = 1;
        }
        last_rotation_time = now;
        return;
    }

    // Next, check if the page wants to handle encoder rotation itself
    if (active_focus_builder && active_focus_builder->on_encoder_rotation) {
        if (active_focus_builder->on_encoder_rotation(delta)) {
            // Page handled the rotation, update speed tracking and return
            uint32_t now = lv_tick_get();
            if (now - last_rotation_time < 100) {
                rotation_count++;
            } else {
                rotation_count = 1;
            }
            last_rotation_time = now;
            return;
        }
    }

    // Handle rotation using our custom focus order IMMEDIATELY
    // CW rotation (delta > 0) = move to next widget
    if (active_focus_builder) {
        if (delta > 0) {
            for (int i = 0; i < delta; i++) {
                active_focus_builder->focus_next();
            }
        } else if (delta < 0) {
            for (int i = 0; i < -delta; i++) {
                active_focus_builder->focus_prev();
            }
        }
    }

    // Track rotation speed
    uint32_t now = lv_tick_get();
    if (now - last_rotation_time < 100) {
        rotation_count++;
    } else {
        rotation_count = 1;
    }
    last_rotation_time = now;
}

void input_feed_button(InputEvent gesture) {
    if (gesture != INPUT_NONE) {
        // Handle button press directly - don't go through LVGL
        handle_button_press(gesture);
    }
}

int input_get_encoder_delta() {
    int delta = encoder_diff;
    encoder_diff = 0;
    return delta;
}

int input_get_rotation_speed() {
    // Decay speed if no recent rotation
    uint32_t now = lv_tick_get();
    if (now - last_rotation_time > 200) {
        rotation_count = 0;
    }
    return rotation_count;
}

// Poll hardware for encoder events (call from main loop)
InputEvent input_poll() {
    // Poll platform-specific hardware (ESP32 encoder interrupts, etc.)
    input_hw_poll();
    return INPUT_NONE;
}

// =============================================================================
// Navigation History
// =============================================================================

void input_push_page(int page_id) {
    // Don't push duplicates
    if (nav_history_idx > 0 && nav_history[nav_history_idx - 1] == page_id) {
        return;
    }
    if (nav_history_idx < NAV_HISTORY_SIZE) {
        nav_history[nav_history_idx++] = page_id;
    } else {
        // Shift history and add new entry
        for (int i = 0; i < NAV_HISTORY_SIZE - 1; i++) {
            nav_history[i] = nav_history[i + 1];
        }
        nav_history[NAV_HISTORY_SIZE - 1] = page_id;
    }
}

int input_get_previous_page() {
    // Return second-to-last page (current is last)
    if (nav_history_idx >= 2) {
        return nav_history[nav_history_idx - 2];
    }
    return -1;
}

int input_pop_page() {
    // Remove current page from history and return previous page
    if (nav_history_idx >= 2) {
        nav_history_idx--;  // Remove current page
        return nav_history[nav_history_idx - 1];  // Return previous page
    }
    return -1;  // No previous page
}

void input_clear_history() {
    nav_history_idx = 0;
    for (int i = 0; i < NAV_HISTORY_SIZE; i++) {
        nav_history[i] = -1;
    }
}

// =============================================================================
// FocusOrderBuilder Implementation
// =============================================================================

void FocusOrderBuilder::init() {
    group = lv_group_create();
    count = 0;
    current_focus = 0;
    edit_mode = false;
    on_long_press = nullptr;
    on_encoder_rotation = nullptr;
    on_double_click = nullptr;
    for (int i = 0; i < MAX_FOCUS_WIDGETS; i++) {
        widgets[i] = nullptr;
    }
}

void FocusOrderBuilder::set_long_press_cb(long_press_cb_t cb) {
    on_long_press = cb;
}

void FocusOrderBuilder::set_encoder_rotation_cb(encoder_rotation_cb_t cb) {
    on_encoder_rotation = cb;
}

void FocusOrderBuilder::set_double_click_cb(double_click_cb_t cb) {
    on_double_click = cb;
}

lv_obj_t* FocusOrderBuilder::add(lv_obj_t* widget, int order_index) {
    if (order_index >= 0 && order_index < MAX_FOCUS_WIDGETS) {
        widgets[order_index] = widget;
        if (order_index >= count) {
            count = order_index + 1;
        }
        // Apply focus style immediately
        apply_focus_style(widget);
    }
    return widget;
}

void FocusOrderBuilder::finalize() {
    // We manage focus completely ourselves - don't use LVGL groups for navigation
    // The group is still created but not used for focus management

    // Register this as the active focus builder for encoder navigation
    active_focus_builder = this;

    // Check if there's a navigation focus hint (from prev/next button press)
    NavFocusHint hint = input_get_nav_focus_hint();
    lv_obj_t* target_widget = nullptr;

    if (hint == NAV_FOCUS_PREV) {
        target_widget = gui_get_btn_prev();
    } else if (hint == NAV_FOCUS_NEXT) {
        target_widget = gui_get_btn_next();
    }

    // Try to focus the hinted widget, otherwise focus first widget
    if (target_widget && focus_widget(target_widget)) {
        // Successfully focused the navigation button
    } else {
        // Default: focus first widget
        current_focus = 0;
        focus_index(0);
    }
}

bool FocusOrderBuilder::focus_widget(lv_obj_t* widget) {
    // Find the widget in our list and focus it
    for (int i = 0; i < count; i++) {
        if (widgets[i] == widget) {
            focus_index(i);
            return true;
        }
    }
    return false;
}

void FocusOrderBuilder::focus_next() {
    // Clear focus state from current widget
    if (current_focus >= 0 && current_focus < count && widgets[current_focus] != nullptr) {
        lv_obj_clear_state(widgets[current_focus], LV_STATE_FOCUSED);
        lv_obj_clear_state(widgets[current_focus], LV_STATE_EDITED);
    }

    // Find next valid widget (skip nulls, disabled, and those in hidden sections)
    int start = current_focus;
    do {
        current_focus++;
        if (current_focus >= count) {
            current_focus = 0;  // Wrap around
        }
        if (widgets[current_focus] != nullptr) {
            // Skip disabled widgets
            if (lv_obj_has_state(widgets[current_focus], LV_STATE_DISABLED)) {
                continue;
            }

            // Check if widget is in a hidden section by walking up the parent chain
            // looking for a parent with LV_OBJ_FLAG_HIDDEN
            bool in_hidden_section = false;
            lv_obj_t* parent = lv_obj_get_parent(widgets[current_focus]);
            while (parent != nullptr) {
                if (lv_obj_has_flag(parent, LV_OBJ_FLAG_HIDDEN)) {
                    in_hidden_section = true;
                    break;
                }
                parent = lv_obj_get_parent(parent);
            }

            if (!in_hidden_section) {
                // Set focus state directly on the widget
                lv_obj_add_state(widgets[current_focus], LV_STATE_FOCUSED);
                // Scroll to make the focused widget visible (recursive for nested containers)
                lv_obj_scroll_to_view_recursive(widgets[current_focus], LV_ANIM_ON);
                return;
            }
        }
    } while (current_focus != start);  // Prevent infinite loop
}

void FocusOrderBuilder::focus_prev() {
    // Clear focus state from current widget
    if (current_focus >= 0 && current_focus < count && widgets[current_focus] != nullptr) {
        lv_obj_clear_state(widgets[current_focus], LV_STATE_FOCUSED);
        lv_obj_clear_state(widgets[current_focus], LV_STATE_EDITED);
    }

    // Find previous valid widget (skip nulls, disabled, and those in hidden sections)
    int start = current_focus;
    do {
        current_focus--;
        if (current_focus < 0) {
            current_focus = count - 1;  // Wrap around
        }
        if (widgets[current_focus] != nullptr) {
            // Skip disabled widgets
            if (lv_obj_has_state(widgets[current_focus], LV_STATE_DISABLED)) {
                continue;
            }

            // Check if widget is in a hidden section by walking up the parent chain
            bool in_hidden_section = false;
            lv_obj_t* parent = lv_obj_get_parent(widgets[current_focus]);
            while (parent != nullptr) {
                if (lv_obj_has_flag(parent, LV_OBJ_FLAG_HIDDEN)) {
                    in_hidden_section = true;
                    break;
                }
                parent = lv_obj_get_parent(parent);
            }

            if (!in_hidden_section) {
                // Set focus state directly on the widget
                lv_obj_add_state(widgets[current_focus], LV_STATE_FOCUSED);
                // Scroll to make the focused widget visible (recursive for nested containers)
                lv_obj_scroll_to_view_recursive(widgets[current_focus], LV_ANIM_ON);
                return;
            }
        }
    } while (current_focus != start);  // Prevent infinite loop
}

void FocusOrderBuilder::focus_index(int idx) {
    // Clear focus state from current widget
    if (current_focus >= 0 && current_focus < count && widgets[current_focus] != nullptr) {
        lv_obj_clear_state(widgets[current_focus], LV_STATE_FOCUSED);
        lv_obj_clear_state(widgets[current_focus], LV_STATE_EDITED);
    }

    if (idx >= 0 && idx < count && widgets[idx] != nullptr) {
        current_focus = idx;
        // Set focus state directly on the widget - completely bypass LVGL groups
        lv_obj_add_state(widgets[idx], LV_STATE_FOCUSED);
        // Scroll to make the focused widget visible (recursive for nested containers)
        lv_obj_scroll_to_view_recursive(widgets[idx], LV_ANIM_ON);
    }
}

lv_obj_t* FocusOrderBuilder::get_focused_widget() {
    if (current_focus >= 0 && current_focus < count) {
        return widgets[current_focus];
    }
    return nullptr;
}

void FocusOrderBuilder::apply_focus_style(lv_obj_t* widget) {
    // =============================================================================
    // FOCUS STYLE OPTIONS - Change FOCUS_STYLE to test different styles
    // =============================================================================
    // 0 = Green outline (original - outside widget)
    // 1 = Inverted colors (dark bg + white text) - causes flickering
    // 2 = Blue border (inside widget, compact)
    // 3 = Blue left bar accent
    #define FOCUS_STYLE 2

    #if FOCUS_STYLE == 0
    // Option 0: Original green outline (outside widget)
    lv_obj_set_style_outline_color(widget, lv_color_hex(FOCUS_COLOR_HEX), LV_STATE_FOCUSED);
    lv_obj_set_style_outline_width(widget, 3, LV_STATE_FOCUSED);
    lv_obj_set_style_outline_pad(widget, 2, LV_STATE_FOCUSED);
    lv_obj_set_style_outline_opa(widget, LV_OPA_COVER, LV_STATE_FOCUSED);

    #elif FOCUS_STYLE == 1
    // Option 1: Inverted colors
    // Dark background with white text - clear but causes more redraws
    lv_obj_set_style_bg_color(widget, lv_color_hex(0x1C5C8C), LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(widget, LV_OPA_COVER, LV_STATE_FOCUSED);
    lv_obj_set_style_text_color(widget, lv_color_white(), LV_STATE_FOCUSED);
    lv_obj_set_style_outline_width(widget, 0, LV_STATE_FOCUSED);

    #elif FOCUS_STYLE == 2
    // Option 2: Blue border (inside widget - no extra space needed)
    // Uses border instead of outline - stays within widget bounds
    // Set transparent border in default state to prevent layout shift
    lv_obj_set_style_border_width(widget, 3, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(widget, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    // Focused state: show blue border
    lv_obj_set_style_border_color(widget, lv_color_hex(0x1C5C8C), LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(widget, LV_OPA_COVER, LV_STATE_FOCUSED);
    // Edit state: show green border (actively editing slider/dropdown)
    lv_obj_set_style_border_color(widget, lv_color_hex(0x00AA00), LV_STATE_EDITED);
    lv_obj_set_style_border_opa(widget, LV_OPA_COVER, LV_STATE_EDITED);
    // Disable outline
    lv_obj_set_style_outline_width(widget, 0, LV_STATE_FOCUSED);

    #elif FOCUS_STYLE == 3
    // Option 3: Blue left bar accent (modern style)
    lv_obj_set_style_border_color(widget, lv_color_hex(0x1C5C8C), LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(widget, 3, LV_STATE_FOCUSED);
    lv_obj_set_style_border_side(widget, LV_BORDER_SIDE_LEFT, LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(widget, LV_OPA_COVER, LV_STATE_FOCUSED);
    // Disable outline
    lv_obj_set_style_outline_width(widget, 0, LV_STATE_FOCUSED);
    #endif
}

void FocusOrderBuilder::destroy() {
    // Clear focus from current widget before destroying
    if (current_focus >= 0 && current_focus < count && widgets[current_focus] != nullptr) {
        lv_obj_clear_state(widgets[current_focus], LV_STATE_FOCUSED);
    }

    // Clear active builder if it's us
    if (active_focus_builder == this) {
        active_focus_builder = nullptr;
    }
    if (group) {
        lv_group_delete(group);
        group = nullptr;
    }
    count = 0;
    current_focus = 0;
    edit_mode = false;
}
