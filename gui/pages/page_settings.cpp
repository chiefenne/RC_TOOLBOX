#include "lvgl.h"
#include "gui/gui.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"

static void lang_en_cb(lv_event_t* e) {
    LV_UNUSED(e);
    lang_set(LANG_EN);
    gui_set_page(PAGE_SETTINGS);  // Refresh to show new language
}

static void lang_de_cb(lv_event_t* e) {
    LV_UNUSED(e);
    lang_set(LANG_DE);
    gui_set_page(PAGE_SETTINGS);  // Refresh to show new language
}

void page_settings_create(lv_obj_t* parent) {
    // Flex column layout - same pattern as page_servo
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(parent, 20, 0);

    // Language label
    lv_obj_t* lang_label = lv_label_create(parent);
    lv_label_set_text(lang_label, tr(STR_SETTINGS_LANGUAGE));
    lv_obj_set_style_text_font(lang_label, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(lang_label, lv_color_hex(GUI_COLOR_SHADES[7]), 0);

    // Language buttons container
    lv_obj_t* lang_cont = lv_obj_create(parent);
    lv_obj_set_size(lang_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(lang_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(lang_cont, 0, 0);
    lv_obj_set_style_pad_all(lang_cont, 5, 0);
    lv_obj_set_flex_flow(lang_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(lang_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(lang_cont, 10, 0);

    // English button
    lv_obj_t* btn_en = lv_button_create(lang_cont);
    lv_obj_set_size(btn_en, 80, 35);
    lv_obj_set_style_bg_color(btn_en,
        lang_get() == LANG_EN ? lv_color_hex(GUI_COLOR_TRIAD[1]) : lv_color_hex(GUI_COLOR_GRAYS[6]), 0);
    lv_obj_t* lbl_en = lv_label_create(btn_en);
    lv_label_set_text(lbl_en, "English");
    lv_obj_set_style_text_font(lbl_en, FONT_DEFAULT, 0);
    lv_obj_center(lbl_en);
    lv_obj_add_event_cb(btn_en, lang_en_cb, LV_EVENT_CLICKED, nullptr);

    // German button
    lv_obj_t* btn_de = lv_button_create(lang_cont);
    lv_obj_set_size(btn_de, 80, 35);
    lv_obj_set_style_bg_color(btn_de,
        lang_get() == LANG_DE ? lv_color_hex(GUI_COLOR_TRIAD[1]) : lv_color_hex(GUI_COLOR_GRAYS[6]), 0);
    lv_obj_t* lbl_de = lv_label_create(btn_de);
    lv_label_set_text(lbl_de, "Deutsch");
    lv_obj_set_style_text_font(lbl_de, FONT_DEFAULT, 0);
    lv_obj_center(lbl_de);
    lv_obj_add_event_cb(btn_de, lang_de_cb, LV_EVENT_CLICKED, nullptr);
}
