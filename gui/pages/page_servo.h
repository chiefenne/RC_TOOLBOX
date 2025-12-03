#pragma once
#include "lvgl.h"

void page_servo_create(lv_obj_t* parent);

// For simulator: adjust PWM value by delta (e.g., +10 or -10)
void page_servo_adjust_pwm(int delta);

// Check if servo auto-sweep is running (blocks navigation)
bool page_servo_is_running();

// Stop servo and clean up timer (call before leaving page)
void page_servo_stop();
