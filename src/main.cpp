// src/main.cpp - ESP32-S3 entry point for RC TOOLBOX
// ILI9341 TFT + XPT2046 Touch + LVGL 9.4.0

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <Adafruit_NeoPixel.h>
#include "gui/gui.h"
#include "gui/input.h"
#include "gui/serial_log.h"
#include "servo_driver.h"
#include "nfc_pn532.h"

// TFT instance (configured via build_flags in platformio.ini)
TFT_eSPI tft = TFT_eSPI();

// NeoPixel RGB LED (built-in on ESP32-S3-DevKitC-1)
Adafruit_NeoPixel pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// LVGL draw buffers - larger buffer = fewer SPI transactions = smoother rendering
// Double buffering: LVGL renders to buf2 while buf1 is being transmitted via SPI
// 40 lines = good balance between memory usage and performance
static lv_color_t buf1[SCREEN_WIDTH * 40];
static lv_color_t buf2[SCREEN_WIDTH * 40];
static lv_display_t *display;
static lv_indev_t *touch_indev;

// Forward declarations
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void my_touch_read(lv_indev_t *drv, lv_indev_data_t *data);

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000) { }  // Wait up to 2s for serial
    Serial.println("\n\n*** BOOT ***");
    Serial.flush();

    delay(100);
    log_println("=== RC TOOLBOX BOOT ===");

    // Initialize TFT
    log_println("[1] Starting TFT...");
    tft.init();
    tft.setRotation(1);  // Landscape mode
    tft.fillScreen(TFT_BLACK);

    // Touch calibration - run touch-calibration environment to get these values
    // Then update with your specific calibration data:
    // GEORG'S VALUES:
    // uint16_t calData[5] = {403, 3387, 375, 3250, 7};
    uint16_t calData[5] = {300, 3600, 300, 3600, 1};  // Default - calibrate for your display!
    tft.setTouch(calData);

    log_println("[1] TFT complete");

    // Initialize LVGL
    log_println("[2] Starting LVGL...");
    lv_init();

    // Create display (LVGL 9.x API)
    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(display, my_disp_flush);
    lv_display_set_buffers(display, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_default(display);

    // Set refresh period to 20ms (default is 33ms) for smoother updates
    lv_timer_set_period(lv_display_get_refr_timer(display), 20);

    // Create touch input device
    touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, my_touch_read);

    // Initialize encoder input device (EC11 rotary encoder)
    input_init();

    log_println("[2] LVGL complete");

    // Initialize GUI (shared code with simulator)
    log_println("[3] Starting GUI...");
    gui_init();

    // Initialize servo PWM driver
    log_println("[4] Initializing servo driver...");
    servo_driver_init();

    // Initialize NFC (PN532)
    nfc_pn532_init();

    // NeoPixel ready indicator (solid green)
    pixel.begin();
    pixel.setBrightness(30);
    pixel.setPixelColor(0, pixel.Color(0, 255, 0));  // Green
    pixel.show();

    log_println("[4] Servo driver complete - RC TOOLBOX ready!");
}

void loop()
{
    lv_tick_inc(5);
    lv_timer_handler();
    input_poll();  // Poll encoder hardware
    nfc_pn532_poll();
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
