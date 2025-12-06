// gui/pages/page_settings.cpp - Settings page using SettingsBuilder

#include "lvgl.h"
#include "gui/gui.h"
#include "gui/settings_builder.h"
#include "gui/config/settings.h"
#include "gui/lang.h"
#include "gui/version.h"
#include <cstdio>

// Background color options (must match BgColorPreset enum order)
static const char* BG_COLOR_OPTIONS =
    "Light Gray\n"
    "White\n"
    "Light Blue\n"
    "Light Green\n"
    "Cream";

// Language options (must match Language enum order)
static const char* LANGUAGE_OPTIONS =
    "English\n"
    "Deutsch\n"
    "Français\n"
    "Español\n"
    "Italiano\n"
    "Nederlands\n"
    "Čeština";

// Servo protocol options (must match ServoProtocol enum order)
static const char* SERVO_PROTOCOL_OPTIONS =
    "Standard\n"
    "Extended\n"
    "Sanwa\n"
    "Futaba\n"
    "Digital Fast\n"
    "Custom";

// Servo frequency options
static const char* SERVO_FREQ_OPTIONS =
    "50 Hz\n"
    "333 Hz";

// UI elements we need to update when protocol changes
static lv_obj_t* lbl_pwm_min = nullptr;
static lv_obj_t* lbl_pwm_center = nullptr;
static lv_obj_t* lbl_pwm_max = nullptr;
static lv_obj_t* dd_frequency = nullptr;
static lv_obj_t* dd_protocol = nullptr;

// Helper to update PWM value labels
static void update_servo_value_labels() {
    if (lbl_pwm_min) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d µs", g_settings.servo_pwm_min);
        lv_label_set_text(lbl_pwm_min, buf);
    }
    if (lbl_pwm_center) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d µs", g_settings.servo_pwm_center);
        lv_label_set_text(lbl_pwm_center, buf);
    }
    if (lbl_pwm_max) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d µs", g_settings.servo_pwm_max);
        lv_label_set_text(lbl_pwm_max, buf);
    }
    if (dd_frequency) {
        lv_dropdown_set_selected(dd_frequency, g_settings.servo_frequency == 333 ? 1 : 0);
    }
}

// Callbacks
static void on_language_change(lv_event_t* e) {
    lv_obj_t* dd = lv_event_get_target_obj(e);
    uint16_t sel = lv_dropdown_get_selected(dd);
    g_settings.language = (uint8_t)sel;
    lang_set((Language)sel);
    settings_save();
    gui_set_page(PAGE_SETTINGS);  // Refresh to show new language
}

static void on_brightness_change(lv_event_t* e) {
    lv_obj_t* sl = lv_event_get_target_obj(e);
    g_settings.brightness = (uint8_t)lv_slider_get_value(sl);
    settings_save();
    // TODO: Apply brightness to display
}

static void on_bg_color_change(lv_event_t* e) {
    lv_obj_t* dd = lv_event_get_target_obj(e);
    uint16_t sel = lv_dropdown_get_selected(dd);
    g_settings.bg_color = (uint8_t)sel;
    gui_set_bg_color((BgColorPreset)sel);
    settings_save();
}

// Servo callbacks
static void on_servo_protocol_change(lv_event_t* e) {
    lv_obj_t* dd = lv_event_get_target_obj(e);
    uint16_t sel = lv_dropdown_get_selected(dd);

    if (sel != SERVO_CUSTOM) {
        servo_apply_preset((ServoProtocol)sel);
        update_servo_value_labels();
    } else {
        g_settings.servo_protocol = SERVO_CUSTOM;
    }
    settings_save();
}

static void on_servo_frequency_change(lv_event_t* e) {
    lv_obj_t* dd = lv_event_get_target_obj(e);
    uint16_t sel = lv_dropdown_get_selected(dd);
    g_settings.servo_frequency = (sel == 1) ? 333 : 50;

    // Switch to custom if values don't match a preset
    g_settings.servo_protocol = SERVO_CUSTOM;
    if (dd_protocol) lv_dropdown_set_selected(dd_protocol, SERVO_CUSTOM);

    settings_save();
}

