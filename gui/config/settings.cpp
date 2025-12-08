// gui/config/settings.cpp - Settings storage implementation
// Platform-agnostic JSON file handling

#include "gui/config/settings.h"
#include <cstdio>
#include <cstring>

// Global settings instance with defaults
Settings g_settings;

// Servo protocol preset values
struct ServoPreset {
    uint16_t pwm_min;
    uint16_t pwm_center;
    uint16_t pwm_max;
    uint16_t frequency;
};

static const ServoPreset SERVO_PRESETS[] = {
    { 1000, 1500, 2000,  50 },  // SERVO_STANDARD
    {  500, 1500, 2500,  50 },  // SERVO_EXTENDED
    {  920, 1520, 2120,  50 },  // SERVO_SANWA
    {  900, 1500, 2100,  50 },  // SERVO_FUTABA
    { 1000, 1500, 2000, 333 },  // SERVO_DIGITAL_FAST
    { 1000, 1500, 2000,  50 },  // SERVO_CUSTOM (defaults)
};

void servo_apply_preset(ServoProtocol protocol) {
    if (protocol >= SERVO_PROTOCOL_COUNT) return;
    if (protocol == SERVO_CUSTOM) return;  // Don't overwrite custom values

    const ServoPreset& p = SERVO_PRESETS[protocol];
    g_settings.servo_protocol = protocol;
    g_settings.servo_pwm_min = p.pwm_min;
    g_settings.servo_pwm_center = p.pwm_center;
    g_settings.servo_pwm_max = p.pwm_max;
    g_settings.servo_frequency = p.frequency;
}

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
        } else if (sscanf(line, " \"servo_protocol\" : %d", &value) == 1) {
            g_settings.servo_protocol = (uint8_t)value;
        } else if (sscanf(line, " \"servo_pwm_min\" : %d", &value) == 1) {
            g_settings.servo_pwm_min = (uint16_t)value;
        } else if (sscanf(line, " \"servo_pwm_center\" : %d", &value) == 1) {
            g_settings.servo_pwm_center = (uint16_t)value;
        } else if (sscanf(line, " \"servo_pwm_max\" : %d", &value) == 1) {
            g_settings.servo_pwm_max = (uint16_t)value;
        } else if (sscanf(line, " \"servo_frequency\" : %d", &value) == 1) {
            g_settings.servo_frequency = (uint16_t)value;
        }
    }

    fclose(f);

    // Validate loaded values are within range
    if (g_settings.language >= LANG_COUNT) g_settings.language = LANG_EN;
    if (g_settings.bg_color >= BG_COLOR_COUNT) g_settings.bg_color = BG_COLOR_WHITE;
    if (g_settings.brightness < 10 || g_settings.brightness > 100) g_settings.brightness = 80;
    if (g_settings.servo_protocol >= SERVO_PROTOCOL_COUNT) g_settings.servo_protocol = SERVO_STANDARD;
    if (g_settings.servo_pwm_min < 500 || g_settings.servo_pwm_min > 1500) g_settings.servo_pwm_min = 1000;
    if (g_settings.servo_pwm_center < 1000 || g_settings.servo_pwm_center > 2000) g_settings.servo_pwm_center = 1500;
    if (g_settings.servo_pwm_max < 1500 || g_settings.servo_pwm_max > 2500) g_settings.servo_pwm_max = 2000;
    if (g_settings.servo_frequency < 50 || g_settings.servo_frequency > 400) g_settings.servo_frequency = 50;
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
    fprintf(f, "    \"brightness\": %d,\n", g_settings.brightness);
    fprintf(f, "    \"servo_protocol\": %d,\n", g_settings.servo_protocol);
    fprintf(f, "    \"servo_pwm_min\": %d,\n", g_settings.servo_pwm_min);
    fprintf(f, "    \"servo_pwm_center\": %d,\n", g_settings.servo_pwm_center);
    fprintf(f, "    \"servo_pwm_max\": %d,\n", g_settings.servo_pwm_max);
    fprintf(f, "    \"servo_frequency\": %d\n", g_settings.servo_frequency);
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
