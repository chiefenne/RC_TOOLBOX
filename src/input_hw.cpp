// src/input_hw.cpp - ESP32 hardware input handling (EC11 rotary encoder)
// Uses interrupts for responsive encoder reading with button gesture detection

#include "gui/input.h"

#if defined(ESP_PLATFORM) || defined(ARDUINO)

#include <Arduino.h>
#include "pins.h"

// =============================================================================
// Encoder State (interrupt-safe)
// =============================================================================
static volatile int32_t encoder_pos = 0;
static int32_t last_encoder_pos = 0;

// Button state machine
static volatile uint32_t btn_press_time = 0;
static volatile uint32_t btn_release_time = 0;
static volatile bool btn_pressed = false;
static volatile int click_count = 0;

// Timing constants (ms)
static constexpr uint32_t DEBOUNCE_MS     = 5;
static constexpr uint32_t LONG_PRESS_MS   = 800;
static constexpr uint32_t DOUBLE_CLICK_MS = 300;

// Last encoder state for quadrature decoding
static volatile uint8_t last_enc_state = 0;

// =============================================================================
// Interrupt Service Routines
// =============================================================================

// Quadrature encoder ISR - called on CLK (A) pin change
static void IRAM_ATTR encoder_isr() {
    uint8_t clk = digitalRead(PIN_ENC_CLK);
    uint8_t dt  = digitalRead(PIN_ENC_DT);

    // Build 2-bit state and combine with previous state
    uint8_t state = (clk << 1) | dt;
    uint8_t combined = (last_enc_state << 2) | state;
    last_enc_state = state;

    // Decode rotation direction using state transition table
    // Valid CW transitions:  0b0001, 0b0111, 0b1110, 0b1000
    // Valid CCW transitions: 0b0010, 0b1011, 0b1101, 0b0100
    switch (combined) {
        case 0b0001: case 0b0111: case 0b1110: case 0b1000:
            encoder_pos++;
            break;
        case 0b0010: case 0b1011: case 0b1101: case 0b0100:
            encoder_pos--;
            break;
    }
}

// Button ISR - called on SW pin change
static void IRAM_ATTR button_isr() {
    uint32_t now = millis();
    bool pressed = (digitalRead(PIN_ENC_SW) == LOW);

    if (pressed && !btn_pressed) {
        // Button just pressed
        if (now - btn_release_time > DEBOUNCE_MS) {
            btn_pressed = true;
            btn_press_time = now;
        }
    } else if (!pressed && btn_pressed) {
        // Button just released
        btn_pressed = false;
        btn_release_time = now;

        uint32_t press_duration = now - btn_press_time;
        if (press_duration < LONG_PRESS_MS) {
            // Short press - could be first click of double-click
            click_count++;
        }
    }
}

// =============================================================================
// Platform-specific functions (called by gui/input.cpp)
// =============================================================================

void input_hw_init() {
    // Configure encoder pins with internal pull-ups
    pinMode(PIN_ENC_CLK, INPUT_PULLUP);
    pinMode(PIN_ENC_DT, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    // Read initial encoder state
    last_enc_state = (digitalRead(PIN_ENC_CLK) << 1) | digitalRead(PIN_ENC_DT);

    // Attach interrupts
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), encoder_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_DT), encoder_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_SW), button_isr, CHANGE);

    encoder_pos = 0;
    last_encoder_pos = 0;
    click_count = 0;
}

void input_hw_poll() {
    // Check for encoder rotation
    int32_t pos = encoder_pos;
    if (pos != last_encoder_pos) {
        int delta = pos - last_encoder_pos;
        last_encoder_pos = pos;

        // Feed rotation to LVGL encoder system
        input_feed_encoder(delta);
    }

    // Check for long press (while button still held)
    if (btn_pressed) {
        uint32_t now = millis();
        if (now - btn_press_time > LONG_PRESS_MS) {
            // Long press detected - reset to avoid repeat
            btn_press_time = now;
            input_feed_button(INPUT_ENC_LONG_PRESS);
        }
    }

    // Check for click gestures (after release, with timeout for double-click)
    if (click_count > 0 && !btn_pressed) {
        uint32_t now = millis();
        if (now - btn_release_time > DOUBLE_CLICK_MS) {
            // Timeout - process accumulated clicks
            InputEvent ev = INPUT_NONE;
            if (click_count >= 2) {
                ev = INPUT_ENC_DOUBLE_CLICK;
            } else {
                ev = INPUT_ENC_PRESS;
            }
            click_count = 0;
            input_feed_button(ev);
        }
    }
}

#endif // ESP_PLATFORM || ARDUINO
