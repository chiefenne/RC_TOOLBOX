// gui/input.h - Hardware-agnostic input abstraction
// Touch is primary (handled by LVGL), encoder/buttons are secondary shortcuts
#pragma once

#include <stdint.h>

// Input events from encoder and tactile buttons
enum InputEvent {
    INPUT_NONE = 0,

    // Encoder events
    INPUT_ENC_CW,           // Clockwise rotation (increase value)
    INPUT_ENC_CCW,          // Counter-clockwise rotation (decrease value)
    INPUT_ENC_PRESS,        // Short press (confirm/enter)
    INPUT_ENC_LONG_PRESS,   // Long press (back/cancel)

    // Tactile button events (directly mapped to actions)
    INPUT_BTN_HOME,         // Go to home page
    INPUT_BTN_BACK,         // Go back / cancel
    INPUT_BTN_ACTION,       // Context-specific action (start/stop, etc.)

    // Future expansion
    INPUT_BTN_4,
    INPUT_BTN_5,
    INPUT_BTN_6,

    INPUT_COUNT
};

// Initialize input system (call once at startup)
void input_init();

// Poll for pending input event (call from main loop)
// Returns INPUT_NONE if no event pending
InputEvent input_poll();

// Get encoder delta since last call (for fine adjustment)
// Positive = clockwise, negative = counter-clockwise
int input_get_encoder_delta();

// Process an input event (call from main loop after input_poll)
// This dispatches to the appropriate GUI action
void input_process(InputEvent ev);
