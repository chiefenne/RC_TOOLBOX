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
            // Short press - click the focused widget
            if (active_focus_builder) {
                lv_obj_t* focused = active_focus_builder->get_focused_widget();
                if (focused) {
                    lv_obj_send_event(focused, LV_EVENT_CLICKED, nullptr);
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
            // Double click - go back one level
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
    lv_style_set_outline_color(&style_focus, lv_color_hex(0x00AA00));  // Green
    lv_style_set_outline_width(&style_focus, 3);
    lv_style_set_outline_pad(&style_focus, 2);
    lv_style_set_outline_opa(&style_focus, LV_OPA_COVER);
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
    // Handle rotation using our custom focus order IMMEDIATELY
    // Note: Direction is inverted to match natural encoder feel
    if (active_focus_builder) {
        if (delta > 0) {
            for (int i = 0; i < delta; i++) {
                active_focus_builder->focus_prev();
            }
        } else if (delta < 0) {
            for (int i = 0; i < -delta; i++) {
                active_focus_builder->focus_next();
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
    on_long_press = nullptr;
    for (int i = 0; i < MAX_FOCUS_WIDGETS; i++) {
        widgets[i] = nullptr;
    }
}

void FocusOrderBuilder::set_long_press_cb(long_press_cb_t cb) {
    on_long_press = cb;
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

    // Set focus to first widget in our defined order
    current_focus = 0;
    focus_index(0);
}

void FocusOrderBuilder::focus_next() {
    // Clear focus state from current widget
    if (current_focus >= 0 && current_focus < count && widgets[current_focus] != nullptr) {
        lv_obj_clear_state(widgets[current_focus], LV_STATE_FOCUSED);
    }

    // Find next valid widget (skip nulls and hidden)
    int start = current_focus;
    do {
        current_focus++;
        if (current_focus >= count) {
            current_focus = 0;  // Wrap around
        }
        // Check if widget exists and is not hidden
        if (widgets[current_focus] != nullptr &&
            !lv_obj_has_flag(widgets[current_focus], LV_OBJ_FLAG_HIDDEN)) {
            // Set focus state directly on the widget - completely bypass LVGL groups
            lv_obj_add_state(widgets[current_focus], LV_STATE_FOCUSED);
            return;
        }
    } while (current_focus != start);  // Prevent infinite loop
}

void FocusOrderBuilder::focus_prev() {
    // Clear focus state from current widget
    if (current_focus >= 0 && current_focus < count && widgets[current_focus] != nullptr) {
        lv_obj_clear_state(widgets[current_focus], LV_STATE_FOCUSED);
    }

    // Find previous valid widget (skip nulls and hidden)
    int start = current_focus;
    do {
        current_focus--;
        if (current_focus < 0) {
            current_focus = count - 1;  // Wrap around
        }
        // Check if widget exists and is not hidden
        if (widgets[current_focus] != nullptr &&
            !lv_obj_has_flag(widgets[current_focus], LV_OBJ_FLAG_HIDDEN)) {
            // Set focus state directly on the widget - completely bypass LVGL groups
            lv_obj_add_state(widgets[current_focus], LV_STATE_FOCUSED);
            return;
        }
    } while (current_focus != start);  // Prevent infinite loop
}

void FocusOrderBuilder::focus_index(int idx) {
    // Clear focus state from current widget
    if (current_focus >= 0 && current_focus < count && widgets[current_focus] != nullptr) {
        lv_obj_clear_state(widgets[current_focus], LV_STATE_FOCUSED);
    }

    if (idx >= 0 && idx < count && widgets[idx] != nullptr) {
        current_focus = idx;
        // Set focus state directly on the widget - completely bypass LVGL groups
        lv_obj_add_state(widgets[idx], LV_STATE_FOCUSED);
    }
}

lv_obj_t* FocusOrderBuilder::get_focused_widget() {
    if (current_focus >= 0 && current_focus < count) {
        return widgets[current_focus];
    }
    return nullptr;
}

void FocusOrderBuilder::apply_focus_style(lv_obj_t* widget) {
    // Green outline when focused
    lv_obj_set_style_outline_color(widget, lv_color_hex(FOCUS_COLOR_HEX), LV_STATE_FOCUSED);
    lv_obj_set_style_outline_width(widget, 3, LV_STATE_FOCUSED);
    lv_obj_set_style_outline_pad(widget, 2, LV_STATE_FOCUSED);
    lv_obj_set_style_outline_opa(widget, LV_OPA_COVER, LV_STATE_FOCUSED);
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
}
