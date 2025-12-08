// src/main.cpp - ESP32-S3 entry point for RC TOOLBOX
// ILI9341 TFT + XPT2046 Touch + LVGL 9.4.0

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <Adafruit_NeoPixel.h>
#include "gui/gui.h"
#include "servo_driver.h"

// TFT instance (configured via build_flags in platformio.ini)
TFT_eSPI tft = TFT_eSPI();

// NeoPixel RGB LED (built-in on ESP32-S3-DevKitC-1)
Adafruit_NeoPixel pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// LVGL draw buffer
static lv_color_t buf1[SCREEN_WIDTH * 10];
static lv_display_t *display;
static lv_indev_t *touch_indev;

// Forward declarations
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void my_touch_read(lv_indev_t *drv, lv_indev_data_t *data);
void neopixel_intro();

// --- NeoPixel Light Show Intro ---
void neopixel_intro()
{
    pixel.begin();
    pixel.setBrightness(50);  // Not too bright
    pixel.show();

    // Color sequence: Rainbow fade
    const uint32_t colors[] = {
        pixel.Color(255, 0, 0),     // Red
        pixel.Color(255, 127, 0),   // Orange
        pixel.Color(255, 255, 0),   // Yellow
        pixel.Color(0, 255, 0),     // Green
        pixel.Color(0, 255, 255),   // Cyan
        pixel.Color(0, 0, 255),     // Blue
        pixel.Color(127, 0, 255),   // Purple
        pixel.Color(255, 0, 127),   // Pink
    };
    const int numColors = sizeof(colors) / sizeof(colors[0]);

    // Phase 1: Fast rainbow cycle (2 rounds)
    for (int round = 0; round < 2; round++) {
        for (int i = 0; i < numColors; i++) {
            pixel.setPixelColor(0, colors[i]);
            pixel.show();
            delay(80);
        }
    }

    // Phase 2: Breathing effect in primary color (blue)
    for (int cycle = 0; cycle < 3; cycle++) {
        // Fade in
        for (int b = 0; b <= 100; b += 5) {
            pixel.setBrightness(b);
            pixel.setPixelColor(0, pixel.Color(0, 100, 255));
            pixel.show();
            delay(15);
        }
        // Fade out
        for (int b = 100; b >= 0; b -= 5) {
            pixel.setBrightness(b);
            pixel.setPixelColor(0, pixel.Color(0, 100, 255));
            pixel.show();
            delay(15);
        }
    }

    // Phase 3: Quick flash finale (white)
    for (int i = 0; i < 3; i++) {
        pixel.setBrightness(100);
        pixel.setPixelColor(0, pixel.Color(255, 255, 255));
        pixel.show();
        delay(50);
        pixel.setPixelColor(0, 0);
        pixel.show();
        delay(50);
    }

    // End: Start with blue (will alternate with orange in loop)
    pixel.setBrightness(30);
    pixel.setPixelColor(0, pixel.Color(0, 100, 255));  // Blue
    pixel.show();
}

// NeoPixel ready indicator mode
enum NeoPixelMode { NEOPIXEL_GREEN, NEOPIXEL_BLUE_ORANGE };
static NeoPixelMode neopixel_mode = NEOPIXEL_GREEN;

// NeoPixel ready blink state
static uint32_t neopixel_last_toggle = 0;
static bool neopixel_is_blue = true;
static bool neopixel_initialized = false;

void neopixel_set_mode(NeoPixelMode mode)
{
    neopixel_mode = mode;
    neopixel_initialized = false;  // Force refresh
}

void neopixel_ready_update()
{
    if (neopixel_mode == NEOPIXEL_GREEN) {
        // Solid green - only set once
        if (!neopixel_initialized) {
            pixel.setBrightness(30);
            pixel.setPixelColor(0, pixel.Color(0, 255, 0));  // Green
            pixel.show();
            neopixel_initialized = true;
        }
    } else {
        // Blue/orange blink - toggle every 500ms
        if (millis() - neopixel_last_toggle >= 500) {
            neopixel_last_toggle = millis();
            neopixel_is_blue = !neopixel_is_blue;
            pixel.setBrightness(30);
            if (neopixel_is_blue) {
                pixel.setPixelColor(0, pixel.Color(0, 100, 255));   // Blue
            } else {
                pixel.setPixelColor(0, pixel.Color(255, 100, 0));   // Orange
            }
            pixel.show();
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(500);  // Give serial time to connect
    Serial.println("\n=== RC TOOLBOX BOOT ===");
    Serial.flush();

    // NeoPixel intro light show
    Serial.println("[1] Starting NeoPixel...");
    Serial.flush();
    neopixel_intro();
    Serial.println("[1] NeoPixel complete");
    Serial.flush();

    // Initialize TFT
    Serial.println("[2] Starting TFT...");
    Serial.flush();
    tft.init();
    tft.setRotation(1);  // Landscape mode
    tft.fillScreen(TFT_BLACK);

    // Touch calibration - run touch-calibration environment to get these values
    // Then update with your specific calibration data:
    uint16_t calData[5] = {300, 3600, 300, 3600, 1};  // Default - calibrate for your display!
    tft.setTouch(calData);

    Serial.println("[2] TFT complete");
    Serial.flush();

    // Initialize LVGL
    Serial.println("[3] Starting LVGL...");
    Serial.flush();
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

    Serial.println("[3] LVGL complete");
    Serial.flush();

    // Initialize GUI (shared code with simulator)
    Serial.println("[4] Starting GUI...");
    Serial.flush();
    gui_init();

    // Initialize servo PWM driver
    Serial.println("[5] Initializing servo driver...");
    Serial.flush();
    servo_driver_init();

    Serial.println("[5] Servo driver complete - RC TOOLBOX ready!");
    Serial.flush();
}

void loop()
{
    lv_tick_inc(5);
    lv_timer_handler();
    neopixel_ready_update();  // Ready indicator (green or blue/orange)
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
