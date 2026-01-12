// Minimal PN532 I2C test for Seeed XIAO ESP32-S3
// Connect: VCC->3.3V, GND->GND, SDA->GPIO5, SCL->GPIO6

#include <Wire.h>

// XIAO ESP32-S3 default I2C pins
#define SDA_PIN 5
#define SCL_PIN 6

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n=== PN532 Standalone Test ===");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  // Scan I2C
  Serial.println("Scanning I2C...");
  uint8_t foundAddr = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found: 0x");
      Serial.println(addr, HEX);
      foundAddr = addr;
    }
  }

  if (!foundAddr) {
    Serial.println("No I2C device found!");
    return;
  }

  // Try to wake up and get firmware version
  Serial.println("\nSending wakeup...");
  for (int i = 0; i < 5; i++) {
    Wire.beginTransmission(foundAddr);
    Wire.write((uint8_t)0x00);
    Wire.endTransmission();
    delay(50);
  }
  delay(100);

  // Read status
  Serial.print("Status: ");
  Wire.requestFrom(foundAddr, (uint8_t)8);
  while (Wire.available()) {
    uint8_t b = Wire.read();
    if (b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
  Serial.println();

  // Send GetFirmwareVersion command
  Serial.println("\nSending GetFirmwareVersion...");
  uint8_t cmd[] = {0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00};
  Wire.beginTransmission(foundAddr);
  Wire.write(cmd, sizeof(cmd));
  int err = Wire.endTransmission();
  Serial.print("Write result: ");
  Serial.println(err);

  // Wait and poll for response
  Serial.println("Polling for ready...");
  for (int i = 0; i < 20; i++) {
    delay(50);
    Wire.requestFrom(foundAddr, (uint8_t)1);
    if (Wire.available()) {
      uint8_t status = Wire.read();
      Serial.print("Status[");
      Serial.print(i);
      Serial.print("]: 0x");
      Serial.println(status, HEX);
      if (status == 0x01) {
        Serial.println("READY!");
        // Read response
        Wire.requestFrom(foundAddr, (uint8_t)16);
        Serial.print("Response: ");
        while (Wire.available()) {
          uint8_t b = Wire.read();
          if (b < 0x10) Serial.print('0');
          Serial.print(b, HEX);
          Serial.print(' ');
        }
        Serial.println();
        return;
      }
    }
  }
  Serial.println("Timeout - no response");
}

void loop() {
  delay(5000);
  Serial.println("Press reset to retry...");
}
