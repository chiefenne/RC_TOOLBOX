#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include <cstdio>

// Servo state
static int pwm_value = 1500;  // 1000-2000 µs
static bool is_auto_mode = true;
static bool is_running = false;
static int sweep_direction = 1;  // 1 = right, -1 = left
static lv_timer_t* sweep_timer = nullptr;

// GUI elements
static lv_obj_t* pwm_label = nullptr;
static lv_obj_t* position_slider = nullptr;
static lv_obj_t* btn_auto = nullptr;
static lv_obj_t* btn_manual = nullptr;
static lv_obj_t* btn_start_stop = nullptr;
static lv_obj_t* lbl_start_stop = nullptr;
static lv_obj_t* start_stop_row = nullptr;   // Container for Start/Stop button
static lv_obj_t* manual_btn_row = nullptr;   // Container for MIN/CENTER/MAX buttons

// Colors
static const lv_color_t COLOR_BTN_ACTIVE = lv_color_hex(GUI_COLOR_TRIAD[1]);    // Green
static const lv_color_t COLOR_BTN_INACTIVE = lv_color_hex(GUI_COLOR_GRAYS[6]);  // Gray
static const lv_color_t COLOR_BTN_STOP = lv_color_hex(0xCC3333);                // Red

static void update_pwm_display() {
    static char buf[16];
    snprintf(buf, sizeof(buf), "%d", pwm_value);
    lv_label_set_text(pwm_label, buf);

    // Update slider position (0-100 from 1000-2000)
    int percent = (pwm_value - 1000) / 10;
    lv_slider_set_value(position_slider, percent, LV_ANIM_ON);
}

static void update_mode_buttons() {
    lv_obj_set_style_bg_color(btn_auto, is_auto_mode ? COLOR_BTN_ACTIVE : COLOR_BTN_INACTIVE, 0);
    lv_obj_set_style_bg_color(btn_manual, !is_auto_mode ? COLOR_BTN_ACTIVE : COLOR_BTN_INACTIVE, 0);

    // Disable mode buttons while running
    if (is_running) {
        lv_obj_add_state(btn_auto, LV_STATE_DISABLED);
        lv_obj_add_state(btn_manual, LV_STATE_DISABLED);
        lv_obj_set_style_opa(btn_auto, LV_OPA_50, 0);
        lv_obj_set_style_opa(btn_manual, LV_OPA_50, 0);
    } else {
        lv_obj_clear_state(btn_auto, LV_STATE_DISABLED);
        lv_obj_clear_state(btn_manual, LV_STATE_DISABLED);
        lv_obj_set_style_opa(btn_auto, LV_OPA_100, 0);
        lv_obj_set_style_opa(btn_manual, LV_OPA_100, 0);
    }
}

static void update_bottom_buttons() {
    if (is_auto_mode) {
        // Show Start/Stop row, hide manual buttons
        lv_obj_clear_flag(start_stop_row, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(manual_btn_row, LV_OBJ_FLAG_HIDDEN);

        if (is_running) {
            lv_label_set_text(lbl_start_stop, tr(STR_SERVO_STOP));
            lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN_STOP, 0);
        } else {
            lv_label_set_text(lbl_start_stop, tr(STR_SERVO_START));
            lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN_ACTIVE, 0);
        }
    } else {
        // Hide Start/Stop row, show manual buttons
        lv_obj_add_flag(start_stop_row, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(manual_btn_row, LV_OBJ_FLAG_HIDDEN);
    }
}

static void btn_auto_cb(lv_event_t* e) {
    LV_UNUSED(e);
    if (is_running) return;  // Ignore while running
    is_auto_mode = true;
    update_mode_buttons();
    update_bottom_buttons();
}

// Timer callback for auto sweep
static void sweep_timer_cb(lv_timer_t* timer) {
    LV_UNUSED(timer);
    if (!is_running || !is_auto_mode) return;

    // Always start by moving right, reverse at endpoints
    pwm_value += sweep_direction * 10;  // 10µs steps

    if (pwm_value >= 2000) {
        pwm_value = 2000;
        sweep_direction = -1;  // Reverse to left
    } else if (pwm_value <= 1000) {
        pwm_value = 1000;
        sweep_direction = 1;   // Reverse to right
    }

    update_pwm_display();
}

static void btn_manual_cb(lv_event_t* e) {
    LV_UNUSED(e);
    if (is_running) return;  // Ignore while running
    is_auto_mode = false;
    is_running = false;
    // Stop sweep timer
    if (sweep_timer) {
        lv_timer_pause(sweep_timer);
    }
    update_mode_buttons();
    update_bottom_buttons();
}

// Manual mode button callbacks
static void btn_min_cb(lv_event_t* e) {
    LV_UNUSED(e);
    pwm_value = 1000;
    update_pwm_display();
}

