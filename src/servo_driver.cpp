// src/servo_driver.cpp - ESP32 LEDC-based servo PWM driver

#include "servo_driver.h"
#include "pins.h"

#if defined(ESP_PLATFORM) || defined(ARDUINO)

#include <Arduino.h>
#include <driver/ledc.h>

// Track current pulse widths and enabled state
static uint16_t servo_pulse_us[NUM_SERVO_PINS] = {0};
static bool servo_enabled[NUM_SERVO_PINS] = {false};

// Convert microseconds to LEDC duty cycle value
// At 50Hz with 16-bit resolution: 65536 counts = 20000µs
// duty = pulse_us * 65536 / 20000
static uint32_t pulse_to_duty(uint16_t pulse_us) {
    constexpr uint32_t period_us = 1000000 / SERVO_PWM_FREQ_HZ; // 20000µs at 50Hz
    constexpr uint32_t max_duty = (1 << SERVO_PWM_RESOLUTION) - 1; // 65535
    return (static_cast<uint32_t>(pulse_us) * max_duty) / period_us;
}

void servo_driver_init() {
    for (uint8_t i = 0; i < NUM_SERVO_PINS; i++) {
        // Configure LEDC timer (timers 0-3 available, we use timer 0 for all)
        ledc_timer_config_t timer_conf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = static_cast<ledc_timer_bit_t>(SERVO_PWM_RESOLUTION),
            .timer_num = LEDC_TIMER_0,
            .freq_hz = SERVO_PWM_FREQ_HZ,
            .clk_cfg = LEDC_AUTO_CLK,
            .deconfigure = false
        };
        if (i == 0) { // Only configure timer once
            ledc_timer_config(&timer_conf);
        }

        // Configure LEDC channel
        ledc_channel_config_t channel_conf = {
            .gpio_num = PIN_SERVO[i],
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = static_cast<ledc_channel_t>(SERVO_LEDC_CHANNEL[i]),
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,  // Start with output off
            .hpoint = 0,
            .flags = { .output_invert = 0 }
        };
        ledc_channel_config(&channel_conf);

        servo_pulse_us[i] = SERVO_PULSE_CENTER_US;
        servo_enabled[i] = false;
    }
}

void servo_set_pulse(uint8_t servo_idx, uint16_t pulse_us) {
    if (servo_idx >= NUM_SERVO_PINS) return;

    // Clamp pulse width to valid range
    if (pulse_us < SERVO_PULSE_MIN_US) pulse_us = SERVO_PULSE_MIN_US;
    if (pulse_us > SERVO_PULSE_MAX_US) pulse_us = SERVO_PULSE_MAX_US;

    servo_pulse_us[servo_idx] = pulse_us;

    // Only update hardware if servo is enabled
    if (servo_enabled[servo_idx]) {
        uint32_t duty = pulse_to_duty(pulse_us);
        ledc_set_duty(LEDC_LOW_SPEED_MODE,
                      static_cast<ledc_channel_t>(SERVO_LEDC_CHANNEL[servo_idx]),
                      duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE,
                         static_cast<ledc_channel_t>(SERVO_LEDC_CHANNEL[servo_idx]));
    }
}

void servo_set_pulse_mask(uint8_t mask, uint16_t pulse_us) {
    for (uint8_t i = 0; i < NUM_SERVO_PINS; i++) {
        if (mask & (1 << i)) {
            servo_set_pulse(i, pulse_us);
        }
    }
}

void servo_enable(uint8_t servo_idx, bool enable) {
    if (servo_idx >= NUM_SERVO_PINS) return;

    servo_enabled[servo_idx] = enable;

    if (enable) {
        // Start PWM with current pulse width
        uint32_t duty = pulse_to_duty(servo_pulse_us[servo_idx]);
        ledc_set_duty(LEDC_LOW_SPEED_MODE,
                      static_cast<ledc_channel_t>(SERVO_LEDC_CHANNEL[servo_idx]),
                      duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE,
                         static_cast<ledc_channel_t>(SERVO_LEDC_CHANNEL[servo_idx]));
    } else {
        // Stop PWM output (duty = 0)
        ledc_set_duty(LEDC_LOW_SPEED_MODE,
                      static_cast<ledc_channel_t>(SERVO_LEDC_CHANNEL[servo_idx]),
                      0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE,
                         static_cast<ledc_channel_t>(SERVO_LEDC_CHANNEL[servo_idx]));
    }
}

void servo_enable_mask(uint8_t mask, bool enable) {
    for (uint8_t i = 0; i < NUM_SERVO_PINS; i++) {
        if (mask & (1 << i)) {
            servo_enable(i, enable);
        }
    }
}

void servo_disable_all() {
    for (uint8_t i = 0; i < NUM_SERVO_PINS; i++) {
        servo_enable(i, false);
    }
}

uint16_t servo_get_pulse(uint8_t servo_idx) {
    if (servo_idx >= NUM_SERVO_PINS) return SERVO_PULSE_CENTER_US;
    return servo_pulse_us[servo_idx];
}

#else
// Stub implementation for simulator

void servo_driver_init() {}
void servo_set_pulse(uint8_t, uint16_t) {}
void servo_set_pulse_mask(uint8_t, uint16_t) {}
void servo_enable(uint8_t, bool) {}
void servo_enable_mask(uint8_t, bool) {}
void servo_disable_all() {}
uint16_t servo_get_pulse(uint8_t) { return 1500; }

#endif
