// gui/pages/page_servo.cpp - Servo tester page

#include "lvgl.h"
#include "gui/page_base.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include <algorithm>
#include <cstdio>

// Configuration constants
namespace {
    // PWM range (microseconds)
    constexpr int PWM_MIN     = 1000;
    constexpr int PWM_CENTER  = 1500;
    constexpr int PWM_MAX     = 2000;
    constexpr int PWM_STEP    = 10;

    // Timer interval (50Hz update rate)
    constexpr uint32_t SWEEP_INTERVAL_MS = 20;

    // UI dimensions
    constexpr lv_coord_t BTN_SM = 65, BTN_MD = 90, BTN_H = 30, ROW_H = 35;

    // Colors
    const lv_color_t COL_ACTIVE   = lv_color_hex(GUI_COLOR_TRIAD[1]);
    const lv_color_t COL_INACTIVE = lv_color_hex(GUI_COLOR_GRAYS[6]);
    const lv_color_t COL_STOP     = lv_color_hex(0xCC3333);
    const lv_color_t COL_PRIMARY  = lv_color_hex(GUI_COLOR_MONO[0]);
    const lv_color_t COL_TEXT     = lv_color_hex(GUI_COLOR_SHADES[7]);
}

// State
struct ServoState {
    int pwm = PWM_CENTER;
    bool auto_mode = true;
    bool running = false;
    int direction = 1;
    lv_timer_t* timer = nullptr;

    // UI elements we need to update
    lv_obj_t* lbl_pwm = nullptr;
    lv_obj_t* slider = nullptr;
    lv_obj_t* btn_auto = nullptr;
    lv_obj_t* btn_manual = nullptr;
    lv_obj_t* btn_start = nullptr;
    lv_obj_t* lbl_start = nullptr;
    lv_obj_t* row_start = nullptr;
    lv_obj_t* row_manual = nullptr;

    void reset() { pwm = PWM_CENTER; auto_mode = true; running = false; direction = 1; }
    void clamp() { pwm = std::clamp(pwm, PWM_MIN, PWM_MAX); }

    void update_display() {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", pwm);
        lv_label_set_text(lbl_pwm, buf);
        lv_slider_set_value(slider, (pwm - PWM_MIN) / PWM_STEP, LV_ANIM_ON);
    }

