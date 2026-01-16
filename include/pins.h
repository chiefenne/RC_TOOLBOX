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
// Rotary Encoder (EC11 with push button)
// =============================================================================
constexpr int PIN_ENC_CLK = 35;  // Encoder output A (CLK)
constexpr int PIN_ENC_DT  = 36;  // Encoder output B (DT)
constexpr int PIN_ENC_SW  = 37;  // Encoder push button (active LOW)

// =============================================================================
// I2C Bus (for future I2C expander, sensors, etc.)
// =============================================================================
constexpr int PIN_I2C_SDA = 47;
constexpr int PIN_I2C_SCL = 39;  // Keep NeoPixel on GPIO48

// =============================================================================
// NFC (PN532) - HW-147C breakout (I2C mode, DIP: 1/0)
// =============================================================================
// Uses I2C bus: GPIO47 (SDA), GPIO39 (SCL)
constexpr int PIN_PN532_IRQ = 41; // Data ready interrupt
constexpr int PIN_PN532_RST = 40; // Hardware reset

// =============================================================================
// GPIO Summary
// =============================================================================
// Used GPIOs:
//   1       - free GPIO
//   2       - free GPIO
//   3       - free GPIO
//   4       - free GPIO (SD card chip select not used)
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
//  35       - Encoder CLK (A)
//  36       - Encoder DT (B)
//  37       - Encoder SW (button)
//  40       - PN532_RST
//  41       - PN532_IRQ
//  47       - I2C SDA (PN532)
//  39       - I2C SCL (PN532)
//  48       - NeoPixel
//  38       - CS_BREAK (SPI breakout)
//  42       - IRQ_BREAK (SPI breakout)

// Available GPIOs:
//  45, 46  - Available (strapping pins, use with care)
