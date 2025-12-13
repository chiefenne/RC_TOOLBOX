#pragma once

// Language codes
enum Language {
    LANG_EN = 0,
    LANG_DE,
    LANG_FR,
    LANG_ES,
    LANG_IT,
    LANG_NL,
    LANG_CS,
    LANG_COUNT
};

// String IDs for all translatable text
enum StringId {
    // App
    STR_APP_TITLE = 0,
    STR_APP_TAGLINE,

    // Splash features
    STR_FEATURE_SERVO,
    STR_FEATURE_LIPO,
    STR_FEATURE_CG_SCALE,
    STR_FEATURE_DEFLECTION,
    STR_FEATURE_ANGLE,

    // Page titles
    STR_PAGE_HOME,
    STR_PAGE_SERVO,
    STR_PAGE_LIPO,
    STR_PAGE_CG_SCALE,
    STR_PAGE_DEFLECTION,
    STR_PAGE_ANGLE,
    STR_PAGE_SETTINGS,
    STR_PAGE_ABOUT,

    // Home buttons
    STR_BTN_SERVO,
    STR_BTN_LIPO,
    STR_BTN_CG_SCALE,
    STR_BTN_DEFLECTION,
    STR_BTN_ANGLE,
    STR_BTN_ABOUT,
    STR_BTN_HOME,

    // About page
    STR_ABOUT_AUTHOR,
    STR_ABOUT_VERSION,

    // Settings
    STR_SETTINGS_LANGUAGE,
    STR_SETTINGS_DISPLAY,
    STR_SETTINGS_BRIGHTNESS,
    STR_SETTINGS_BACKGROUND,
    STR_SETTINGS_SYSTEM,
    STR_SETTINGS_FIRMWARE,

    // Servo settings
    STR_SETTINGS_SERVO,
    STR_SETTINGS_PROTOCOL,
    STR_SETTINGS_PWM_MIN,
    STR_SETTINGS_PWM_CENTER,
    STR_SETTINGS_PWM_MAX,
    STR_SETTINGS_FREQUENCY,
    STR_SETTINGS_SERVO_STEP,      // "Step Servo X"
    STR_SETTINGS_SERVO_RESET,     // "Reset Servos"
    STR_SETTINGS_SERVO_STEP_US,   // "X µs" format

    // Page content placeholders
    STR_SERVO_CONTENT,
    STR_LIPO_CONTENT,
    STR_CG_SCALE_CONTENT,
    STR_DEFLECTION_CONTENT,
    STR_ANGLE_CONTENT,

    // Servo page
    STR_SERVO_MODE_AUTO,
    STR_SERVO_MODE_MANUAL,
    STR_SERVO_PWM_LABEL,
    STR_SERVO_POSITION,
    STR_SERVO_START,
    STR_SERVO_STOP,
    STR_SERVO_SPEED,
    STR_SERVO_US,

    // Background color options
    STR_BG_LIGHT_GRAY,
    STR_BG_WHITE,
    STR_BG_LIGHT_BLUE,
    STR_BG_LIGHT_GREEN,
    STR_BG_CREAM,

    // Frequency options
    STR_FREQ_50HZ,
    STR_FREQ_333HZ,

    STR_COUNT
};

// Get current language
Language lang_get();

// Set language
void lang_set(Language lang);

// Get translated string
const char* tr(StringId id);
