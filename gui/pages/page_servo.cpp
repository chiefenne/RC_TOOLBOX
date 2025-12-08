// gui/pages/page_servo.cpp - Servo tester page (multi-servo support)

#include "lvgl.h"
#include "gui/page_base.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include "gui/config/settings.h"
#include "servo_driver.h"
#include <algorithm>
#include <cstdio>

// Configuration constants
namespace {
    // Number of servos
    constexpr int NUM_SERVOS = 6;

    // PWM values now come from settings - these are just for step size
    constexpr int PWM_STEP    = 10;

    // Timer interval (50Hz update rate)
    constexpr uint32_t SWEEP_INTERVAL_MS = 20;

    // UI dimensions
    constexpr lv_coord_t BTN_SM = 65, BTN_MD = 90, BTN_H = 30, ROW_H = 35;
    constexpr lv_coord_t SIDEBAR_W = 42;  // Width of servo selection sidebar
    constexpr lv_coord_t SERVO_BTN_H = 22; // Height of servo toggle buttons

    // Colors
    const lv_color_t COL_ACTIVE   = lv_color_hex(GUI_COLOR_TRIAD[1]);
    const lv_color_t COL_INACTIVE = lv_color_hex(GUI_COLOR_GRAYS[6]);
    const lv_color_t COL_STOP     = lv_color_hex(0xCC3333);
    const lv_color_t COL_PRIMARY  = lv_color_hex(GUI_COLOR_MONO[0]);
    const lv_color_t COL_TEXT     = lv_color_hex(GUI_COLOR_SHADES[7]);
    const lv_color_t COL_SERVO_ON = lv_color_hex(GUI_COLOR_MONO[1]);  // Blue when selected

    // Helper macros to get PWM values from settings
    #define PWM_MIN    ((int)g_settings.servo_pwm_min)
    #define PWM_CENTER ((int)g_settings.servo_pwm_center)
    #define PWM_MAX    ((int)g_settings.servo_pwm_max)
}

// State
struct ServoState {
    int pwm = 1500;  // Will be reset to PWM_CENTER on page load
    bool auto_mode = true;
    bool running = false;
    int direction = 1;
    lv_timer_t* timer = nullptr;

    // Servo selection (bitmask: bit 0 = servo 1, etc.)
    uint8_t selected = 0x01;  // Default: servo 1 selected
    lv_obj_t* btn_servo[NUM_SERVOS] = {};

    // UI elements we need to update
    lv_obj_t* lbl_pwm = nullptr;
    lv_obj_t* slider = nullptr;
    lv_obj_t* btn_auto = nullptr;
    lv_obj_t* btn_manual = nullptr;
    lv_obj_t* btn_start = nullptr;
    lv_obj_t* lbl_start = nullptr;
    lv_obj_t* row_start = nullptr;
    lv_obj_t* row_manual = nullptr;

    void reset() { pwm = PWM_CENTER; auto_mode = true; running = false; direction = 1; selected = 0x01; }
    void clamp() { pwm = (pwm < PWM_MIN) ? PWM_MIN : (pwm > PWM_MAX) ? PWM_MAX : pwm; }

    bool is_servo_selected(int idx) const { return (selected & (1 << idx)) != 0; }
    void toggle_servo(int idx) { selected ^= (1 << idx); }
    void select_all() { selected = (1 << NUM_SERVOS) - 1; }  // All bits set
    void deselect_all() { selected = 0; }
    bool all_selected() const { return selected == ((1 << NUM_SERVOS) - 1); }
    bool any_selected() const { return selected != 0; }

    void update_servo_buttons() {
        for (int i = 0; i < NUM_SERVOS; i++) {
            if (btn_servo[i]) {
                lv_obj_set_style_bg_color(btn_servo[i],
                    is_servo_selected(i) ? COL_SERVO_ON : COL_INACTIVE, 0);
            }
        }
    }

