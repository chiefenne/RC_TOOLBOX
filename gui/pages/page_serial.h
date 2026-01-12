#pragma once
#include "lvgl.h"

void page_serial_create(lv_obj_t* parent);
void page_serial_destroy();

// Add a debug message to the serial monitor display
void page_serial_add_message(const char* msg);

// Clear all messages
void page_serial_clear();
