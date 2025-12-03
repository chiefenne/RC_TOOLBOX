#include "lvgl.h"
#include "gui/gui.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"

static void btn_servo_cb(lv_event_t* e) { LV_UNUSED(e); gui_set_page(PAGE_SERVO); }
static void btn_lipo_cb(lv_event_t* e) { LV_UNUSED(e); gui_set_page(PAGE_LIPO); }
static void btn_cg_scale_cb(lv_event_t* e) { LV_UNUSED(e); gui_set_page(PAGE_CG_SCALE); }
static void btn_deflection_cb(lv_event_t* e) { LV_UNUSED(e); gui_set_page(PAGE_DEFLECTION); }
static void btn_incidence_cb(lv_event_t* e) { LV_UNUSED(e); gui_set_page(PAGE_ANGLE); }
static void btn_about_cb(lv_event_t* e) { LV_UNUSED(e); gui_set_page(PAGE_ABOUT); }

static lv_obj_t* create_nav_button(lv_obj_t* parent, const char* text, lv_event_cb_t cb) {
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_size(btn, 140, 40);
    lv_obj_set_style_bg_color(btn, lv_color_hex(GUI_COLOR_MONO[1]), 0);
    lv_obj_set_style_radius(btn, 5, 0);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, FONT_BUTTON_SMMD, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(GUI_COLOR_TINTS[10]), 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(lbl);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
    return btn;
}

void page_home_create(lv_obj_t* parent) {
    // Create a grid of buttons
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(parent, 10, 0);
    lv_obj_set_style_pad_column(parent, 10, 0);

    create_nav_button(parent, tr(STR_BTN_SERVO), btn_servo_cb);
    create_nav_button(parent, tr(STR_BTN_LIPO), btn_lipo_cb);
    create_nav_button(parent, tr(STR_BTN_CG_SCALE), btn_cg_scale_cb);
    create_nav_button(parent, tr(STR_BTN_DEFLECTION), btn_deflection_cb);
    create_nav_button(parent, tr(STR_BTN_ANGLE), btn_incidence_cb);
    create_nav_button(parent, tr(STR_BTN_ABOUT), btn_about_cb);
}