    void update_display() {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", pwm);
        lv_label_set_text(lbl_pwm, buf);
        // Scale slider to 0-100 range based on current PWM range
        int range = PWM_MAX - PWM_MIN;
        int slider_val = (range > 0) ? ((pwm - PWM_MIN) * 100 / range) : 50;
        lv_slider_set_value(slider, slider_val, LV_ANIM_ON);
        // Output PWM to selected servos
        servo_set_pulse_mask(selected, static_cast<uint16_t>(pwm));
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

// Servo selection callbacks
static void on_servo_toggle(lv_event_t* e) {
    int idx = reinterpret_cast<intptr_t>(lv_event_get_user_data(e));
    S.toggle_servo(idx);
    S.update_servo_buttons();
    // Enable/disable PWM output for this servo
    bool enabled = S.is_servo_selected(idx);
    servo_enable(static_cast<uint8_t>(idx), enabled);
    if (enabled) {
        servo_set_pulse(static_cast<uint8_t>(idx), static_cast<uint16_t>(S.pwm));
    }
}

// Long-press on any servo button toggles ALL
static void on_servo_long_press(lv_event_t*) {
    if (S.all_selected()) {
        S.deselect_all();
        servo_disable_all();
    } else {
        S.select_all();
        servo_enable_mask(S.selected, true);
        servo_set_pulse_mask(S.selected, static_cast<uint16_t>(S.pwm));
    }
    S.update_servo_buttons();
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
    // Slider is 0-100, map to PWM range from settings
    int slider_val = lv_slider_get_value(lv_event_get_target_obj(e));
    int range = PWM_MAX - PWM_MIN;
    S.pwm = PWM_MIN + (slider_val * range / 100);
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

    // Main layout: horizontal row with sidebar + content
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(parent, 0, 0);

    // === SIDEBAR: Servo selection ===
    lv_obj_t* sidebar = lv_obj_create(parent);
    lv_obj_remove_style_all(sidebar);
    lv_obj_set_size(sidebar, SIDEBAR_W, LV_PCT(100));
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sidebar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(sidebar, 4, 0);
    lv_obj_set_style_pad_row(sidebar, 2, 0);
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(GUI_COLOR_GRAYS[9]), 0);
    lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, 0);
    lv_obj_clear_flag(sidebar, LV_OBJ_FLAG_SCROLLABLE);

    // Servo toggle buttons (1-6)
    // Short tap = toggle single servo, Long press = toggle ALL
    for (int i = 0; i < NUM_SERVOS; i++) {
        S.btn_servo[i] = lv_button_create(sidebar);
        lv_obj_set_size(S.btn_servo[i], SIDEBAR_W - 8, SERVO_BTN_H);
        lv_obj_set_style_bg_color(S.btn_servo[i], i == 0 ? COL_SERVO_ON : COL_INACTIVE, 0);
        lv_obj_set_style_radius(S.btn_servo[i], 4, 0);
        lv_obj_t* lbl = lv_label_create(S.btn_servo[i]);
        lv_label_set_text_fmt(lbl, "%d", i + 1);
        lv_obj_set_style_text_font(lbl, &arial_14, 0);
        lv_obj_center(lbl);
        // Short click = toggle this servo
        lv_obj_add_event_cb(S.btn_servo[i], on_servo_toggle, LV_EVENT_SHORT_CLICKED, (void*)(intptr_t)i);
        // Long press = toggle ALL servos
        lv_obj_add_event_cb(S.btn_servo[i], on_servo_long_press, LV_EVENT_LONG_PRESSED, nullptr);
    }

    // === CONTENT: Controls ===
    lv_obj_t* content = lv_obj_create(parent);
    lv_obj_remove_style_all(content);
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_height(content, LV_PCT(100));
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    // Mode row
    lv_obj_t* row_mode = make_row(content);
    S.btn_auto = make_btn(row_mode, tr(STR_SERVO_MODE_AUTO), BTN_MD, COL_ACTIVE, on_auto);
    S.btn_manual = make_btn(row_mode, tr(STR_SERVO_MODE_MANUAL), BTN_MD, COL_INACTIVE, on_manual);

    // PWM display row
    lv_obj_t* row_pwm = make_row(content, 90, 30);
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
    lv_obj_t* row_slider = lv_obj_create(content);
    lv_obj_set_size(row_slider, LV_PCT(85), ROW_H);
    lv_obj_set_style_bg_opa(row_slider, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row_slider, 0, 0);
    lv_obj_set_style_pad_hor(row_slider, 10, 0);
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
    S.row_start = make_row(content);
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
    S.row_manual = make_row(content);
    lv_obj_add_flag(S.row_manual, LV_OBJ_FLAG_HIDDEN);
    make_btn(S.row_manual, "Min", BTN_SM, COL_PRIMARY, on_preset, (void*)(intptr_t)PWM_MIN);
    make_btn(S.row_manual, "Center", BTN_MD, COL_PRIMARY, on_preset, (void*)(intptr_t)PWM_CENTER);
    make_btn(S.row_manual, "Max", BTN_SM, COL_PRIMARY, on_preset, (void*)(intptr_t)PWM_MAX);

    S.update_display();
    S.update_ui();
    S.update_servo_buttons();

    // Enable initially selected servo(s) and set pulse
    servo_enable_mask(S.selected, true);
    servo_set_pulse_mask(S.selected, static_cast<uint16_t>(S.pwm));
}

void page_servo_destroy() {
    S.running = false;
    servo_disable_all();  // Stop all servo outputs
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
    servo_disable_all();  // Stop all servo outputs when hiding
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