static void btn_center_cb(lv_event_t* e) {
    LV_UNUSED(e);
    pwm_value = 1500;
    update_pwm_display();
}

static void btn_max_cb(lv_event_t* e) {
    LV_UNUSED(e);
    pwm_value = 2000;
    update_pwm_display();
}

static void btn_start_stop_cb(lv_event_t* e) {
    LV_UNUSED(e);
    if (is_auto_mode) {
        is_running = !is_running;

        if (is_running) {
            // Start: continue in current direction (direction only reset on page create)
            if (sweep_timer) {
                lv_timer_resume(sweep_timer);
            }
        } else {
            // Stop: pause the timer
            if (sweep_timer) {
                lv_timer_pause(sweep_timer);
            }
        }

        update_bottom_buttons();
        update_mode_buttons();  // Update disabled state of mode buttons
    }
}

// Slider callback - update PWM when slider is dragged
static void slider_cb(lv_event_t* e) {
    lv_obj_t* slider = (lv_obj_t*)lv_event_get_target(e);
    int percent = lv_slider_get_value(slider);
    pwm_value = 1000 + percent * 10;  // Convert 0-100 to 1000-2000

    // Update PWM label (but not slider, as it's already at correct position)
    static char buf[16];
    snprintf(buf, sizeof(buf), "%d", pwm_value);
    lv_label_set_text(pwm_label, buf);
}

// Public function to adjust PWM from keyboard/encoder
void page_servo_adjust_pwm(int delta) {
    if (position_slider != nullptr) {
        pwm_value += delta;
        if (pwm_value < 1000) pwm_value = 1000;
        if (pwm_value > 2000) pwm_value = 2000;
        update_pwm_display();
    }
}

// Check if servo auto-sweep is running
bool page_servo_is_running() {
    return is_running && is_auto_mode;
}

// Stop servo and clean up timer
void page_servo_stop() {
    is_running = false;
    if (sweep_timer) {
        lv_timer_delete(sweep_timer);
        sweep_timer = nullptr;
    }
}

