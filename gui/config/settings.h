// gui/config/settings.h - Settings storage interface
// Stores settings in a JSON file (gui/config/settings.json on simulator, /settings.json on SPIFFS for ESP32)
#pragma once

#include <stdint.h>
#include "gui/gui.h"
#include "gui/lang.h"

// Settings version - increment when adding/changing settings structure
#define SETTINGS_VERSION 3

// Number of servos supported
#define NUM_SERVOS 6

// Default PWM step size (µs per tick)
#define DEFAULT_PWM_STEP 10

// Servo protocol presets
enum ServoProtocol {
    SERVO_STANDARD = 0,   // 1000-1500-2000 @ 50Hz
    SERVO_EXTENDED,       // 500-1500-2500 @ 50Hz
    SERVO_SANWA,          // 920-1520-2120 @ 50Hz
    SERVO_FUTABA,         // 900-1500-2100 @ 50Hz
    SERVO_DIGITAL_FAST,   // 1000-1500-2000 @ 333Hz
    SERVO_CUSTOM,         // User-defined values
    SERVO_PROTOCOL_COUNT
};

// Settings structure with defaults
struct Settings {
    uint8_t version    = SETTINGS_VERSION;
    uint8_t language   = LANG_EN;
    uint8_t bg_color   = BG_COLOR_WHITE;
    uint8_t brightness = 80;

    // Servo settings
    uint8_t  servo_protocol   = SERVO_STANDARD;
    uint16_t servo_pwm_min    = 1000;
    uint16_t servo_pwm_center = 1500;
    uint16_t servo_pwm_max    = 2000;
    uint16_t servo_frequency  = 50;   // Hz

    // Per-servo PWM step size (µs per tick in auto-sweep)
    uint8_t servo_pwm_step[NUM_SERVOS] = {DEFAULT_PWM_STEP, DEFAULT_PWM_STEP, DEFAULT_PWM_STEP,
                                          DEFAULT_PWM_STEP, DEFAULT_PWM_STEP, DEFAULT_PWM_STEP};
};

// Reset all servo PWM steps to default
void servo_reset_pwm_steps();

// Apply servo preset values (updates min/center/max/frequency based on protocol)
void servo_apply_preset(ServoProtocol protocol);

// Global settings instance
extern Settings g_settings;

// Initialize settings (call once at startup, before gui_init)
void settings_init();

// Load settings from JSON file (uses defaults if file not found)
void settings_load();

// Save current settings to JSON file
void settings_save();

// Reset settings to defaults and save
void settings_reset();

// Get path to settings file (for user info)
const char* settings_get_path();
