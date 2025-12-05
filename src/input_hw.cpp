// src/input_hw.cpp - ESP32 hardware input handling (encoder + tactile buttons)
// TODO: Implement actual hardware reading with GPIO interrupts/polling

#include "gui/input.h"

#if defined(ESP_PLATFORM) || defined(ARDUINO)

#include <Arduino.h>

// GPIO pin definitions - adjust to your hardware
// #define ENC_PIN_A    GPIO_NUM_xx
// #define ENC_PIN_B    GPIO_NUM_xx
// #define ENC_PIN_SW   GPIO_NUM_xx
// #define BTN_HOME_PIN GPIO_NUM_xx
// #define BTN_ACTION_PIN GPIO_NUM_xx

// Global encoder delta (accessed by gui/input.cpp)
int g_encoder_delta = 0;

// Pending event
static InputEvent pending_event = INPUT_NONE;

// Encoder state
static volatile int encoder_pos = 0;
static int last_encoder_pos = 0;

// Button debounce
static uint32_t last_btn_time = 0;
static const uint32_t DEBOUNCE_MS = 50;

// Long press detection
static uint32_t enc_press_start = 0;
static bool enc_pressed = false;
static const uint32_t LONG_PRESS_MS = 800;

void input_init() {
    g_encoder_delta = 0;
    pending_event = INPUT_NONE;

    // TODO: Configure GPIO pins
    // pinMode(ENC_PIN_A, INPUT_PULLUP);
    // pinMode(ENC_PIN_B, INPUT_PULLUP);
    // pinMode(ENC_PIN_SW, INPUT_PULLUP);
    // pinMode(BTN_HOME_PIN, INPUT_PULLUP);
    // pinMode(BTN_ACTION_PIN, INPUT_PULLUP);

    // TODO: Attach encoder interrupt
    // attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), encoder_isr, CHANGE);
}

// TODO: Encoder interrupt service routine
// void IRAM_ATTR encoder_isr() {
//     // Read encoder state and update encoder_pos
// }

InputEvent input_poll() {
    // Check for pending event first
    if (pending_event != INPUT_NONE) {
        InputEvent ev = pending_event;
        pending_event = INPUT_NONE;
        return ev;
    }

    // Check encoder rotation
    int pos = encoder_pos;
    if (pos != last_encoder_pos) {
        int delta = pos - last_encoder_pos;
        last_encoder_pos = pos;
        g_encoder_delta = delta * 10;  // Scale to PWM units
        return (delta > 0) ? INPUT_ENC_CW : INPUT_ENC_CCW;
    }

    // TODO: Check encoder button (with long press detection)
    // if (!digitalRead(ENC_PIN_SW)) {
    //     if (!enc_pressed) {
    //         enc_pressed = true;
    //         enc_press_start = millis();
    //     } else if (millis() - enc_press_start > LONG_PRESS_MS) {
    //         enc_pressed = false;
    //         return INPUT_ENC_LONG_PRESS;
    //     }
    // } else if (enc_pressed) {
    //     enc_pressed = false;
    //     if (millis() - enc_press_start < LONG_PRESS_MS) {
    //         return INPUT_ENC_PRESS;
    //     }
    // }

    // TODO: Check tactile buttons (with debounce)
    // if (!digitalRead(BTN_HOME_PIN) && millis() - last_btn_time > DEBOUNCE_MS) {
    //     last_btn_time = millis();
    //     return INPUT_BTN_HOME;
    // }

    return INPUT_NONE;
}

#endif // ESP_PLATFORM || ARDUINO