void page_servo_create(lv_obj_t* parent) {
    // Reset state
    pwm_value = 1500;
    is_auto_mode = true;
    is_running = false;
    sweep_direction = 1;

    // Create sweep timer (paused initially) - 20ms = 50Hz update
    if (sweep_timer) {
        lv_timer_delete(sweep_timer);
    }
    sweep_timer = lv_timer_create(sweep_timer_cb, 20, nullptr);
    lv_timer_pause(sweep_timer);

    // Flex column layout with even spacing
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_top(parent, 0, 0);
    lv_obj_set_style_pad_bottom(parent, 0, 0);

    // === Mode buttons row ===
    lv_obj_t* mode_row = lv_obj_create(parent);
    lv_obj_set_size(mode_row, LV_PCT(90), 35);
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
    lv_obj_set_style_text_font(lbl_auto, FONT_BUTTON_MD, 0);
    lv_obj_center(lbl_auto);
    lv_obj_add_event_cb(btn_auto, btn_auto_cb, LV_EVENT_CLICKED, nullptr);

    // Manual button
    btn_manual = lv_button_create(mode_row);
    lv_obj_set_size(btn_manual, 90, 30);
    lv_obj_set_style_bg_color(btn_manual, COLOR_BTN_INACTIVE, 0);
    lv_obj_t* lbl_manual = lv_label_create(btn_manual);
    lv_label_set_text(lbl_manual, tr(STR_SERVO_MODE_MANUAL));
    lv_obj_set_style_text_font(lbl_manual, FONT_BUTTON_MD, 0);
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

    // PWM label
    lv_obj_t* pwm_title = lv_label_create(pwm_container);
    lv_label_set_text_fmt(pwm_title, "%s (%s):", tr(STR_SERVO_PWM_LABEL), tr(STR_SERVO_US));
    lv_obj_set_style_text_font(pwm_title, FONT_BUTTON_MD, 0);
    lv_obj_set_style_text_color(pwm_title, lv_color_hex(GUI_COLOR_SHADES[7]), 0);

    // PWM value monospace - updated by encoder/auto mode
    pwm_label = lv_label_create(pwm_container);
    lv_label_set_text(pwm_label, "1500");
    lv_obj_set_style_text_font(pwm_label, FONT_MONO_BOLD_LG, 0);
    lv_obj_set_style_text_color(pwm_label, lv_color_hex(GUI_COLOR_MONO[0]), 0);

    // === Position Slider ===
    lv_obj_t* slider_container = lv_obj_create(parent);
    lv_obj_set_size(slider_container, LV_PCT(80), 35);
    lv_obj_set_style_bg_opa(slider_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(slider_container, 0, 0);
    lv_obj_set_style_pad_left(slider_container, 18, 0);   // Space for knob at left
    lv_obj_set_style_pad_right(slider_container, 18, 0);  // Space for knob at right
    lv_obj_set_style_pad_top(slider_container, 0, 0);
    lv_obj_set_style_pad_bottom(slider_container, 0, 0);
    lv_obj_set_flex_flow(slider_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(slider_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(slider_container, 2, 0);
    lv_obj_clear_flag(slider_container, LV_OBJ_FLAG_SCROLLABLE);  // Prevent clipping

    // Position slider - interactive in manual mode
    position_slider = lv_slider_create(slider_container);
    lv_obj_set_size(position_slider, LV_PCT(100), 20);
    lv_slider_set_range(position_slider, 0, 100);
    lv_slider_set_value(position_slider, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(position_slider, lv_color_hex(GUI_COLOR_GRAYS[7]), 0);
    lv_obj_set_style_bg_color(position_slider, lv_color_hex(GUI_COLOR_MONO[1]), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(position_slider, lv_color_hex(GUI_COLOR_MONO[0]), LV_PART_KNOB);
    lv_obj_add_event_cb(position_slider, slider_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    // === Start/Stop Button row (Auto mode) ===
    start_stop_row = lv_obj_create(parent);
    lv_obj_set_size(start_stop_row, LV_PCT(90), 35);
    lv_obj_set_style_bg_opa(start_stop_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(start_stop_row, 0, 0);
    lv_obj_set_style_pad_all(start_stop_row, 0, 0);
    lv_obj_set_flex_flow(start_stop_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(start_stop_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    btn_start_stop = lv_button_create(start_stop_row);
    lv_obj_set_size(btn_start_stop, 90, 30);
    lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN_ACTIVE, 0);
    lbl_start_stop = lv_label_create(btn_start_stop);
    lv_label_set_text(lbl_start_stop, tr(STR_SERVO_START));
    lv_obj_set_style_text_font(lbl_start_stop, FONT_BUTTON_MD, 0);
    lv_obj_center(lbl_start_stop);
    lv_obj_add_event_cb(btn_start_stop, btn_start_stop_cb, LV_EVENT_CLICKED, nullptr);

    // === Manual mode buttons row (MIN/CENTER/MAX) ===
    manual_btn_row = lv_obj_create(parent);
    lv_obj_set_size(manual_btn_row, LV_PCT(90), 35);
    lv_obj_set_style_bg_opa(manual_btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(manual_btn_row, 0, 0);
    lv_obj_set_style_pad_all(manual_btn_row, 0, 0);
    lv_obj_set_flex_flow(manual_btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(manual_btn_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(manual_btn_row, 10, 0);
    lv_obj_add_flag(manual_btn_row, LV_OBJ_FLAG_HIDDEN);  // Hidden initially (auto mode)

    // MIN button
    lv_obj_t* btn_min = lv_button_create(manual_btn_row);
    lv_obj_set_size(btn_min, 65, 30);
    lv_obj_set_style_bg_color(btn_min, lv_color_hex(GUI_COLOR_MONO[0]), 0);
    lv_obj_t* lbl_min = lv_label_create(btn_min);
    lv_label_set_text(lbl_min, "Min");
    lv_obj_set_style_text_font(lbl_min, FONT_BUTTON_MD, 0);
    lv_obj_center(lbl_min);
    lv_obj_add_event_cb(btn_min, btn_min_cb, LV_EVENT_CLICKED, nullptr);

    // CENTER button
    lv_obj_t* btn_center = lv_button_create(manual_btn_row);
    lv_obj_set_size(btn_center, 90, 30);
    lv_obj_set_style_bg_color(btn_center, lv_color_hex(GUI_COLOR_MONO[0]), 0);
    lv_obj_t* lbl_center = lv_label_create(btn_center);
    lv_label_set_text(lbl_center, "Center");
    lv_obj_set_style_text_font(lbl_center, FONT_BUTTON_MD, 0);
    lv_obj_center(lbl_center);
    lv_obj_add_event_cb(btn_center, btn_center_cb, LV_EVENT_CLICKED, nullptr);

    // MAX button
    lv_obj_t* btn_max = lv_button_create(manual_btn_row);
    lv_obj_set_size(btn_max, 65, 30);
    lv_obj_set_style_bg_color(btn_max, lv_color_hex(GUI_COLOR_MONO[0]), 0);
    lv_obj_t* lbl_max = lv_label_create(btn_max);
    lv_label_set_text(lbl_max, "Max");
    lv_obj_set_style_text_font(lbl_max, FONT_BUTTON_MD, 0);
    lv_obj_center(lbl_max);
    lv_obj_add_event_cb(btn_max, btn_max_cb, LV_EVENT_CLICKED, nullptr);

    // Initial display update
    update_pwm_display();
    update_mode_buttons();
    update_bottom_buttons();
}
