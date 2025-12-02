#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include <cstdio>

// Servo state
static int pwm_value = 1500;  // 1000-2000 µs
static bool is_auto_mode = true;
static bool is_running = false;

// GUI elements
static lv_obj_t* pwm_label = nullptr;
static lv_obj_t* position_bar = nullptr;
static lv_obj_t* btn_auto = nullptr;
static lv_obj_t* btn_manual = nullptr;
static lv_obj_t* btn_start_stop = nullptr;
static lv_obj_t* lbl_start_stop = nullptr;

// Colors
static const lv_color_t COLOR_BTN_ACTIVE = lv_color_hex(GUI_COLOR_TRIAD[1]);    // Green
static const lv_color_t COLOR_BTN_INACTIVE = lv_color_hex(GUI_COLOR_GRAYS[6]);  // Gray
static const lv_color_t COLOR_BTN_STOP = lv_color_hex(0xCC3333);                // Red

static void update_pwm_display() {
    static char buf[16];
    snprintf(buf, sizeof(buf), "%d", pwm_value);
    lv_label_set_text(pwm_label, buf);

    // Update position bar (0-100 from 1000-2000)
    int percent = (pwm_value - 1000) / 10;
    lv_bar_set_value(position_bar, percent, LV_ANIM_ON);
}

static void update_mode_buttons() {
    lv_obj_set_style_bg_color(btn_auto, is_auto_mode ? COLOR_BTN_ACTIVE : COLOR_BTN_INACTIVE, 0);
    lv_obj_set_style_bg_color(btn_manual, !is_auto_mode ? COLOR_BTN_ACTIVE : COLOR_BTN_INACTIVE, 0);
}

static void update_start_stop_button() {
    if (is_running) {
        lv_label_set_text(lbl_start_stop, tr(STR_SERVO_STOP));
        lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN_STOP, 0);
    } else {
        lv_label_set_text(lbl_start_stop, tr(STR_SERVO_START));
        lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN_ACTIVE, 0);
    }
}

static void btn_auto_cb(lv_event_t* e) {
    LV_UNUSED(e);
    is_auto_mode = true;
    update_mode_buttons();
}

static void btn_manual_cb(lv_event_t* e) {
    LV_UNUSED(e);
    is_auto_mode = false;
    is_running = false;
    update_mode_buttons();
    update_start_stop_button();
}

static void btn_start_stop_cb(lv_event_t* e) {
    LV_UNUSED(e);
    if (is_auto_mode) {
        is_running = !is_running;
        update_start_stop_button();
    }
}

