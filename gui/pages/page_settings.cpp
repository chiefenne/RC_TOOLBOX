// gui/pages/page_settings.cpp - Settings page using SettingsBuilder

#include "lvgl.h"
#include "gui/gui.h"
#include "gui/settings_builder.h"
#include "gui/config/settings.h"
#include "gui/lang.h"
#include "gui/version.h"
#include "gui/input.h"
#include <cstdio>
#include <cstring>

// =============================================================================
// Focus Order Configuration
// =============================================================================
// Define focus order here - change these numbers to reorder navigation
enum FocusOrder {
    FO_SEC_LANGUAGE = 0,   // Section headers
    FO_LANGUAGE     = 1,
    FO_SEC_DISPLAY  = 2,
    FO_BRIGHTNESS   = 3,
    FO_BACKGROUND   = 4,
    FO_SEC_SERVO    = 5,
    FO_PROTOCOL     = 6,
    FO_FREQUENCY    = 7,
    FO_PWM_MIN      = 8,
    FO_PWM_CENTER   = 9,
    FO_PWM_MAX      = 10,
    FO_SERVO_STEP_1 = 11,
    FO_SERVO_STEP_2 = 12,
    FO_SERVO_STEP_3 = 13,
    FO_SERVO_STEP_4 = 14,
    FO_SERVO_STEP_5 = 15,
    FO_SERVO_STEP_6 = 16,
    FO_SERVO_RESET  = 17,
    FO_SEC_SYSTEM   = 18,
    FO_BTN_HOME     = 19,
    FO_BTN_PREV     = 20,
    FO_BTN_NEXT     = 21,
    FO_BTN_SETTINGS = 22,
};

// Focus group builder for this page
static FocusOrderBuilder focus_builder;

// Language options (must match Language enum order) - kept in native language
static const char* LANGUAGE_OPTIONS =
    "English\n"
    "Deutsch\n"
    "Français\n"
    "Español\n"
    "Italiano\n"
    "Nederlands\n"
    "Čeština";

// Servo protocol options (must match ServoProtocol enum order) - industry terms, not translated
static const char* SERVO_PROTOCOL_OPTIONS =
    "Standard\n"
    "Extended\n"
    "Sanwa\n"
    "Futaba\n"
    "Digital Fast\n"
    "Custom";

// Helper to build translated dropdown options at runtime
static char bg_color_options[128];
static char freq_options[64];

static void build_translated_options() {
    // Background colors
    snprintf(bg_color_options, sizeof(bg_color_options), "%s\n%s\n%s\n%s\n%s",
        tr(STR_BG_LIGHT_GRAY), tr(STR_BG_WHITE), tr(STR_BG_LIGHT_BLUE),
        tr(STR_BG_LIGHT_GREEN), tr(STR_BG_CREAM));

    // Frequency options
    snprintf(freq_options, sizeof(freq_options), "%s\n%s",
        tr(STR_FREQ_50HZ), tr(STR_FREQ_333HZ));
}

// UI elements we need to update when protocol changes
static lv_obj_t* lbl_pwm_min = nullptr;
static lv_obj_t* lbl_pwm_center = nullptr;
static lv_obj_t* lbl_pwm_max = nullptr;
static lv_obj_t* dd_frequency = nullptr;
static lv_obj_t* dd_protocol = nullptr;

// Per-servo step labels
static lv_obj_t* lbl_servo_step[NUM_SERVOS] = {};
static lv_obj_t* sl_servo_step[NUM_SERVOS] = {};

// Helper to update PWM value labels
static void update_servo_value_labels() {
    if (lbl_pwm_min) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%4d µs", g_settings.servo_pwm_min);
        lv_label_set_text(lbl_pwm_min, buf);
    }
    if (lbl_pwm_center) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%4d µs", g_settings.servo_pwm_center);
        lv_label_set_text(lbl_pwm_center, buf);
    }
    if (lbl_pwm_max) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%4d µs", g_settings.servo_pwm_max);
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
    gui_set_page(PAGE_SETTINGS);  // Refresh to show new language (triggers destroy->save)
}

static void on_brightness_change(lv_event_t* e) {
    lv_obj_t* sl = lv_event_get_target_obj(e);
    g_settings.brightness = (uint8_t)lv_slider_get_value(sl);
    // TODO: Apply brightness to display
}

static void on_bg_color_change(lv_event_t* e) {
    lv_obj_t* dd = lv_event_get_target_obj(e);
    uint16_t sel = lv_dropdown_get_selected(dd);
    g_settings.bg_color = (uint8_t)sel;
    gui_set_bg_color((BgColorPreset)sel);
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
}

static void on_servo_frequency_change(lv_event_t* e) {
    lv_obj_t* dd = lv_event_get_target_obj(e);
    uint16_t sel = lv_dropdown_get_selected(dd);
    g_settings.servo_frequency = (sel == 1) ? 333 : 50;

    // Switch to custom if values don't match a preset
    g_settings.servo_protocol = SERVO_CUSTOM;
    if (dd_protocol) lv_dropdown_set_selected(dd_protocol, SERVO_CUSTOM);
}

