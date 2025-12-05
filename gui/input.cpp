// gui/input.cpp - Input event processing (shared between platforms)
// Platform-specific input reading is in src/input_hw.cpp (ESP32) and simulator/input_sim.cpp

#include "gui/input.h"
#include "gui/gui.h"
#include "gui/pages/page_servo.h"

// External: platform-specific encoder delta (set by input_hw.cpp or input_sim.cpp)
extern int g_encoder_delta;

// Current page needs to be accessible for context-specific actions
extern GuiPage gui_get_current_page();

void input_process(InputEvent ev) {
    if (ev == INPUT_NONE) return;

    switch (ev) {
        case INPUT_BTN_HOME:
            gui_set_page(PAGE_HOME);
            break;

        case INPUT_BTN_BACK:
            // Go back to home from any page
            gui_set_page(PAGE_HOME);
            break;

        case INPUT_ENC_CW:
        case INPUT_ENC_CCW: {
            // Context-specific: adjust values on current page
            GuiPage page = gui_get_current_page();
            int delta = (ev == INPUT_ENC_CW) ? 10 : -10;

            if (page == PAGE_SERVO) {
                page_servo_adjust_pwm(delta);
            }
            // Add other pages as needed
            break;
        }

        case INPUT_ENC_PRESS:
            // Context-specific confirm action
            // Could toggle start/stop on servo page, etc.
            break;

        case INPUT_ENC_LONG_PRESS:
            // Back to home
            gui_set_page(PAGE_HOME);
            break;

        case INPUT_BTN_ACTION: {
            // Context-specific action button
            GuiPage page = gui_get_current_page();
            if (page == PAGE_SERVO) {
                page_servo_toggle_sweep();  // Toggle auto sweep
            }
            break;
        }

        default:
            break;
    }
}

int input_get_encoder_delta() {
    int delta = g_encoder_delta;
    g_encoder_delta = 0;
    return delta;
}
