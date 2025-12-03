// src/main.cpp - ESP32-S3 entry point for RC TOOLBOX
// ILI9341 TFT + XPT2046 Touch + LVGL 9.4.0

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include "gui/gui.h"

// TFT instance (configured via build_flags in platformio.ini)
TFT_eSPI tft = TFT_eSPI();

// LVGL draw buffer
static lv_color_t buf1[SCREEN_WIDTH * 10];
static lv_display_t *display;
static lv_indev_t *touch_indev;

// Forward declarations
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void my_touch_read(lv_indev_t *drv, lv_indev_data_t *data);

void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== RC TOOLBOX ===");

    // Initialize TFT
    tft.init();
    tft.setRotation(1);  // Landscape mode
    tft.fillScreen(TFT_BLACK);

    // Touch calibration - run touch-calibration environment to get these values
    // Then update with your specific calibration data:
    uint16_t calData[5] = {300, 3600, 300, 3600, 1};  // Default - calibrate for your display!
    tft.setTouch(calData);

    Serial.println("TFT initialized");

    // Initialize LVGL
    lv_init();

    // Create display (LVGL 9.x API)
    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(display, my_disp_flush);
    lv_display_set_buffers(display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_default(display);

    // Create touch input device
    touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, my_touch_read);

    Serial.println("LVGL initialized");

    // Initialize GUI (shared code with simulator)
    gui_init();

    Serial.println("GUI initialized - RC TOOLBOX ready!");
}

void loop()
{
    lv_tick_inc(5);
    lv_timer_handler();
    delay(5);
}

// --- LVGL Display Flush Callback ---
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();

    lv_display_flush_ready(disp);
}

// --- LVGL Touch Input Callback ---
void my_touch_read(lv_indev_t *drv, lv_indev_data_t *data)
{
    uint16_t x, y;
    bool touched = tft.getTouch(&x, &y);

    if (touched)
    {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
