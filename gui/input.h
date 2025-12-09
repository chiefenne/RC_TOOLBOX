// gui/input.h - Hardware-agnostic input abstraction with LVGL encoder support
// Touch is primary (handled by LVGL), encoder provides navigation + edit mode
#pragma once

#include "lvgl.h"
#include <stdint.h>

// =============================================================================
// Input Events (from hardware encoder)
// =============================================================================
enum InputEvent {
    INPUT_NONE = 0,

    // Encoder rotation
    INPUT_ENC_CW,           // Clockwise rotation
    INPUT_ENC_CCW,          // Counter-clockwise rotation

    // Encoder button gestures
    INPUT_ENC_PRESS,        // Short press (<500ms)
    INPUT_ENC_LONG_PRESS,   // Long press (>800ms)
    INPUT_ENC_DOUBLE_CLICK, // Double click (<300ms between clicks)

    INPUT_COUNT
};

// =============================================================================
// Encoder Input System
// =============================================================================

// Initialize encoder input system (call once at startup, after lv_init)
void input_init();

// Poll for pending input event (called by platform-specific code)
// Returns INPUT_NONE if no event pending
InputEvent input_poll();

// Get encoder rotation delta since last call (for speed-based acceleration)
// Positive = clockwise, negative = counter-clockwise
int input_get_encoder_delta();

// Get rotation speed (steps per 100ms) for value acceleration
int input_get_rotation_speed();

// =============================================================================
// LVGL Encoder Integration
// =============================================================================

// Get the LVGL encoder input device (for setting focus groups)
lv_indev_t* input_get_encoder_indev();

// Get the default focus group (pages can use this or create their own)
lv_group_t* input_get_default_group();

// Create a new focus group for a page
lv_group_t* input_create_group();

// Set the active focus group for encoder navigation
void input_set_group(lv_group_t* group);

// Add styling for focused widgets (call once during GUI init)
void input_add_focus_style();

// =============================================================================
// Platform-specific functions (implemented in input_hw.cpp / input_sim.cpp)
// =============================================================================

// Called by platform code to feed encoder events
void input_feed_encoder(int delta);           // Rotation
void input_feed_button(InputEvent gesture);   // Press/long/double

// Hardware-specific init and poll (called by input_init/input_poll)
// Implemented in src/input_hw.cpp for ESP32
void input_hw_init();
void input_hw_poll();
