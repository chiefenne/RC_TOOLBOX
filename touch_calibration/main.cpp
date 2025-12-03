// touch_calibration/main.cpp - Touch calibration for XPT2046
// Run this first to get calibration values for your specific display

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void touch_calibrate();

void setup()
{
    Serial.begin(115200);
    delay(100);

    Serial.println("\n=== RC TOOLBOX - Touch Calibration ===");

    tft.init();
    tft.setRotation(1);  // Landscape - same as main app

    touch_calibrate();

    tft.fillScreen(TFT_BLACK);
    tft.drawCentreString("Touch screen to test!", tft.width() / 2, tft.height() / 2, 2);
}

void loop()
{
    uint16_t x = 0, y = 0;
    bool pressed = tft.getTouch(&x, &y);

    if (pressed)
    {
        tft.fillCircle(x, y, 3, TFT_WHITE);
        Serial.printf("Touch: x=%d, y=%d\n", x, y);
    }
}

void touch_calibrate()
{
    uint16_t calData[5];

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");
    tft.setTextFont(1);
    tft.println();

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    Serial.println();
    Serial.println("// Copy this calibration code to src/main.cpp setup():");
    Serial.print("uint16_t calData[5] = { ");
    for (uint8_t i = 0; i < 5; i++)
    {
        Serial.print(calData[i]);
        if (i < 4) Serial.print(", ");
    }
    Serial.println(" };");
    Serial.println("tft.setTouch(calData);");
    Serial.println();

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");
    tft.println("Check Serial Monitor for values.");

    delay(3000);
}
