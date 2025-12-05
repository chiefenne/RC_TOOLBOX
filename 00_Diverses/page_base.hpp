// gui/pages/page_base.hpp
#pragma once

#include "lvgl.h"

class PageBase {
public:
    virtual ~PageBase() = default;

    virtual void create(lv_obj_t* parent) = 0;
    virtual void destroy() {}
    virtual void on_show() {}
    virtual void on_hide() {}

    // Optional: rotary encoder / keyboard navigation
    virtual void adjust_encoder(int delta) {}

    // Optional: used by main loop to know if background task (sweep, logging, etc.) is active
    virtual bool is_background_task_running() const { return false; }
};
