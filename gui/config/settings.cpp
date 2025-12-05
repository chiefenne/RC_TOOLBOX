// gui/config/settings.cpp - Settings storage implementation
// Platform-agnostic JSON file handling

#include "gui/config/settings.h"
#include <cstdio>
#include <cstring>

// Global settings instance with defaults
Settings g_settings;

// Platform-specific path handling
#if defined(ESP_PLATFORM) || defined(ARDUINO)
    // ESP32: Use SPIFFS/LittleFS root
    #include <FS.h>
    #include <SPIFFS.h>
    static const char* SETTINGS_PATH = "/settings.json";

    static FILE* settings_fopen(const char* mode) {
        // For ESP32, we need to use SPIFFS file API
        // This is a simplified wrapper - actual implementation may vary
        return fopen(SETTINGS_PATH, mode);
    }
#else
    // Simulator: Use gui/config/ folder relative to executable
    static const char* SETTINGS_PATH = "gui/config/settings.json";

    static FILE* settings_fopen(const char* mode) {
        return fopen(SETTINGS_PATH, mode);
    }
#endif

void settings_init() {
    // Initialize with defaults (struct already has default values)
    g_settings = Settings();

#if defined(ESP_PLATFORM) || defined(ARDUINO)
    // Initialize SPIFFS for ESP32
    if (!SPIFFS.begin(true)) {
        // Format SPIFFS if mount failed
        SPIFFS.format();
        SPIFFS.begin(true);
    }
#endif
}

void settings_load() {
    FILE* f = settings_fopen("r");
    if (!f) {
        // File doesn't exist, keep defaults
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        int value;
        if (sscanf(line, " \"version\" : %d", &value) == 1) {
            g_settings.version = (uint8_t)value;
        } else if (sscanf(line, " \"language\" : %d", &value) == 1) {
            g_settings.language = (uint8_t)value;
        } else if (sscanf(line, " \"bg_color\" : %d", &value) == 1) {
            g_settings.bg_color = (uint8_t)value;
        } else if (sscanf(line, " \"brightness\" : %d", &value) == 1) {
            g_settings.brightness = (uint8_t)value;
        }
    }

    fclose(f);

    // Validate loaded values are within range
    if (g_settings.language >= LANG_COUNT) g_settings.language = LANG_EN;
    if (g_settings.bg_color >= BG_COLOR_COUNT) g_settings.bg_color = BG_COLOR_LIGHT_GRAY;
    if (g_settings.brightness < 10 || g_settings.brightness > 100) g_settings.brightness = 80;
}

void settings_save() {
    FILE* f = settings_fopen("w");
    if (!f) {
        return;
    }

    fprintf(f, "{\n");
    fprintf(f, "    \"version\": %d,\n", g_settings.version);
    fprintf(f, "    \"language\": %d,\n", g_settings.language);
    fprintf(f, "    \"bg_color\": %d,\n", g_settings.bg_color);
    fprintf(f, "    \"brightness\": %d\n", g_settings.brightness);
    fprintf(f, "}\n");

    fclose(f);
}

void settings_reset() {
    g_settings = Settings();
    settings_save();
}

const char* settings_get_path() {
    return SETTINGS_PATH;
}