static void on_servo_pwm_min_change(lv_event_t* e) {
    lv_obj_t* sl = lv_event_get_target_obj(e);
    g_settings.servo_pwm_min = (uint16_t)lv_slider_get_value(sl);

    // Update label and switch to custom
    char buf[16];
    snprintf(buf, sizeof(buf), "%4d µs", g_settings.servo_pwm_min);
    if (lbl_pwm_min) lv_label_set_text(lbl_pwm_min, buf);

    g_settings.servo_protocol = SERVO_CUSTOM;
    if (dd_protocol) lv_dropdown_set_selected(dd_protocol, SERVO_CUSTOM);
}

static void on_servo_pwm_center_change(lv_event_t* e) {
    lv_obj_t* sl = lv_event_get_target_obj(e);
    g_settings.servo_pwm_center = (uint16_t)lv_slider_get_value(sl);

    char buf[16];
    snprintf(buf, sizeof(buf), "%4d µs", g_settings.servo_pwm_center);
    if (lbl_pwm_center) lv_label_set_text(lbl_pwm_center, buf);

    g_settings.servo_protocol = SERVO_CUSTOM;
    if (dd_protocol) lv_dropdown_set_selected(dd_protocol, SERVO_CUSTOM);
}

static void on_servo_pwm_max_change(lv_event_t* e) {
    lv_obj_t* sl = lv_event_get_target_obj(e);
    g_settings.servo_pwm_max = (uint16_t)lv_slider_get_value(sl);

    char buf[16];
    snprintf(buf, sizeof(buf), "%4d µs", g_settings.servo_pwm_max);
    if (lbl_pwm_max) lv_label_set_text(lbl_pwm_max, buf);

    g_settings.servo_protocol = SERVO_CUSTOM;
    if (dd_protocol) lv_dropdown_set_selected(dd_protocol, SERVO_CUSTOM);
}

// Per-servo PWM step callbacks
static void on_servo_step_change(lv_event_t* e) {
    int idx = reinterpret_cast<intptr_t>(lv_event_get_user_data(e));
    if (idx < 0 || idx >= NUM_SERVOS) return;

    lv_obj_t* sl = lv_event_get_target_obj(e);
    g_settings.servo_pwm_step[idx] = (uint8_t)lv_slider_get_value(sl);

    if (lbl_servo_step[idx]) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%4d µs", g_settings.servo_pwm_step[idx]);
        lv_label_set_text(lbl_servo_step[idx], buf);
    }
}

static void on_servo_reset(lv_event_t* e) {
    (void)e;
    // Reset all servo PWM steps to default
    servo_reset_pwm_steps();

    // Update all step slider UI
    for (int i = 0; i < NUM_SERVOS; i++) {
        if (sl_servo_step[i]) {
            lv_slider_set_value(sl_servo_step[i], DEFAULT_PWM_STEP, LV_ANIM_OFF);
        }
        if (lbl_servo_step[i]) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%4d µs", DEFAULT_PWM_STEP);
            lv_label_set_text(lbl_servo_step[i], buf);
        }
    }
}

