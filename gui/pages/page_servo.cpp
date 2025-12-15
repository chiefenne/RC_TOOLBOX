// gui/pages/page_servo.cpp - Servo tester page (multi-servo support)

#include "lvgl.h"
#include "gui/page_base.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include "gui/config/settings.h"
#include "gui/input.h"
#include "gui/gui.h"
#include "servo_driver.h"
#include <algorithm>
#include <cstdio>
#include <cstring>  // for strcmp

// =============================================================================
// Focus Order Configuration
// =============================================================================
// Define focus order here - change these numbers to reorder navigation
// Servo buttons are NOT in focus order - use touch only for servo selection
// Focus order: Mode buttons, slider, start/presets, footer
enum FocusOrder {
    FO_AUTO       = 0,
    FO_MANUAL     = 1,
    FO_SLIDER     = 2,
    FO_START      = 3,
    FO_MIN        = 4,
    FO_CENTER     = 5,
    FO_MAX        = 6,
    FO_BTN_HOME   = 7,
    FO_BTN_PREV   = 8,
    FO_BTN_NEXT   = 9,
    FO_BTN_SETTINGS = 10,
};

// Focus group builder for this page
static FocusOrderBuilder focus_builder;

// Configuration constants
namespace {
    // Number of servos (use from settings.h)
    // constexpr int NUM_SERVOS = 6;  // Now defined in settings.h

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
    // Per-servo PWM values (each servo maintains its own position)
    int pwm[NUM_SERVOS] = {1500, 1500, 1500, 1500, 1500, 1500};

    // Per-servo sweep direction (1 = increasing, -1 = decreasing)
    int direction[NUM_SERVOS] = {1, 1, 1, 1, 1, 1};

    bool auto_mode = true;
    bool running = false;
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
    // Preset buttons (need individual tracking for focus management)
    lv_obj_t* btn_min = nullptr;
    lv_obj_t* btn_center = nullptr;
    lv_obj_t* btn_max = nullptr;

    void reset() {
        for (int i = 0; i < NUM_SERVOS; i++) {
            pwm[i] = PWM_CENTER;
            direction[i] = 1;
        }
        auto_mode = true; running = false; selected = 0x01;
    }

    // Get PWM step for a servo from settings
    int get_pwm_step(int idx) const {
        if (idx >= 0 && idx < NUM_SERVOS) {
            return g_settings.servo_pwm_step[idx];
        }
        return DEFAULT_PWM_STEP;
    }

    // Clamp a single servo's PWM value
    void clamp(int idx) {
        if (idx >= 0 && idx < NUM_SERVOS) {
            pwm[idx] = (pwm[idx] < PWM_MIN) ? PWM_MIN : (pwm[idx] > PWM_MAX) ? PWM_MAX : pwm[idx];
        }
    }

    // Clamp all selected servos
    void clamp_selected() {
        for (int i = 0; i < NUM_SERVOS; i++) {
            if (is_servo_selected(i)) clamp(i);
        }
    }

    // Get the primary (lowest numbered) selected servo index, or -1 if none
    int get_primary_servo() const {
        for (int i = 0; i < NUM_SERVOS; i++) {
            if (is_servo_selected(i)) return i;
        }
        return -1;
    }

    // Get PWM of primary servo (for display)
    int get_display_pwm() const {
        int primary = get_primary_servo();
        return (primary >= 0) ? pwm[primary] : PWM_CENTER;
    }

    // Check if multiple servos are selected
    bool is_multi_select() const {
        int count = 0;
        for (int i = 0; i < NUM_SERVOS; i++) {
            if (is_servo_selected(i)) count++;
            if (count > 1) return true;
        }
        return false;
    }

    // Apply a delta to all selected servos (relative adjustment)
    void apply_delta(int delta) {
        for (int i = 0; i < NUM_SERVOS; i++) {
            if (is_servo_selected(i)) {
                pwm[i] += delta;
                clamp(i);
            }
        }
    }

