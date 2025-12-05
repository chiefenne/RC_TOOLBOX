// gui/pages/page_servo.hpp
#pragma once

#include "page_base.hpp"

class ServoPage : public PageBase {
public:
    void create(lv_obj_t* parent) override;
    void destroy() override;
    void on_hide() override;
    void adjust_encoder(int delta) override;
    bool is_background_task_running() const override;

    // Optional: force PWM from elsewhere (e.g. calibration routine)
    void set_pwm(int value_us);
};

// One global instance — standard pattern in embedded LVGL apps
extern ServoPage servo_page;
