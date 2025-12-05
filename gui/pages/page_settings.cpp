// gui/pages/page_settings.cpp - Settings page using SettingsBuilder

#include "lvgl.h"
#include "gui/gui.h"
#include "gui/settings_builder.h"
#include "gui/lang.h"
#include "gui/version.h"

// Demo settings values (will be replaced with persistent storage later)
static int s_brightness = 80;
static bool s_dark_mode = false;

// Callbacks
static void on_language_change(lv_event_t* e) {
    lv_obj_t* dd = lv_event_get_target_obj(e);
    uint16_t sel = lv_dropdown_get_selected(dd);
    lang_set(sel == 0 ? LANG_EN : LANG_DE);
    gui_set_page(PAGE_SETTINGS);  // Refresh to show new language
}

static void on_brightness_change(lv_event_t* e) {
    lv_obj_t* sl = lv_event_get_target_obj(e);
    s_brightness = lv_slider_get_value(sl);
    // TODO: Apply brightness to display
}

static void on_dark_mode_change(lv_event_t* e) {
    lv_obj_t* sw = lv_event_get_target_obj(e);
    s_dark_mode = lv_obj_has_state(sw, LV_STATE_CHECKED);
    // TODO: Apply theme change
}

void page_settings_create(lv_obj_t* parent) {
    SettingsBuilder sb(parent);

    // Language section
    sb.begin_section(tr(STR_SETTINGS_LANGUAGE));
    sb.dropdown(tr(STR_SETTINGS_LANGUAGE), "English\nDeutsch",
                lang_get() == LANG_EN ? 0 : 1, on_language_change);
    sb.end_section();

    // Display section (demo)
    sb.begin_section("Display");
    sb.slider("Brightness", 10, 100, s_brightness, on_brightness_change);
    sb.toggle("Dark Mode", s_dark_mode, on_dark_mode_change);
    sb.end_section();

    // System section
    sb.begin_section("System");
    sb.info("Firmware", APP_VERSION);
    sb.info("LVGL", LVGL_VERSION_STRING);
    sb.end_section();
}