    // Set all selected servos to a specific value (sync/preset)
    void set_all_selected(int value) {
        for (int i = 0; i < NUM_SERVOS; i++) {
            if (is_servo_selected(i)) {
                pwm[i] = value;
                clamp(i);
            }
        }
    }

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
        // === OPTIMIZATION: Only update widgets if value actually changed ===
        // This prevents unnecessary LVGL invalidations and reduces flicker.
        // Key: Use lv_label_set_text_static when possible, and always use LV_ANIM_OFF
        // during continuous updates to prevent animation queue buildup.

        // 1. Get display PWM (primary servo's value)
        int display_pwm = get_display_pwm();
        bool multi = is_multi_select();

        // 2. Update label only if text changed (avoid redundant invalidation)
        //    Show asterisk (*) suffix when multiple servos selected
        static char pwm_buf[12];  // Static to persist for lv_label_set_text_static
        char new_buf[12];
        if (multi) {
            snprintf(new_buf, sizeof(new_buf), "%d*", display_pwm);
        } else {
            snprintf(new_buf, sizeof(new_buf), "%d", display_pwm);
        }
        const char* current_text = lv_label_get_text(lbl_pwm);
        if (strcmp(current_text, new_buf) != 0) {
            // Copy to static buffer and use set_text_static to avoid internal allocation
            strcpy(pwm_buf, new_buf);
            lv_label_set_text_static(lbl_pwm, pwm_buf);
        }

        // 3. Scale slider to 0-100 range based on current PWM range
        int range = PWM_MAX - PWM_MIN;
        int slider_val = (range > 0) ? ((display_pwm - PWM_MIN) * 100 / range) : 50;

        // 4. Update slider only if value changed
        //    ALWAYS use LV_ANIM_OFF during sweep - animations cause multiple redraws
        int current_slider_val = lv_slider_get_value(slider);
        if (current_slider_val != slider_val) {
            lv_slider_set_value(slider, slider_val, LV_ANIM_OFF);
        }

        // 5. Output PWM to each selected servo with its individual value
        for (int i = 0; i < NUM_SERVOS; i++) {
            if (is_servo_selected(i)) {
                servo_set_pulse(static_cast<uint8_t>(i), static_cast<uint16_t>(pwm[i]));
            }
        }
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

