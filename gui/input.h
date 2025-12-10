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
    INPUT_ENC_TRIPLE_CLICK, // Triple click (<300ms between clicks)

    INPUT_COUNT
};

// =============================================================================
// Focus Order Helper
// =============================================================================
// Maximum widgets per page that can be in focus order
constexpr int MAX_FOCUS_WIDGETS = 20;

// Focus color (green) - use this in pages for consistent styling
constexpr uint32_t FOCUS_COLOR_HEX = 0x00AA00;

// Callback type for long-press handler
typedef void (*long_press_cb_t)();

// Callback type for encoder rotation handler
// Return true if the rotation was handled, false to use default focus navigation
typedef bool (*encoder_rotation_cb_t)(int delta);

// Helper struct to build focus groups with specific order
struct FocusOrderBuilder {
    lv_group_t* group;
    lv_obj_t* widgets[MAX_FOCUS_WIDGETS];
    int count;
    int current_focus;  // Current focus index in our order
    long_press_cb_t on_long_press;  // Optional long-press callback
    encoder_rotation_cb_t on_encoder_rotation;  // Optional rotation handler (return true if handled)

    // Initialize with a new group
    void init();

    // Add widget at specific position (0-based index in focus order)
    // Returns the widget for chaining
    lv_obj_t* add(lv_obj_t* widget, int order_index);

    // Finalize: add all widgets to group in order, apply focus style
    void finalize();

    // Navigate to next/previous widget in our defined order
    void focus_next();
    void focus_prev();

    // Focus a specific index
    void focus_index(int idx);

    // Get currently focused widget
    lv_obj_t* get_focused_widget();

    // Get current focus index
    int get_focus_index() const { return current_focus; }

    // Set long-press callback for this page
    void set_long_press_cb(long_press_cb_t cb);

    // Set encoder rotation callback (return true from callback if handled)
    void set_encoder_rotation_cb(encoder_rotation_cb_t cb);

    // Apply focus style to a widget (green outline)
    static void apply_focus_style(lv_obj_t* widget);

    // Cleanup
    void destroy();
};

// Get the active focus builder (for encoder callback to use)
FocusOrderBuilder* input_get_active_focus_builder();

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
// Navigation History (for back/home gestures)
// =============================================================================

// Record page navigation (call when entering a new page)
void input_push_page(int page_id);

// Get previous page (-1 if none)
int input_get_previous_page();

// Pop current page from history and return previous page (-1 if none)
int input_pop_page();

// Clear navigation history
void input_clear_history();

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
