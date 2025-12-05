// gui/pages/page_servo.cpp
#include "page_servo.hpp"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include <algorithm>
#include <cstdio>

// ====================== GLOBAL INSTANCE ======================
ServoPage servo_page;  // This is the one and only instance

// ====================== CONSTANTS ======================
constexpr int SERVO_MIN_PWM       = 1000;
constexpr int SERVO_MAX_PWM       = 2000;
constexpr int SERVO_CENTER_PWM    = 1500;
constexpr int SERVO_STEP_US       = 10;
constexpr int SERVO_SWEEP_MS      = 20;        // ~50 Hz
constexpr int SERVO_SLIDER_STEPS  = 100;

constexpr lv_color_t COLOR_ACTIVE   = lv_color_hex(GUI_COLOR_TRIAD[1]);   // Green
constexpr lv_color_t COLOR_INACTIVE = lv_color_hex(GUI_COLOR_GRAYS[6]);   // Gray
constexpr lv_color_t COLOR_STOP     = lv_color_hex(0xCC3333);             // Red

// ====================== PRIVATE STATE (inside the global object) ======================
struct ServoPage::State {
    int pwm_value = SERVO_CENTER_PWM;
    bool is_auto_mode = true;
    bool is_running = false;
    int sweep_direction = 1;

    lv_obj_t* pwm_label = nullptr;
    lv_obj_t* position_slider = nullptr;
    lv_obj_t* btn_auto = nullptr;
    lv_obj_t* btn_manual = nullptr;
    lv_obj_t* btn_start_stop = nullptr;
    lv_obj_t* lbl_start_stop = nullptr;
    lv_obj_t* start_stop_row = nullptr;
    lv_obj_t* manual_btn_row = nullptr;

    lv_timer_t* sweep_timer = nullptr;
};

static ServoPage::State& s = servo_page.state;  // shorthand

// ====================== HELPERS ======================
static lv_obj_t* create_btn(lv_obj_t* parent, const char* text, lv_event_cb_t cb = nullptr, lv_coord_t w = 90)
{
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_size(btn, w, 30);
    lv_obj_set_style_bg_color(btn, lv_color_hex(GUI_COLOR_MONO[0]), 0);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, FONT_BUTTON_MD, 0);
    lv_obj_center(label);

    if (cb) lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
    return btn;
}

static void update_pwm_display()
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", s.pwm_value);
    lv_label_set_text(s.pwm_label, buf);

    int percent = (s.pwm_value - SERVO_MIN_PWM) * SERVO_SLIDER_STEPS / (SERVO_MAX_PWM - SERVO_MIN_PWM);
    lv_slider_set_value(s.position_slider, percent, LV_ANIM_ON);
}

static void sweep_timer_cb(lv_timer_t* timer)
{
    LV_UNUSED(timer);
    if (!s.is_running || !s.is_auto_mode) return;

    s.pwm_value += s.sweep_direction * SERVO_STEP_US;

    if (s.pwm_value >= SERVO_MAX_PWM) {
        s.pwm_value = SERVO_MAX_PWM;
        s.sweep_direction = -1;
    } else if (s.pwm_value <= SERVO_MIN_PWM) {
        s.pwm_value = SERVO_MIN_PWM;
        s.sweep_direction = 1;
    }

    update_pwm_display();
}

// ====================== CALLBACKS ======================
static void btn_auto_cb(lv_event_t*)     { if (!s.is_running) { s.is_auto_mode = true;  lv_obj_set_style_bg_color(s.btn_auto, COLOR_ACTIVE, 0);   lv_obj_set_style_bg_color(s.btn_manual, COLOR_INACTIVE, 0);  update_bottom(); } }
static void btn_manual_cb(lv_event_t*)   { if (!s.is_running) { s.is_auto_mode = false; s.is_running = false; lv_timer_pause(s.sweep_timer); lv_obj_set_style_bg_color(s.btn_auto, COLOR_INACTIVE, 0); lv_obj_set_style_bg_color(s.btn_manual, COLOR_ACTIVE, 0); update_bottom(); } }
static void btn_start_stop_cb(lv_event_t*)
{
    if (!s.is_auto_mode) return;
    s.is_running = !s.is_running;
    if (s.is_running) lv_timer_resume(s.sweep_timer);
    else               lv_timer_pause(s.sweep_timer);
    update_bottom();
}

static void btn_min_cb(lv_event_t*)    { servo_page.set_pwm(SERVO_MIN_PWM); }
static void btn_center_cb(lv_event_t*){ servo_page.set_pwm(SERVO_CENTER_PWM); }
static void btn_max_cb(lv_event_t*)    { servo_page.set_pwm(SERVO_MAX_PWM); }

static void slider_cb(lv_event_t* e)
{
    int val = lv_slider_get_value(lv_event_get_target(e));
    int pwm = SERVO_MIN_PWM + val * (SERVO_MAX_PWM - SERVO_MIN_PWM) / SERVO_SLIDER_STEPS;
    servo_page.set_pwm(pwm);
}