        // Bottom row visibility + individual button visibility for focus navigation
        // LVGL focus navigation checks the widget's own HIDDEN flag, not parent's
        if (auto_mode) {
            lv_obj_clear_flag(row_start, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(btn_start, LV_OBJ_FLAG_HIDDEN);  // Show start button for focus
            lv_obj_add_flag(row_manual, LV_OBJ_FLAG_HIDDEN);
            // Disable slider in auto mode - still visible but focus skips disabled widgets
            if (slider) {
                lv_obj_add_state(slider, LV_STATE_DISABLED);
            }
            // Hide preset buttons so focus skips them
            if (btn_min) lv_obj_add_flag(btn_min, LV_OBJ_FLAG_HIDDEN);
            if (btn_center) lv_obj_add_flag(btn_center, LV_OBJ_FLAG_HIDDEN);
            if (btn_max) lv_obj_add_flag(btn_max, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(lbl_start, running ? tr(STR_SERVO_STOP) : tr(STR_SERVO_START));
            lv_obj_set_style_bg_color(btn_start, running ? COL_STOP : COL_ACTIVE, 0);
        } else {
            lv_obj_add_flag(row_start, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(btn_start, LV_OBJ_FLAG_HIDDEN);  // Hide start button for focus
            lv_obj_clear_flag(row_manual, LV_OBJ_FLAG_HIDDEN);
            // Enable slider in manual mode so it can be focused
            if (slider) {
                lv_obj_clear_state(slider, LV_STATE_DISABLED);
            }
            // Show preset buttons so focus includes them
            if (btn_min) lv_obj_clear_flag(btn_min, LV_OBJ_FLAG_HIDDEN);
            if (btn_center) lv_obj_clear_flag(btn_center, LV_OBJ_FLAG_HIDDEN);
            if (btn_max) lv_obj_clear_flag(btn_max, LV_OBJ_FLAG_HIDDEN);
            // In manual mode, encoder rotation directly controls PWM (via on_encoder_rotation callback)
            // No focus navigation happens - all rotation is intercepted for PWM adjustment
        }
    }
};

static ServoState S;

// UI Helpers
static lv_obj_t* make_row(lv_obj_t* p, int w_pct = 90, int h = ROW_H) {
    lv_obj_t* r = lv_obj_create(p);
    lv_obj_set_size(r, LV_PCT(w_pct), h);
    // Opaque background with parent color prevents redraw propagation
    lv_obj_set_style_bg_color(r, lv_color_hex(GUI_COLOR_BG[0]), 0);
    lv_obj_set_style_bg_opa(r, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(r, 0, 0);
    lv_obj_set_style_pad_all(r, 0, 0);
    lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(r, 10, 0);
    // Prevent scroll and event bubbling invalidations
    lv_obj_clear_flag(r, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(r, LV_OBJ_FLAG_EVENT_BUBBLE);  // Events don't trigger parent redraw
    return r;
}

// Create button with focus order (use -1 to skip focus group)
static lv_obj_t* make_btn(lv_obj_t* p, const char* txt, int w, lv_color_t col, lv_event_cb_t cb, int focus_order, void* data = &S) {
    lv_obj_t* b = lv_button_create(p);
    lv_obj_set_size(b, w, BTN_H);
    lv_obj_set_style_bg_color(b, col, 0);

    lv_obj_t* l = lv_label_create(b);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_font(l, FONT_BUTTON_MD, 0);
    lv_obj_center(l);
    if (cb) lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, data);

    // Add to focus builder at specified order position
    if (focus_order >= 0) {
        focus_builder.add(b, focus_order);
    }

    return b;
}

// Callbacks
static void on_timer(lv_timer_t* t) {
    if (!S.running || !S.auto_mode) return;

    // Apply sweep delta to each selected servo individually
    // All servos use the same sweep step (adjustable via encoder)
    int step = g_settings.servo_sweep_step;
    for (int i = 0; i < NUM_SERVOS; i++) {
        if (S.is_servo_selected(i)) {
            S.pwm[i] += S.direction[i] * step;

            // Check bounds and reverse direction for THIS servo
            if (S.pwm[i] >= PWM_MAX) {
                S.pwm[i] = PWM_MAX;
                S.direction[i] = -1;
            } else if (S.pwm[i] <= PWM_MIN) {
                S.pwm[i] = PWM_MIN;
                S.direction[i] = 1;
            }
        }
    }

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
        // Output this servo's individual PWM value
        servo_set_pulse(static_cast<uint8_t>(idx), static_cast<uint16_t>(S.pwm[idx]));
    }
    // Update display to show the new primary servo's value
    S.update_display();
}

// Long-press callback: toggle all servos on/off (works from anywhere on the page)
static void on_long_press_toggle_all() {
    // Toggle all servos - useful shortcut accessible from any focus position
    if (S.all_selected()) {
        S.deselect_all();
        servo_disable_all();
    } else {
        S.select_all();
        // Enable all servos and output their individual PWM values
        for (int i = 0; i < NUM_SERVOS; i++) {
            servo_enable(static_cast<uint8_t>(i), true);
            servo_set_pulse(static_cast<uint8_t>(i), static_cast<uint16_t>(S.pwm[i]));
        }
    }
    S.update_servo_buttons();
    S.update_display();
}

// Double-click callback: in manual mode, switch back to auto mode instead of leaving page
static bool on_double_click() {
    // In manual mode, double-click switches back to auto mode
    if (!S.auto_mode) {
        S.auto_mode = true;
        S.update_ui();
        return true;  // Handled - don't go back
    }
    // In auto mode, let default behavior (go back) happen
    return false;
}

// Encoder rotation callback: adjust sweep speed in auto mode, PWM in manual mode
static bool on_encoder_rotation(int delta) {
    // In auto mode RUNNING: encoder adjusts sweep step size
    if (S.auto_mode && S.running) {
        int increment = g_settings.servo_sweep_step_increment;
        int new_step = g_settings.servo_sweep_step + (delta * increment);
        // Clamp to valid range (1-100)
        if (new_step < 1) new_step = 1;
        if (new_step > 100) new_step = 100;
        g_settings.servo_sweep_step = (uint8_t)new_step;
        // Note: settings_save() is called when STOP is pressed, not on every tick
        return true;  // We handled the rotation
    }

    // In manual mode, encoder rotation adjusts all selected servos by relative delta
    if (!S.auto_mode && !S.running) {
        // Get rotation speed for acceleration
        int speed = input_get_rotation_speed();

        // Get PWM step from primary servo (use as base step)
        int primary = S.get_primary_servo();
        int base_step = (primary >= 0) ? S.get_pwm_step(primary) : DEFAULT_PWM_STEP;

        // Acceleration: faster rotation = bigger steps
        int step = base_step;
        if (speed > 5) {
            step = base_step * 5;  // Fast: 5x step
        } else if (speed > 2) {
            step = base_step * 2;  // Medium: 2x step
        }

        // CW rotation (delta > 0) = increase PWM (slider moves right)
        // Apply relative delta to all selected servos
        S.apply_delta(delta * step);
        S.update_display();
        return true;  // We handled the rotation
    }
    return false;  // Let default focus navigation handle it
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
    bool was_running = S.running;
    S.running = !S.running;
    if (S.timer) S.running ? lv_timer_resume(S.timer) : lv_timer_pause(S.timer);
    // Persist sweep step when stopping (may have been adjusted via encoder)
    if (was_running && !S.running) {
        settings_save();
    }
    S.update_ui();
}

static void on_preset(lv_event_t* e) {
    int preset_value = reinterpret_cast<intptr_t>(lv_event_get_user_data(e));
    // Set all selected servos to this preset value (sync operation)
    S.set_all_selected(preset_value);
    S.update_display();
}

static void on_slider(lv_event_t* e) {
    // Slider is 0-100, map to PWM range from settings
    int slider_val = lv_slider_get_value(lv_event_get_target_obj(e));
    int range = PWM_MAX - PWM_MIN;
    int new_pwm = PWM_MIN + (slider_val * range / 100);

    // Calculate delta from primary servo's current position
    int primary = S.get_primary_servo();
    if (primary >= 0) {
        int delta = new_pwm - S.pwm[primary];
        // Apply relative delta to all selected servos
        S.apply_delta(delta);
    }

    // Update display (label) - slider already has correct value from user interaction
    int display_pwm = S.get_display_pwm();
    bool multi = S.is_multi_select();
    static char pwm_buf[12];
    if (multi) {
        snprintf(pwm_buf, sizeof(pwm_buf), "%d*", display_pwm);
    } else {
        snprintf(pwm_buf, sizeof(pwm_buf), "%d", display_pwm);
    }
    lv_label_set_text_static(S.lbl_pwm, pwm_buf);

    // Output PWM to each selected servo with its individual value
    for (int i = 0; i < NUM_SERVOS; i++) {
        if (S.is_servo_selected(i)) {
            servo_set_pulse(static_cast<uint8_t>(i), static_cast<uint16_t>(S.pwm[i]));
        }
    }
}

// Public API
void page_servo_create(lv_obj_t* parent) {
    S.reset();

    // Register this page in navigation history
    input_push_page(PAGE_SERVO);

    // Initialize focus builder for this page
    focus_builder.init();

    // Register long-press callback for select/deselect all servos
    focus_builder.set_long_press_cb(on_long_press_toggle_all);

    // Register encoder rotation callback for direct PWM control in manual mode
    focus_builder.set_encoder_rotation_cb(on_encoder_rotation);

    // Register double-click callback: in manual mode, switch to auto instead of going back
    focus_builder.set_double_click_cb(on_double_click);

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
    // Touch only - not in encoder focus group to keep navigation simple
    // Long press anywhere = toggle ALL servos
    for (int i = 0; i < NUM_SERVOS; i++) {
        S.btn_servo[i] = lv_button_create(sidebar);
        lv_obj_set_size(S.btn_servo[i], SIDEBAR_W - 8, SERVO_BTN_H);
        lv_obj_set_style_bg_color(S.btn_servo[i], i == 0 ? COL_SERVO_ON : COL_INACTIVE, 0);
        lv_obj_set_style_radius(S.btn_servo[i], 4, 0);

        lv_obj_t* lbl = lv_label_create(S.btn_servo[i]);
        lv_label_set_text_fmt(lbl, "%d", i + 1);
        lv_obj_set_style_text_font(lbl, &arial_14, 0);
        lv_obj_center(lbl);
        // Touch click = toggle this servo
        lv_obj_add_event_cb(S.btn_servo[i], on_servo_toggle, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        // NOTE: Servo buttons are NOT added to focus group - use touch only
    }

    // === CONTENT: Controls ===
    lv_obj_t* content = lv_obj_create(parent);
    lv_obj_remove_style_all(content);
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_height(content, LV_PCT(100));
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    // Use START alignment instead of SPACE_EVENLY to prevent layout recalculation
    // when child widgets update their content
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(content, 8, 0);
    lv_obj_set_style_pad_row(content, 8, 0);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    // IMPORTANT: Opaque background prevents parent redraw when children update
    lv_obj_set_style_bg_color(content, lv_color_hex(GUI_COLOR_BG[0]), 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, 0);

    // Mode row
    lv_obj_t* row_mode = make_row(content);
    S.btn_auto = make_btn(row_mode, tr(STR_SERVO_MODE_AUTO), BTN_MD, COL_ACTIVE, on_auto, FO_AUTO);
    S.btn_manual = make_btn(row_mode, tr(STR_SERVO_MODE_MANUAL), BTN_MD, COL_INACTIVE, on_manual, FO_MANUAL);

    // PWM display row
    lv_obj_t* row_pwm = make_row(content, 90, 30);
    lv_obj_set_style_pad_column(row_pwm, 5, 0);
    lv_obj_t* lbl_title = lv_label_create(row_pwm);
    lv_label_set_text_fmt(lbl_title, "%s (%s):", tr(STR_SERVO_PWM_LABEL), tr(STR_SERVO_US));
    lv_obj_set_style_text_font(lbl_title, FONT_BUTTON_MD, 0);
    lv_obj_set_style_text_color(lbl_title, COL_TEXT, 0);
    S.lbl_pwm = lv_label_create(row_pwm);
    // Use fixed-width text to prevent label size changes causing row reflow
    // Extra space for asterisk (*) in multi-select mode
    lv_obj_set_style_min_width(S.lbl_pwm, 60, 0);  // Enough for "2500*"
    lv_label_set_text(S.lbl_pwm, "1500");
    lv_obj_set_style_text_font(S.lbl_pwm, FONT_MONO_BOLD_LG, 0);
    lv_obj_set_style_text_color(S.lbl_pwm, COL_PRIMARY, 0);

    // Slider
    lv_obj_t* row_slider = lv_obj_create(content);
    lv_obj_set_size(row_slider, LV_PCT(85), ROW_H);
    lv_obj_set_style_bg_color(row_slider, lv_color_hex(GUI_COLOR_BG[0]), 0);
    lv_obj_set_style_bg_opa(row_slider, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(row_slider, 0, 0);
    lv_obj_set_style_pad_hor(row_slider, 10, 0);
    lv_obj_set_style_pad_ver(row_slider, 0, 0);
    lv_obj_set_flex_flow(row_slider, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row_slider, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row_slider, LV_OBJ_FLAG_SCROLLABLE);

    S.slider = lv_slider_create(row_slider);
    lv_obj_set_size(S.slider, LV_PCT(100), 12);  // Thinner track
    lv_slider_set_range(S.slider, 0, 100);
    lv_slider_set_value(S.slider, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(S.slider, lv_color_hex(GUI_COLOR_GRAYS[7]), 0);
    lv_obj_set_style_bg_color(S.slider, lv_color_hex(GUI_COLOR_MONO[1]), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(S.slider, COL_PRIMARY, LV_PART_KNOB);
    // Make knob larger and more visible
    lv_obj_set_style_height(S.slider, 20, LV_PART_KNOB);
    lv_obj_set_style_width(S.slider, 20, LV_PART_KNOB);
    lv_obj_set_style_radius(S.slider, 10, LV_PART_KNOB);  // Circular
    lv_obj_set_style_pad_all(S.slider, 0, LV_PART_KNOB);
    lv_obj_add_event_cb(S.slider, on_slider, LV_EVENT_VALUE_CHANGED, nullptr);
    // Add slider to focus builder
    focus_builder.add(S.slider, FO_SLIDER);

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
    // Add start button to focus builder
    focus_builder.add(S.btn_start, FO_START);

    // Manual preset row (hidden initially)
    S.row_manual = make_row(content);
    lv_obj_add_flag(S.row_manual, LV_OBJ_FLAG_HIDDEN);
    S.btn_min = make_btn(S.row_manual, "Min", BTN_SM, COL_PRIMARY, on_preset, FO_MIN, (void*)(intptr_t)PWM_MIN);
    S.btn_center = make_btn(S.row_manual, "Center", BTN_MD, COL_PRIMARY, on_preset, FO_CENTER, (void*)(intptr_t)PWM_CENTER);
    S.btn_max = make_btn(S.row_manual, "Max", BTN_SM, COL_PRIMARY, on_preset, FO_MAX, (void*)(intptr_t)PWM_MAX);
    // Hide preset buttons initially (auto mode is default) so focus navigation skips them
    lv_obj_add_flag(S.btn_min, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(S.btn_center, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(S.btn_max, LV_OBJ_FLAG_HIDDEN);

    S.update_display();
    S.update_ui();
    S.update_servo_buttons();

    // Enable initially selected servo(s) and set their individual pulses
    for (int i = 0; i < NUM_SERVOS; i++) {
        if (S.is_servo_selected(i)) {
            servo_enable(static_cast<uint8_t>(i), true);
            servo_set_pulse(static_cast<uint8_t>(i), static_cast<uint16_t>(S.pwm[i]));
        }
    }

    // Add footer buttons to focus order
    focus_builder.add(gui_get_btn_home(), FO_BTN_HOME);
    focus_builder.add(gui_get_btn_prev(), FO_BTN_PREV);
    focus_builder.add(gui_get_btn_next(), FO_BTN_NEXT);
    focus_builder.add(gui_get_btn_settings(), FO_BTN_SETTINGS);

    // Finalize focus builder - adds widgets to group in order and activates it
    focus_builder.finalize();
}

void page_servo_destroy() {
    S.running = false;
    servo_disable_all();  // Stop all servo outputs
    if (S.timer) { lv_timer_delete(S.timer); S.timer = nullptr; }
    focus_builder.destroy();
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
    if (S.slider) {
        S.apply_delta(delta);
        S.update_display();
    }
}

void page_servo_toggle_sweep() {
    if (!S.btn_start) return;
    // Simulate clicking the start/stop button
    lv_obj_send_event(S.btn_start, LV_EVENT_CLICKED, nullptr);
}
