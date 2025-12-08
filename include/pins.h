// pins.h - GPIO assignments for RC TOOLBOX (ESP32-S3 DevKitC-1)
#pragma once

// =============================================================================
// TFT Display (ILI9341) - defined in platformio.ini
// =============================================================================
// TFT_MOSI  11   (SPI data out)
// TFT_MISO  13   (SPI data in)
// TFT_SCLK  12   (SPI clock)
// TFT_CS    10   (TFT chip select)
// TFT_DC     8   (Data/Command)
// TFT_RST    9   (Reset)

// =============================================================================
// Touch Controller (XPT2046) - defined in platformio.ini
// =============================================================================
// TOUCH_CS  14   (Touch chip select)
// TOUCH_IRQ  7   (Touch interrupt)

// =============================================================================
// SD Card - defined in platformio.ini
// =============================================================================
// SD_CS      4   (SD card chip select, directly accessible for files)

// =============================================================================
// NeoPixel LED - defined in platformio.ini
// =============================================================================
// NEOPIXEL_PIN  48   (Built-in RGB LED on DevKitC-1)

// =============================================================================
// TFT Backlight (directly accessible from this file)
// =============================================================================
constexpr int PIN_TFT_BACKLIGHT = 5;

// =============================================================================
// Servo PWM Outputs (directly accessible from this file)
// =============================================================================
constexpr int NUM_SERVO_PINS = 6;
constexpr int PIN_SERVO[] = {6, 15, 16, 17, 18, 21};

// LEDC channels for servo PWM (ESP32 has 8 channels, 0-7)
constexpr int SERVO_LEDC_CHANNEL[] = {0, 1, 2, 3, 4, 5};

// =============================================================================
// Analog Inputs (ADC) - for battery voltage, etc.
// =============================================================================
constexpr int NUM_ADC_PINS = 3;
constexpr int PIN_ADC[] = {1, 2, 3};  // ADC1 channels

// Voltage divider factor (adjust based on your resistor values)
// Example: 10k + 10k divider = factor 2.0
constexpr float ADC_VOLTAGE_DIVIDER = 2.0f;

// =============================================================================
// I2C Bus (for future I2C expander, sensors, etc.)
// =============================================================================
constexpr int PIN_I2C_SDA = 47;
constexpr int PIN_I2C_SCL = 48;  // Note: Shared with NeoPixel on DevKitC-1!
// Alternative I2C pins if NeoPixel conflicts:
// constexpr int PIN_I2C_SDA = 35;
// constexpr int PIN_I2C_SCL = 36;

// =============================================================================
// GPIO Summary
// =============================================================================
// Used GPIOs:
//   4       - SD_CS
//   5       - TFT Backlight PWM
//   6       - Servo 1
//   7       - TOUCH_IRQ
//   8       - TFT_DC
//   9       - TFT_RST
//  10       - TFT_CS
//  11       - TFT_MOSI
//  12       - TFT_SCLK
//  13       - TFT_MISO
//  14       - TOUCH_CS
//  15       - Servo 2
//  16       - Servo 3
//  17       - Servo 4
//  18       - Servo 5
//  21       - Servo 6
//  47       - I2C SDA (future)
//  48       - NeoPixel / I2C SCL (conflict, choose one)
//
// Available GPIOs:
//   1, 2, 3 - ADC inputs (battery voltage)
//  35, 36, 37, 38, 39, 40, 41, 42 - General purpose
//  45, 46  - Available (strapping pins, use with care)