void page_servo_create(lv_obj_t* parent) {
    // Reset state
    pwm_value = 1500;
    is_auto_mode = true;
    is_running = false;

    // Flex column layout with even spacing
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_top(parent, 0, 0);
    lv_obj_set_style_pad_bottom(parent, 0, 0);

    // === Mode buttons row ===
    lv_obj_t* mode_row = lv_obj_create(parent);
    lv_obj_set_size(mode_row, LV_PCT(90), 30);
    lv_obj_set_style_bg_opa(mode_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(mode_row, 0, 0);
    lv_obj_set_style_pad_all(mode_row, 0, 0);
    lv_obj_set_flex_flow(mode_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(mode_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(mode_row, 10, 0);

    // Auto button
    btn_auto = lv_button_create(mode_row);
    lv_obj_set_size(btn_auto, 90, 30);
    lv_obj_set_style_bg_color(btn_auto, COLOR_BTN_ACTIVE, 0);
    lv_obj_t* lbl_auto = lv_label_create(btn_auto);
    lv_label_set_text(lbl_auto, tr(STR_SERVO_MODE_AUTO));
    lv_obj_set_style_text_font(lbl_auto, FONT_DEFAULT, 0);
    lv_obj_center(lbl_auto);
    lv_obj_add_event_cb(btn_auto, btn_auto_cb, LV_EVENT_CLICKED, nullptr);

    // Manual button
    btn_manual = lv_button_create(mode_row);
    lv_obj_set_size(btn_manual, 90, 30);
    lv_obj_set_style_bg_color(btn_manual, COLOR_BTN_INACTIVE, 0);
    lv_obj_t* lbl_manual = lv_label_create(btn_manual);
    lv_label_set_text(lbl_manual, tr(STR_SERVO_MODE_MANUAL));
    lv_obj_set_style_text_font(lbl_manual, FONT_DEFAULT, 0);
    lv_obj_center(lbl_manual);
    lv_obj_add_event_cb(btn_manual, btn_manual_cb, LV_EVENT_CLICKED, nullptr);

    // === PWM Display ===
    lv_obj_t* pwm_container = lv_obj_create(parent);
    lv_obj_set_size(pwm_container, LV_PCT(90), 30);
    lv_obj_set_style_bg_opa(pwm_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(pwm_container, 0, 0);
    lv_obj_set_style_pad_all(pwm_container, 0, 0);
    lv_obj_set_flex_flow(pwm_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pwm_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(pwm_container, 5, 0);

    // PWM label (small)
    lv_obj_t* pwm_title = lv_label_create(pwm_container);
    lv_label_set_text(pwm_title, tr(STR_SERVO_PWM_LABEL));
    lv_obj_set_style_text_font(pwm_title, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(pwm_title, lv_color_hex(GUI_COLOR_SHADES[7]), 0);

    // PWM value (large, monospace)
    pwm_label = lv_label_create(pwm_container);
    lv_label_set_text(pwm_label, "1500");
    lv_obj_set_style_text_font(pwm_label, FONT_MONO_LG, 0);
    lv_obj_set_style_text_color(pwm_label, lv_color_hex(GUI_COLOR_MONO[0]), 0);

    // µs unit
    lv_obj_t* pwm_unit = lv_label_create(pwm_container);
    lv_label_set_text(pwm_unit, tr(STR_SERVO_US));
    lv_obj_set_style_text_font(pwm_unit, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(pwm_unit, lv_color_hex(GUI_COLOR_SHADES[7]), 0);

    // === Position Bar ===
    lv_obj_t* bar_container = lv_obj_create(parent);
    lv_obj_set_size(bar_container, LV_PCT(90), 35);
    lv_obj_set_style_bg_opa(bar_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(bar_container, 0, 0);
    lv_obj_set_style_pad_all(bar_container, 0, 0);
    lv_obj_set_flex_flow(bar_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(bar_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(bar_container, 2, 0);

    // Position label
    lv_obj_t* pos_title = lv_label_create(bar_container);
    lv_label_set_text(pos_title, tr(STR_SERVO_POSITION));
    lv_obj_set_style_text_font(pos_title, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(pos_title, lv_color_hex(GUI_COLOR_SHADES[7]), 0);

    // Position bar
    position_bar = lv_bar_create(bar_container);
    lv_obj_set_size(position_bar, LV_PCT(100), 16);
    lv_bar_set_range(position_bar, 0, 100);
    lv_bar_set_value(position_bar, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(position_bar, lv_color_hex(GUI_COLOR_GRAYS[7]), 0);
    lv_obj_set_style_bg_color(position_bar, lv_color_hex(GUI_COLOR_MONO[1]), LV_PART_INDICATOR);

    // === Start/Stop Button ===
    btn_start_stop = lv_button_create(parent);
    lv_obj_set_size(btn_start_stop, 120, 35);
    lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN_ACTIVE, 0);
    lbl_start_stop = lv_label_create(btn_start_stop);
    lv_label_set_text(lbl_start_stop, tr(STR_SERVO_START));
    lv_obj_set_style_text_font(lbl_start_stop, FONT_DEFAULT, 0);
    lv_obj_center(lbl_start_stop);
    lv_obj_add_event_cb(btn_start_stop, btn_start_stop_cb, LV_EVENT_CLICKED, nullptr);

    // Initial display update
    update_pwm_display();
    update_mode_buttons();
    update_start_stop_button();
}
