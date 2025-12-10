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
static volatile bool long_press_fired = false;  // Suppress click after long press

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

    // NOTE: Using polling instead of interrupts - more reliable on ESP32-S3 GPIO 35/36
    // attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), encoder_isr, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(PIN_ENC_DT), encoder_isr, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(PIN_ENC_SW), button_isr, CHANGE);

    encoder_pos = 0;
    last_encoder_pos = 0;
    click_count = 0;
    btn_pressed = false;
}

// Poll encoder state (called from input_hw_poll)
// EC11 encoder: mechanical with 20 detents per revolution, 4 state changes per detent
// Uses a robust state machine that only counts complete detent cycles
static void poll_encoder() {
    static int8_t enc_delta = 0;           // Accumulated partial steps
    static uint8_t last_stable_state = 0;  // Last confirmed stable state
    static uint32_t state_start_time = 0;  // When current state was entered
    static constexpr uint32_t STABLE_TIME_MS = 1;  // State must be stable for 1ms

    uint8_t clk = digitalRead(PIN_ENC_CLK);
    uint8_t dt  = digitalRead(PIN_ENC_DT);
    uint8_t state = (clk << 1) | dt;
    uint32_t now = millis();

    // Only process if state changed
    if (state == last_enc_state) {
        return;
    }

    // State changed - check if previous state was stable long enough
    if (now - state_start_time < STABLE_TIME_MS) {
        // Previous state wasn't stable - this is likely bounce, ignore
        // But update the state tracking
        last_enc_state = state;
        state_start_time = now;
        return;
    }

    // Previous state was stable, now we have a valid transition
    uint8_t combined = (last_enc_state << 2) | state;

    // Full quadrature state machine - accumulate steps
    // EC11 goes through sequence: 11 -> 01 -> 00 -> 10 -> 11 (CW)
    //                         or: 11 -> 10 -> 00 -> 01 -> 11 (CCW)
    switch (combined) {
        // CW transitions
        case 0b1101:  // 11 -> 01
        case 0b0100:  // 01 -> 00
        case 0b0010:  // 00 -> 10
        case 0b1011:  // 10 -> 11
            enc_delta++;
            break;
        // CCW transitions
        case 0b1110:  // 11 -> 10
        case 0b1000:  // 10 -> 00
        case 0b0001:  // 00 -> 01
        case 0b0111:  // 01 -> 11
            enc_delta--;
            break;
    }

    // Count one step per complete detent (4 transitions = 1 detent)
    // We trigger when returning to state 11 (both pins HIGH at detent)
    if (state == 0b11) {
        if (enc_delta >= 2) {
            encoder_pos++;
            enc_delta = 0;
        } else if (enc_delta <= -2) {
            encoder_pos--;
            enc_delta = 0;
        }
        // Reset partial counts if we're back at detent without enough movement
        // This handles cases where user moves slightly and returns
        if (enc_delta > -2 && enc_delta < 2) {
            enc_delta = 0;
        }
    }

    last_enc_state = state;
    state_start_time = now;
}

// Poll button state (called from input_hw_poll)
static void poll_button() {
    uint32_t now = millis();
    bool pressed = (digitalRead(PIN_ENC_SW) == LOW);

    if (pressed && !btn_pressed) {
        // Button just pressed - debounce
        static uint32_t last_press = 0;
        if (now - last_press > DEBOUNCE_MS) {
            btn_pressed = true;
            btn_press_time = now;
            long_press_fired = false;  // Reset on new press
            last_press = now;
        }
    } else if (!pressed && btn_pressed) {
        // Button just released
        btn_pressed = false;
        btn_release_time = now;

        // Only count as click if long press wasn't already fired
        if (!long_press_fired) {
            uint32_t press_duration = now - btn_press_time;
            if (press_duration < LONG_PRESS_MS) {
                // Short press - could be first click of double-click
                click_count++;
            }
        }
        long_press_fired = false;  // Reset for next press
    }
}

void input_hw_poll() {
    // Poll encoder and button (instead of using interrupts)
    poll_encoder();
    poll_button();

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
        if (now - btn_press_time > LONG_PRESS_MS && !long_press_fired) {
            // Long press detected - set flag to prevent click on release
            long_press_fired = true;
            input_feed_button(INPUT_ENC_LONG_PRESS);
        }
    }

    // Check for click gestures (after release, with timeout for double-click)
    if (click_count > 0 && !btn_pressed) {
        uint32_t now = millis();
        if (now - btn_release_time > DOUBLE_CLICK_MS) {
            // Timeout - process accumulated clicks
            InputEvent ev = INPUT_NONE;
            if (click_count >= 3) {
                ev = INPUT_ENC_TRIPLE_CLICK;
            } else if (click_count >= 2) {
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
