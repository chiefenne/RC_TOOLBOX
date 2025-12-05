// gui/config/settings.h - Settings storage interface
// Stores settings in a JSON file (gui/config/settings.json on simulator, /settings.json on SPIFFS for ESP32)
#pragma once

#include <stdint.h>
#include "gui/gui.h"
#include "gui/lang.h"

// Settings version - increment when adding/changing settings structure
#define SETTINGS_VERSION 1

// Settings structure with defaults
struct Settings {
    uint8_t version    = SETTINGS_VERSION;
    uint8_t language   = LANG_EN;
    uint8_t bg_color   = BG_COLOR_LIGHT_GRAY;
    uint8_t brightness = 80;
};

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