static void update_bottom()
{
    if (s.is_auto_mode) {
        lv_obj_clear_flag(s.start_stop_row, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s.manual_btn_row, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(s.lbl_start_stop, s.is_running ? tr(STR_SERVO_STOP) : tr(STR_SERVO_START));
        lv_obj_set_style_bg_color(s.btn_start_stop, s.is_running ? COLOR_STOP : COLOR_ACTIVE, 0);
    } else {
        lv_obj_add_flag(s.start_stop_row, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s.manual_btn_row, LV_OBJ_FLAG_HIDDEN);
    }

    // Disable mode buttons while running
    bool disabled = s.is_running;
    lv_obj_set_state(s.btn_auto,   disabled ? LV_STATE_DISABLED : 0, LV_STATE_DISABLED);
    lv_obj_set_state(s.btn_manual, disabled ? LV_STATE_DISABLED : 0, LV_STATE_DISABLED);
    lv_obj_set_style_opa(s.btn_auto,   disabled ? LV_OPA_50 : LV_OPA_100, 0);
    lv_obj_set_style_opa(s.btn_manual, disabled ? LV_OPA_50 : LV_OPA_100, 0);
}

// ====================== PUBLIC METHODS ======================
void ServoPage::create(lv_obj_t* parent)
{
    destroy();  // Safety first

    // Reset state
    s = {};
    s.pwm_value = SERVO_CENTER_PWM;
    s.is_auto_mode = true;
    s.sweep_direction = 1;

    s.sweep_timer = lv_timer_create(sweep_timer_cb, SERVO_SWEEP_MS, nullptr);
    lv_timer_pause(s.sweep_timer);

    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clean(parent);

    // === Mode Row ===
    auto* mode_row = lv_obj_create(parent);
    lv_obj_set_size(mode_row, LV_PCT(90), 35);
    lv_obj_set_style_bg_opa(mode_row, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(mode_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(mode_row, 10, 0);

    s.btn_auto   = create_btn(mode_row, tr(STR_SERVO_MODE_AUTO),   btn_auto_cb);
    s.btn_manual = create_btn(mode_row, tr(STR_SERVO_MODE_MANUAL), btn_manual_cb);
    lv_obj_set_style_bg_color(s.btn_auto, COLOR_ACTIVE, 0);

    // === PWM Display ===
    auto* pwm_cont = lv_obj_create(parent);
    lv_obj_set_size(pwm_cont, LV_PCT(90), 30);
    lv_obj_set_style_bg_opa(pwm_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(pwm_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(pwm_cont, 5, 0);

    auto* title = lv_label_create(pwm_cont);
    lv_label_set_text_fmt(title, "%s (%s):", tr(STR_SERVO_PWM_LABEL), tr(STR_SERVO_US));
    lv_obj_set_style_text_font(title, FONT_BUTTON_MD, 0);

    s.pwm_label = lv_label_create(pwm_cont);
    lv_obj_set_style_text_font(s.pwm_label, FONT_MONO_BOLD_LG, 0);
    lv_obj_set_style_text_color(s.pwm_label, lv_color_hex(GUI_COLOR_MONO[0]), 0);

    // === Slider ===
    auto* slider_cont = lv_obj_create(parent);
    lv_obj_set_size(slider_cont, LV_PCT(80), 40);
    lv_obj_set_style_bg_opa(slider_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_hor(slider_cont, 18, 0);

    s.position_slider = lv_slider_create(slider_cont);
    lv_obj_set_width(s.position_slider, LV_PCT(100));
    lv_slider_set_range(s.position_slider, 0, SERVO_SLIDER_STEPS);
    lv_slider_set_value(s.position_slider, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s.position_slider, lv_color_hex(GUI_COLOR_MONO[1]), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s.position_slider, lv_color_hex(GUI_COLOR_MONO[0]), LV_PART_KNOB);
    lv_obj_add_event_cb(s.position_slider, slider_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    // === Start/Stop Row ===
    s.start_stop_row = lv_obj_create(parent);
    lv_obj_set_size(s.start_stop_row, LV_PCT(90), 35);
    lv_obj_set_style_bg_opa(s.start_stop_row, LV_OPA_TRANSP, 0);

    s.btn_start_stop = create_btn(s.start_stop_row, tr(STR_SERVO_START), btn_start_stop_cb);
    s.lbl_start_stop = lv_obj_get_child(s.btn_start_stop, 0);  // label is first child

    // === Manual Buttons Row ===
    s.manual_btn_row = lv_obj_create(parent);
    lv_obj_set_size(s.manual_btn_row, LV_PCT(90), 35);
    lv_obj_set_style_bg_opa(s.manual_btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(s.manual_btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(s.manual_btn_row, 10, 0);
    lv_obj_add_flag(s.manual_btn_row, LV_OBJ_FLAG_HIDDEN);

    create_btn(s.manual_btn_row, "Min",    btn_min_cb,    65);
    create_btn(s.manual_btn_row, "Center", btn_center_cb, 90);
    create_btn(s.manual_btn_row, "Max",    btn_max_cb,    65);

    // Initial state
    update_pwm_display();
    update_bottom();
}

void ServoPage::destroy()
{
    if (s.sweep_timer) {
        lv_timer_delete(s.sweep_timer);
        s.sweep_timer = nullptr;
    }
    s = {};
}

void ServoPage::on_hide()
{
    s.is_running = false;
    if (s.sweep_timer) lv_timer_pause(s.sweep_timer);
    update_bottom();
}

void ServoPage::adjust_encoder(int delta)
{
    if (!s.position_slider) return;
    int new_val = s.pwm_value + delta * 10;
    new_val = std::clamp(new_val, SERVO_MIN_PWM, SERVO_MAX_PWM);
    set_pwm(new_val);
}

bool ServoPage::is_background_task_running() const
{
    return s.is_running && s.is_auto_mode;
}

void ServoPage::set_pwm(int value_us)
{
    s.pwm_value = std::clamp(value_us, SERVO_MIN_PWM, SERVO_MAX_PWM);
    update_pwm_display();
    // TODO: Actually drive the servo PWM here if needed
}
