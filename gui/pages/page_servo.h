#pragma once
#include "lvgl.h"

void page_servo_create(lv_obj_t* parent);

// For simulator: adjust PWM value by delta (e.g., +10 or -10)
void page_servo_adjust_pwm(int delta);
