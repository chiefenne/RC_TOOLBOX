// src/input_hw.cpp - ESP32 hardware input handling (EC11 rotary encoder)
// Uses interrupts for responsive encoder reading with button gesture detection

#include "gui/input.h"

#if defined(ESP_PLATFORM) || defined(ARDUINO)

#include <Arduino.h>
#include "pins.h"

// Uncomment to enable encoder debug output
// #define DEBUG_ENCODER

// =============================================================================
// Encoder State (interrupt-safe)
// =============================================================================
static volatile int32_t encoder_pos = 0;
static int32_t last_encoder_pos = 0;

// Encoder state machine (in ISR)
static volatile uint8_t enc_state = 0;
static volatile int8_t enc_count = 0;  // Accumulated counts within detent

// Button state machine
static volatile uint32_t btn_press_time = 0;
static volatile uint32_t btn_release_time = 0;
static volatile bool btn_pressed = false;
static volatile int click_count = 0;
static volatile bool long_press_fired = false;  // Suppress click after long press

// Debug: ISR activity counter and last sampled states (can't print from ISR, but can store)
static volatile uint32_t isr_count = 0;
static volatile uint8_t last_isr_clk = 0;
static volatile uint8_t last_isr_dt = 0;
static volatile uint8_t last_isr_state = 0;
static volatile int8_t last_isr_dir = 0;
static volatile int8_t last_isr_count = 0;

// Timing constants (ms)
static constexpr uint32_t DEBOUNCE_MS     = 5;
static constexpr uint32_t LONG_PRESS_MS   = 800;
static constexpr uint32_t DOUBLE_CLICK_MS = 300;

// =============================================================================
// Interrupt Service Routine for Encoder
// =============================================================================
// Gray code state machine - bulletproof decoding
// States: 00=0, 01=1, 11=2, 10=3 (Gray code order)
// CW sequence:  0->1->2->3->0 (decimal: 0,1,3,2,0)
// CCW sequence: 0->3->2->1->0 (decimal: 0,2,3,1,0)

static const int8_t ENC_STATES[] = {
//  00  01  10  11  <- new state
     0, -1,  1,  0,  // old state 00
     1,  0,  0, -1,  // old state 01
    -1,  0,  0,  1,  // old state 10
     0,  1, -1,  0   // old state 11
};

static void IRAM_ATTR encoder_isr() {
    isr_count++;  // Debug: count ISR invocations

    // Read both pins
    uint8_t clk = digitalRead(PIN_ENC_CLK);
    uint8_t dt  = digitalRead(PIN_ENC_DT);
    uint8_t new_state = (clk << 1) | dt;

    // Look up direction from state transition table
    int8_t dir = ENC_STATES[(enc_state << 2) | new_state];
    enc_state = new_state;

    // Debug: store last sampled values
    last_isr_clk = clk;
    last_isr_dt = dt;
    last_isr_state = new_state;
    last_isr_dir = dir;

    if (dir != 0) {
        enc_count += dir;
        last_isr_count = enc_count;

        // EC11 has 4 state changes per detent
        // Count one step when we've accumulated 4 transitions in same direction
        // Or when we return to detent position (state 11) with enough counts
        if (new_state == 0b11) {  // At detent position
            if (enc_count >= 2) {
                encoder_pos++;
                enc_count = 0;
            } else if (enc_count <= -2) {
                encoder_pos--;
                enc_count = 0;
            } else {
                // Small oscillation at detent, reset
                enc_count = 0;
            }
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

#ifdef DEBUG_ENCODER
    Serial.printf("[ENC-INIT] Pins: CLK=%d DT=%d SW=%d\n", PIN_ENC_CLK, PIN_ENC_DT, PIN_ENC_SW);
    Serial.printf("[ENC-INIT] Raw pin state: CLK=%d DT=%d SW=%d (SW expect 1=HIGH)\n",
                  digitalRead(PIN_ENC_CLK), digitalRead(PIN_ENC_DT), digitalRead(PIN_ENC_SW));
#endif

    // Read initial encoder state
    enc_state = (digitalRead(PIN_ENC_CLK) << 1) | digitalRead(PIN_ENC_DT);
    enc_count = 0;

    // Attach interrupts on BOTH encoder pins for full quadrature decoding
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), encoder_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_DT), encoder_isr, CHANGE);

    encoder_pos = 0;
    last_encoder_pos = 0;
    click_count = 0;
    btn_pressed = false;
    btn_release_time = millis();  // Initialize to prevent stale timeout
    isr_count = 0;

#ifdef DEBUG_ENCODER
    Serial.printf("[ENC-INIT] Complete: enc_state=0x%02X\n", enc_state);
#endif
}

// Poll button state (called from input_hw_poll)
// Button uses polling with debounce (interrupts not needed for slow human input)
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
#ifdef DEBUG_ENCODER
            Serial.printf("[BTN] PRESS detected at %lu\n", now);
#endif
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
#ifdef DEBUG_ENCODER
            Serial.printf("[BTN] RELEASE duration=%lu click_count=%d long_fired=%d\n",
                          press_duration, click_count, long_press_fired);
        } else {
            Serial.printf("[BTN] RELEASE after long press, ignoring\n");
#endif
        }
        long_press_fired = false;  // Reset for next press
    }
}

void input_hw_poll() {
#ifdef DEBUG_ENCODER
    // Periodic raw state dump
    static uint32_t last_dbg = 0;
    if (millis() - last_dbg > 500) {
        last_dbg = millis();
        noInterrupts();
        uint32_t isr_cnt = isr_count;
        int32_t pos_dbg = encoder_pos;
        uint8_t dbg_clk = last_isr_clk;
        uint8_t dbg_dt = last_isr_dt;
        uint8_t dbg_state = last_isr_state;
        int8_t dbg_dir = last_isr_dir;
        int8_t dbg_count = last_isr_count;
        interrupts();
        Serial.printf("[ENC-POLL] RAW: CLK=%d DT=%d SW=%d pos=%ld isr=%lu | last: CLK=%d DT=%d state=0x%02X dir=%d cnt=%d\n",
                      digitalRead(PIN_ENC_CLK), digitalRead(PIN_ENC_DT),
                      digitalRead(PIN_ENC_SW), pos_dbg, isr_cnt,
                      dbg_clk, dbg_dt, dbg_state, dbg_dir, dbg_count);
    }
#endif

    // Button is polled (slow human input, debounce needed)
    poll_button();

    // Encoder rotation is handled by ISR - just check for changes
    // Use noInterrupts/interrupts to safely read volatile
    noInterrupts();
    int32_t pos = encoder_pos;
    interrupts();

    if (pos != last_encoder_pos) {
        int delta = pos - last_encoder_pos;
        last_encoder_pos = pos;

#ifdef DEBUG_ENCODER
        Serial.printf("[ENC] ROTATION delta=%d pos=%ld\n", delta, pos);
#endif
        // Feed rotation to LVGL encoder system
        input_feed_encoder(delta);
    }

    // Check for long press (while button still held)
    if (btn_pressed) {
        uint32_t now = millis();
        if (now - btn_press_time > LONG_PRESS_MS && !long_press_fired) {
            // Long press detected - set flag to prevent click on release
            long_press_fired = true;
#ifdef DEBUG_ENCODER
            Serial.printf("[BTN] LONG_PRESS fired at %lu\n", now);
#endif
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
#ifdef DEBUG_ENCODER
            Serial.printf("[BTN] FIRE event=%d clicks=%d\n", ev, click_count);
#endif
            click_count = 0;
            input_feed_button(ev);
        }
    }
}

#endif // ESP_PLATFORM || ARDUINO