static void on_servo_pwm_min_change(lv_event_t* e) {
    lv_obj_t* sl = lv_event_get_target_obj(e);
    g_settings.servo_pwm_min = (uint16_t)lv_slider_get_value(sl);

    // Update label and switch to custom
    char buf[16];
    snprintf(buf, sizeof(buf), "%d µs", g_settings.servo_pwm_min);
    if (lbl_pwm_min) lv_label_set_text(lbl_pwm_min, buf);

    g_settings.servo_protocol = SERVO_CUSTOM;
    if (dd_protocol) lv_dropdown_set_selected(dd_protocol, SERVO_CUSTOM);

    settings_save();
}

static void on_servo_pwm_center_change(lv_event_t* e) {
    lv_obj_t* sl = lv_event_get_target_obj(e);
    g_settings.servo_pwm_center = (uint16_t)lv_slider_get_value(sl);

    char buf[16];
    snprintf(buf, sizeof(buf), "%d µs", g_settings.servo_pwm_center);
    if (lbl_pwm_center) lv_label_set_text(lbl_pwm_center, buf);

    g_settings.servo_protocol = SERVO_CUSTOM;
    if (dd_protocol) lv_dropdown_set_selected(dd_protocol, SERVO_CUSTOM);

    settings_save();
}

static void on_servo_pwm_max_change(lv_event_t* e) {
    lv_obj_t* sl = lv_event_get_target_obj(e);
    g_settings.servo_pwm_max = (uint16_t)lv_slider_get_value(sl);

    char buf[16];
    snprintf(buf, sizeof(buf), "%d µs", g_settings.servo_pwm_max);
    if (lbl_pwm_max) lv_label_set_text(lbl_pwm_max, buf);

    g_settings.servo_protocol = SERVO_CUSTOM;
    if (dd_protocol) lv_dropdown_set_selected(dd_protocol, SERVO_CUSTOM);

    settings_save();
}

void page_settings_create(lv_obj_t* parent) {
    SettingsBuilder sb(parent);

    // Language section
    sb.begin_section(tr(STR_SETTINGS_LANGUAGE));
    sb.dropdown(tr(STR_SETTINGS_LANGUAGE), LANGUAGE_OPTIONS,
                lang_get(), on_language_change);
    sb.end_section();

    // Display section
    sb.begin_section(tr(STR_SETTINGS_DISPLAY));
    sb.slider(tr(STR_SETTINGS_BRIGHTNESS), 10, 100, g_settings.brightness, on_brightness_change);
    sb.dropdown(tr(STR_SETTINGS_BACKGROUND), BG_COLOR_OPTIONS, gui_get_bg_color(), on_bg_color_change);
    sb.end_section();

    // Servo section
    sb.begin_section(tr(STR_SETTINGS_SERVO));

    // Protocol dropdown
    dd_protocol = sb.dropdown(tr(STR_SETTINGS_PROTOCOL), SERVO_PROTOCOL_OPTIONS,
                              g_settings.servo_protocol, on_servo_protocol_change);

    // Frequency dropdown
    dd_frequency = sb.dropdown(tr(STR_SETTINGS_FREQUENCY), SERVO_FREQ_OPTIONS,
                               g_settings.servo_frequency == 333 ? 1 : 0, on_servo_frequency_change);

    // PWM value sliders (500-2500 range, step 10)
    lv_obj_t* sl_min = sb.slider(tr(STR_SETTINGS_PWM_MIN), 500, 1500, g_settings.servo_pwm_min, on_servo_pwm_min_change);
    lbl_pwm_min = lv_obj_get_child(lv_obj_get_parent(sl_min), 1);  // Value label

    lv_obj_t* sl_center = sb.slider(tr(STR_SETTINGS_PWM_CENTER), 1000, 2000, g_settings.servo_pwm_center, on_servo_pwm_center_change);
    lbl_pwm_center = lv_obj_get_child(lv_obj_get_parent(sl_center), 1);

    lv_obj_t* sl_max = sb.slider(tr(STR_SETTINGS_PWM_MAX), 1500, 2500, g_settings.servo_pwm_max, on_servo_pwm_max_change);
    lbl_pwm_max = lv_obj_get_child(lv_obj_get_parent(sl_max), 1);

    sb.end_section();

    // System section
    sb.begin_section(tr(STR_SETTINGS_SYSTEM));
    sb.info(tr(STR_SETTINGS_FIRMWARE), APP_VERSION);
    sb.info("LVGL", LVGL_VERSION_STRING);
    sb.end_section();
}