void page_settings_create(lv_obj_t* parent) {
    // Initialize focus builder
    focus_builder.init();

    // Record this page in navigation history
    input_push_page(PAGE_SETTINGS);

    // Build translated option strings
    build_translated_options();

    SettingsBuilder sb(parent);

    // Language section
    lv_obj_t* sec_lang = sb.begin_section(tr(STR_SETTINGS_LANGUAGE));
    focus_builder.add(sec_lang, FO_SEC_LANGUAGE);
    lv_obj_t* dd_lang = sb.dropdown(tr(STR_SETTINGS_LANGUAGE), LANGUAGE_OPTIONS,
                lang_get(), on_language_change);
    focus_builder.add(dd_lang, FO_LANGUAGE);
    sb.end_section();

    // Display section
    lv_obj_t* sec_display = sb.begin_section(tr(STR_SETTINGS_DISPLAY));
    focus_builder.add(sec_display, FO_SEC_DISPLAY);
    lv_obj_t* sl_brightness = sb.slider(tr(STR_SETTINGS_BRIGHTNESS), 10, 100, g_settings.brightness, on_brightness_change);
    focus_builder.add(sl_brightness, FO_BRIGHTNESS);
    lv_obj_t* dd_bg = sb.dropdown(tr(STR_SETTINGS_BACKGROUND), bg_color_options, gui_get_bg_color(), on_bg_color_change);
    focus_builder.add(dd_bg, FO_BACKGROUND);
    sb.end_section();

    // Servo section
    lv_obj_t* sec_servo = sb.begin_section(tr(STR_SETTINGS_SERVO));
    focus_builder.add(sec_servo, FO_SEC_SERVO);

    // Protocol dropdown
    dd_protocol = sb.dropdown(tr(STR_SETTINGS_PROTOCOL), SERVO_PROTOCOL_OPTIONS,
                              g_settings.servo_protocol, on_servo_protocol_change);
    focus_builder.add(dd_protocol, FO_PROTOCOL);

    // Frequency dropdown
    dd_frequency = sb.dropdown(tr(STR_SETTINGS_FREQUENCY), freq_options,
                               g_settings.servo_frequency == 333 ? 1 : 0, on_servo_frequency_change);
    focus_builder.add(dd_frequency, FO_FREQUENCY);

    // PWM value sliders (500-2500 range, step 10)
    char pwm_buf[16];

    lv_obj_t* sl_min = sb.slider(tr(STR_SETTINGS_PWM_MIN), 500, 1500, g_settings.servo_pwm_min, on_servo_pwm_min_change);
    lbl_pwm_min = lv_obj_get_child(lv_obj_get_parent(sl_min), 1);  // Value label
    snprintf(pwm_buf, sizeof(pwm_buf), "%4d µs", g_settings.servo_pwm_min);
    lv_label_set_text(lbl_pwm_min, pwm_buf);
    focus_builder.add(sl_min, FO_PWM_MIN);

    lv_obj_t* sl_center = sb.slider(tr(STR_SETTINGS_PWM_CENTER), 1000, 2000, g_settings.servo_pwm_center, on_servo_pwm_center_change);
    lbl_pwm_center = lv_obj_get_child(lv_obj_get_parent(sl_center), 1);
    snprintf(pwm_buf, sizeof(pwm_buf), "%4d µs", g_settings.servo_pwm_center);
    lv_label_set_text(lbl_pwm_center, pwm_buf);
    focus_builder.add(sl_center, FO_PWM_CENTER);

    lv_obj_t* sl_max = sb.slider(tr(STR_SETTINGS_PWM_MAX), 1500, 2500, g_settings.servo_pwm_max, on_servo_pwm_max_change);
    lbl_pwm_max = lv_obj_get_child(lv_obj_get_parent(sl_max), 1);
    snprintf(pwm_buf, sizeof(pwm_buf), "%4d µs", g_settings.servo_pwm_max);
    lv_label_set_text(lbl_pwm_max, pwm_buf);
    focus_builder.add(sl_max, FO_PWM_MAX);

    // Per-servo PWM step sliders (1-100 µs range)
    char step_label[32];
    char step_value[16];
    for (int i = 0; i < NUM_SERVOS; i++) {
        snprintf(step_label, sizeof(step_label), "%s %d", tr(STR_SETTINGS_SERVO_STEP), i + 1);
        sl_servo_step[i] = sb.slider(step_label, 1, 100, g_settings.servo_pwm_step[i],
                                      on_servo_step_change, (void*)(intptr_t)i);
        lbl_servo_step[i] = lv_obj_get_child(lv_obj_get_parent(sl_servo_step[i]), 1);
        // Initialize label with unit
        snprintf(step_value, sizeof(step_value), "%4d µs", g_settings.servo_pwm_step[i]);
        lv_label_set_text(lbl_servo_step[i], step_value);
        focus_builder.add(sl_servo_step[i], FO_SERVO_STEP_1 + i);
    }

    // Reset servos button
    lv_obj_t* btn_reset = sb.button(tr(STR_SETTINGS_SERVO_RESET), on_servo_reset);
    focus_builder.add(btn_reset, FO_SERVO_RESET);

    sb.end_section();

    // System section - only on ESP32 (causes crash in macOS simulator)
#ifdef ESP32
    lv_obj_t* sec_system = sb.begin_section(tr(STR_SETTINGS_SYSTEM));
    focus_builder.add(sec_system, FO_SEC_SYSTEM);
    sb.info(tr(STR_SETTINGS_FIRMWARE), APP_VERSION);
    sb.info("LVGL", LVGL_VERSION_STRING);
    sb.end_section();
#endif

    // Add footer buttons to focus order
    focus_builder.add(gui_get_btn_home(), FO_BTN_HOME);
    focus_builder.add(gui_get_btn_prev(), FO_BTN_PREV);
    focus_builder.add(gui_get_btn_next(), FO_BTN_NEXT);
    focus_builder.add(gui_get_btn_settings(), FO_BTN_SETTINGS);

    // Finalize focus builder
    focus_builder.finalize();
}

void page_settings_destroy() {
    // Save settings once on page exit (deferred auto-save)
    settings_save();

    // Destroy focus builder
    focus_builder.destroy();

    // Reset static pointers to avoid use-after-free
    lbl_pwm_min = nullptr;
    lbl_pwm_center = nullptr;
    lbl_pwm_max = nullptr;
    dd_frequency = nullptr;
    dd_protocol = nullptr;

    // Reset per-servo step pointers
    for (int i = 0; i < NUM_SERVOS; i++) {
        lbl_servo_step[i] = nullptr;
        sl_servo_step[i] = nullptr;
    }
}
