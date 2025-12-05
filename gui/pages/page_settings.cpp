// gui/pages/page_settings.cpp - Settings page using SettingsBuilder

#include "lvgl.h"
#include "gui/gui.h"
#include "gui/settings_builder.h"
#include "gui/config/settings.h"
#include "gui/lang.h"
#include "gui/version.h"

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

void page_settings_create(lv_obj_t* parent) {
    SettingsBuilder sb(parent);

    // Language section
    sb.begin_section(tr(STR_SETTINGS_LANGUAGE));
    sb.dropdown(tr(STR_SETTINGS_LANGUAGE), LANGUAGE_OPTIONS,
                lang_get(), on_language_change);
    sb.end_section();

    // Display section
    sb.begin_section("Display");
    sb.slider("Brightness", 10, 100, g_settings.brightness, on_brightness_change);
    sb.dropdown("Background", BG_COLOR_OPTIONS, gui_get_bg_color(), on_bg_color_change);
    sb.end_section();

    // System section
    sb.begin_section("System");
    sb.info("Firmware", APP_VERSION);
    sb.info("LVGL", LVGL_VERSION_STRING);
    sb.end_section();
}
