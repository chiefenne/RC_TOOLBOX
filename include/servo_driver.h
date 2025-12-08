// include/servo_driver.h - Servo PWM driver for ESP32 LEDC
// Controls up to 6 servo outputs via hardware PWM

#pragma once

#include <cstdint>

// Servo PWM parameters
constexpr int SERVO_PWM_FREQ_HZ = 50;          // 50 Hz = 20ms period
constexpr int SERVO_PWM_RESOLUTION = 16;       // 16-bit resolution for precise timing
constexpr int SERVO_PULSE_MIN_US = 500;        // Minimum pulse width (microseconds)
constexpr int SERVO_PULSE_MAX_US = 2500;       // Maximum pulse width (microseconds)
constexpr int SERVO_PULSE_CENTER_US = 1500;    // Center position (microseconds)

/**
 * Initialize all servo LEDC channels
 * Must be called once at startup before using servo functions
 */
void servo_driver_init();

/**
 * Set PWM pulse width for a single servo
 * @param servo_idx Servo index (0-5)
 * @param pulse_us Pulse width in microseconds (500-2500)
 */
void servo_set_pulse(uint8_t servo_idx, uint16_t pulse_us);

/**
 * Set PWM pulse width for multiple servos using a bitmask
 * @param mask Bitmask where bit N = servo N (e.g., 0b00101011 = servos 0,1,3,5)
 * @param pulse_us Pulse width in microseconds (500-2500)
 */
void servo_set_pulse_mask(uint8_t mask, uint16_t pulse_us);

/**
 * Enable/disable servo output for a single servo
 * When disabled, no PWM signal is sent (servo can freewheel)
 * @param servo_idx Servo index (0-5)
 * @param enable true to enable, false to disable
 */
void servo_enable(uint8_t servo_idx, bool enable);

/**
 * Enable/disable servo output for multiple servos using a bitmask
 * @param mask Bitmask where bit N = servo N
 * @param enable true to enable, false to disable
 */
void servo_enable_mask(uint8_t mask, bool enable);

/**
 * Disable all servo outputs
 */
void servo_disable_all();

/**
 * Get current pulse width for a servo
 * @param servo_idx Servo index (0-5)
 * @return Current pulse width in microseconds
 */
uint16_t servo_get_pulse(uint8_t servo_idx);
