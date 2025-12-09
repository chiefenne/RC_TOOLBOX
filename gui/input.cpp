// gui/input.cpp - LVGL encoder input device integration
// Platform-specific reading is in src/input_hw.cpp (ESP32) and simulator/input_sim.cpp

#include "gui/input.h"
#include "gui/color_palette.h"

// =============================================================================
// Encoder State (fed by platform-specific code)
// =============================================================================
static int encoder_diff = 0;           // Accumulated rotation delta
static InputEvent pending_gesture = INPUT_NONE;  // Button gesture
static uint32_t last_rotation_time = 0;
static int rotation_count = 0;         // Rotations in time window

// =============================================================================
// LVGL Encoder Input Device
// =============================================================================
static lv_indev_t* encoder_indev = nullptr;
static lv_group_t* default_group = nullptr;

// Focus style (blue outline)
static lv_style_t style_focus;
static bool style_initialized = false;

// LVGL encoder read callback
static void encoder_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    (void)indev;

    // Rotation → enc_diff
    data->enc_diff = static_cast<int16_t>(encoder_diff);
    encoder_diff = 0;

    // Button state
    static lv_indev_state_t btn_state = LV_INDEV_STATE_RELEASED;

    switch (pending_gesture) {
        case INPUT_ENC_PRESS:
            // Short press → ENTER (select/activate)
            data->key = LV_KEY_ENTER;
            btn_state = LV_INDEV_STATE_PRESSED;
            pending_gesture = INPUT_NONE;
            break;

        case INPUT_ENC_LONG_PRESS:
            // Long press → also acts as ENTER but pages can check
            data->key = LV_KEY_ENTER;
            btn_state = LV_INDEV_STATE_PRESSED;
            pending_gesture = INPUT_NONE;
            break;

        case INPUT_ENC_DOUBLE_CLICK:
            // Double click → ESC (exit edit mode / go back)
            data->key = LV_KEY_ESC;
            btn_state = LV_INDEV_STATE_PRESSED;
            pending_gesture = INPUT_NONE;
            break;

        default:
            // Release after one cycle
            if (btn_state == LV_INDEV_STATE_PRESSED) {
                btn_state = LV_INDEV_STATE_RELEASED;
            }
            break;
    }

    data->state = btn_state;
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

    // Create LVGL encoder input device
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
    lv_style_set_outline_color(&style_focus, lv_color_hex(GUI_COLOR_MONO[1]));  // Blue
    lv_style_set_outline_width(&style_focus, 2);
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
    encoder_diff += delta;

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
        pending_gesture = gesture;
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