    void update_ui() {
        // Mode buttons
        lv_obj_set_style_bg_color(btn_auto, auto_mode ? COL_ACTIVE : COL_INACTIVE, 0);
        lv_obj_set_style_bg_color(btn_manual, auto_mode ? COL_INACTIVE : COL_ACTIVE, 0);
        lv_obj_set_style_opa(btn_auto, running ? LV_OPA_50 : LV_OPA_100, 0);
        lv_obj_set_style_opa(btn_manual, running ? LV_OPA_50 : LV_OPA_100, 0);
        if (running) {
            lv_obj_add_state(btn_auto, LV_STATE_DISABLED);
            lv_obj_add_state(btn_manual, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(btn_auto, LV_STATE_DISABLED);
            lv_obj_clear_state(btn_manual, LV_STATE_DISABLED);
        }

        // Bottom row visibility
        if (auto_mode) {
            lv_obj_clear_flag(row_start, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(row_manual, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(lbl_start, running ? tr(STR_SERVO_STOP) : tr(STR_SERVO_START));
            lv_obj_set_style_bg_color(btn_start, running ? COL_STOP : COL_ACTIVE, 0);
        } else {
            lv_obj_add_flag(row_start, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(row_manual, LV_OBJ_FLAG_HIDDEN);
        }
    }
};

static ServoState S;

// UI Helpers
static lv_obj_t* make_row(lv_obj_t* p, int w_pct = 90, int h = ROW_H) {
    lv_obj_t* r = lv_obj_create(p);
    lv_obj_set_size(r, LV_PCT(w_pct), h);
    lv_obj_set_style_bg_opa(r, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(r, 0, 0);
    lv_obj_set_style_pad_all(r, 0, 0);
    lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(r, 10, 0);
    return r;
}

static lv_obj_t* make_btn(lv_obj_t* p, const char* txt, int w, lv_color_t col, lv_event_cb_t cb, void* data = &S) {
    lv_obj_t* b = lv_button_create(p);
    lv_obj_set_size(b, w, BTN_H);
    lv_obj_set_style_bg_color(b, col, 0);
    lv_obj_t* l = lv_label_create(b);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_font(l, FONT_BUTTON_MD, 0);
    lv_obj_center(l);
    if (cb) lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, data);
    return b;
}

// Callbacks
static void on_timer(lv_timer_t* t) {
    if (!S.running || !S.auto_mode) return;
    S.pwm += S.direction * PWM_STEP;
    if (S.pwm >= PWM_MAX) { S.pwm = PWM_MAX; S.direction = -1; }
    else if (S.pwm <= PWM_MIN) { S.pwm = PWM_MIN; S.direction = 1; }
    S.update_display();
}

static void on_auto(lv_event_t*) {
    if (S.running) return;
    S.auto_mode = true;
    S.update_ui();
}

static void on_manual(lv_event_t*) {
    if (S.running) return;
    S.auto_mode = false;
    S.running = false;
    if (S.timer) lv_timer_pause(S.timer);
    S.update_ui();
}

static void on_start_stop(lv_event_t*) {
    if (!S.auto_mode) return;
    S.running = !S.running;
    if (S.timer) S.running ? lv_timer_resume(S.timer) : lv_timer_pause(S.timer);
    S.update_ui();
}

static void on_preset(lv_event_t* e) {
    S.pwm = reinterpret_cast<intptr_t>(lv_event_get_user_data(e));
    S.update_display();
}

static void on_slider(lv_event_t* e) {
    S.pwm = PWM_MIN + lv_slider_get_value(lv_event_get_target_obj(e)) * PWM_STEP;
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", S.pwm);
    lv_label_set_text(S.lbl_pwm, buf);
}

// Public API
void page_servo_create(lv_obj_t* parent) {
    S.reset();

    // Timer
    if (S.timer) lv_timer_delete(S.timer);
    S.timer = lv_timer_create(on_timer, SWEEP_INTERVAL_MS, nullptr);
    lv_timer_pause(S.timer);

    // Layout
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    // Mode row
    lv_obj_t* row_mode = make_row(parent);
    S.btn_auto = make_btn(row_mode, tr(STR_SERVO_MODE_AUTO), BTN_MD, COL_ACTIVE, on_auto);
    S.btn_manual = make_btn(row_mode, tr(STR_SERVO_MODE_MANUAL), BTN_MD, COL_INACTIVE, on_manual);

    // PWM display row
    lv_obj_t* row_pwm = make_row(parent, 90, 30);
    lv_obj_set_style_pad_column(row_pwm, 5, 0);
    lv_obj_t* lbl_title = lv_label_create(row_pwm);
    lv_label_set_text_fmt(lbl_title, "%s (%s):", tr(STR_SERVO_PWM_LABEL), tr(STR_SERVO_US));
    lv_obj_set_style_text_font(lbl_title, FONT_BUTTON_MD, 0);
    lv_obj_set_style_text_color(lbl_title, COL_TEXT, 0);
    S.lbl_pwm = lv_label_create(row_pwm);
    lv_label_set_text(S.lbl_pwm, "1500");
    lv_obj_set_style_text_font(S.lbl_pwm, FONT_MONO_BOLD_LG, 0);
    lv_obj_set_style_text_color(S.lbl_pwm, COL_PRIMARY, 0);

    // Slider
    lv_obj_t* row_slider = lv_obj_create(parent);
    lv_obj_set_size(row_slider, LV_PCT(80), ROW_H);
    lv_obj_set_style_bg_opa(row_slider, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row_slider, 0, 0);
    lv_obj_set_style_pad_hor(row_slider, 18, 0);
    lv_obj_set_style_pad_ver(row_slider, 0, 0);
    lv_obj_set_flex_flow(row_slider, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row_slider, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row_slider, LV_OBJ_FLAG_SCROLLABLE);

    S.slider = lv_slider_create(row_slider);
    lv_obj_set_size(S.slider, LV_PCT(100), 20);
    lv_slider_set_range(S.slider, 0, 100);
    lv_slider_set_value(S.slider, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(S.slider, lv_color_hex(GUI_COLOR_GRAYS[7]), 0);
    lv_obj_set_style_bg_color(S.slider, lv_color_hex(GUI_COLOR_MONO[1]), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(S.slider, COL_PRIMARY, LV_PART_KNOB);
    lv_obj_add_event_cb(S.slider, on_slider, LV_EVENT_VALUE_CHANGED, nullptr);

    // Start/Stop row (auto mode)
    S.row_start = make_row(parent);
    lv_obj_set_style_pad_column(S.row_start, 0, 0);
    S.btn_start = lv_button_create(S.row_start);
    lv_obj_set_size(S.btn_start, BTN_MD, BTN_H);
    lv_obj_set_style_bg_color(S.btn_start, COL_ACTIVE, 0);
    S.lbl_start = lv_label_create(S.btn_start);
    lv_label_set_text(S.lbl_start, tr(STR_SERVO_START));
    lv_obj_set_style_text_font(S.lbl_start, FONT_BUTTON_MD, 0);
    lv_obj_center(S.lbl_start);
    lv_obj_add_event_cb(S.btn_start, on_start_stop, LV_EVENT_CLICKED, nullptr);

    // Manual preset row (hidden initially)
    S.row_manual = make_row(parent);
    lv_obj_add_flag(S.row_manual, LV_OBJ_FLAG_HIDDEN);
    make_btn(S.row_manual, "Min", BTN_SM, COL_PRIMARY, on_preset, (void*)(intptr_t)PWM_MIN);
    make_btn(S.row_manual, "Center", BTN_MD, COL_PRIMARY, on_preset, (void*)(intptr_t)PWM_CENTER);
    make_btn(S.row_manual, "Max", BTN_SM, COL_PRIMARY, on_preset, (void*)(intptr_t)PWM_MAX);

    S.update_display();
    S.update_ui();
}

void page_servo_destroy() {
    S.running = false;
    if (S.timer) { lv_timer_delete(S.timer); S.timer = nullptr; }
    S = ServoState{};  // Reset all pointers
}

void page_servo_on_hide() {
    // Stop sweep when leaving page (but don't destroy timer)
    if (S.running) {
        S.running = false;
        if (S.timer) lv_timer_pause(S.timer);
        S.update_ui();
    }
}

bool page_servo_is_running() { return S.running && S.auto_mode; }

void page_servo_stop() {
    S.running = false;
    if (S.timer) { lv_timer_delete(S.timer); S.timer = nullptr; }
}

void page_servo_adjust_pwm(int delta) {
    if (S.slider) { S.pwm += delta; S.clamp(); S.update_display(); }
}

void page_servo_toggle_sweep() {
    if (!S.btn_start) return;
    // Simulate clicking the start/stop button
    lv_obj_send_event(S.btn_start, LV_EVENT_CLICKED, nullptr);
}
