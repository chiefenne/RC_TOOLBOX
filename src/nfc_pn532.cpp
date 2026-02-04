// nfc_pn532.cpp - NFC driver using Adafruit PN532 library
// Hardware: Elechouse PN532 V3 @ I2C address 0x24
// Connections: SDA=GPIO47, SCL=GPIO39, IRQ=GPIO41, RST=GPIO40

#include "nfc_pn532.h"

#if defined(ESP_PLATFORM) || defined(ARDUINO)

#include <Wire.h>
#include <Adafruit_PN532.h>
#include "pins.h"
#include "gui/serial_log.h"

#ifndef PN532_SWAP_I2C
#define PN532_SWAP_I2C 0
#endif

// ============================================================================
// Debug Flags - Set to 1 to enable specific debug features
// ============================================================================
#define I2C_DEBUG 1     // Enable I2C bus scanner at startup

// Create PN532 instance with I2C (IRQ and RESET pins)
Adafruit_PN532 nfc(PIN_PN532_IRQ, PIN_PN532_RST);

// ============================================================================
// I2C Scanner - Scans all addresses to find connected devices
// ============================================================================
#if I2C_DEBUG
void i2c_scan(int sda_pin, int scl_pin) {
  log_println("[I2C] Scanning I2C bus...");
  serial_printf("[I2C] SDA=GPIO%d, SCL=GPIO%d\n", sda_pin, scl_pin);

  int devices_found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      // Device found
      devices_found++;
      const char* device_name = "";

      // Known device identification
      if (addr == 0x24) {
        device_name = " <- PN532 (Elechouse V3)";
      } else if (addr == 0x28) {
        device_name = " <- PN532 (HW-147C/alternate)";
      } else if (addr == 0x48) {
        device_name = " <- PN532 (alternate address)";
      }

      serial_printf("[I2C] Found device at 0x%02X%s\n", addr, device_name);
    } else if (error == 4) {
      serial_printf("[I2C] Unknown error at 0x%02X\n", addr);
    }
  }

  if (devices_found == 0) {
    log_println("[I2C] ERROR: No devices found on I2C bus!");
    log_println("[I2C] Check: SDA/SCL wiring, pull-up resistors, power");
  } else {
    serial_printf("[I2C] Scan complete: %d device(s) found\n", devices_found);
  }
}
#endif

namespace {
  bool nfc_ready = false;
  uint8_t last_uid[10] = {0};
  uint8_t last_uid_len = 0;
  bool tag_present = false;  // Track if tag is currently present

  bool uid_equals_last(const uint8_t *uid, uint8_t uid_len) {
    if (uid_len != last_uid_len) return false;
    for (uint8_t i = 0; i < uid_len; i++) {
      if (uid[i] != last_uid[i]) return false;
    }
    return true;
  }

  void store_last_uid(const uint8_t *uid, uint8_t uid_len) {
    last_uid_len = uid_len;
    for (size_t i = 0; i < sizeof(last_uid); i++) {
      last_uid[i] = (i < uid_len) ? uid[i] : 0;
    }
  }

  void print_uid(const uint8_t *uid, uint8_t uid_len) {
    char uid_msg[128];
    char temp[16];
    snprintf(uid_msg, sizeof(uid_msg), "[NFC] TAG DETECTED (%d bytes): ", uid_len);
    for (uint8_t i = 0; i < uid_len; i++) {
      snprintf(temp, sizeof(temp), "%02X", uid[i]);
      strcat(uid_msg, temp);
      if (i + 1 < uid_len) strcat(uid_msg, ":");
    }
    log_println(uid_msg);
  }
}

void nfc_pn532_init() {
  log_println("[NFC] Initializing Adafruit PN532 library...");

  // Initialize I2C
  const int pn532_sda = PN532_SWAP_I2C ? PIN_I2C_SCL : PIN_I2C_SDA;
  const int pn532_scl = PN532_SWAP_I2C ? PIN_I2C_SDA : PIN_I2C_SCL;
  pinMode(pn532_sda, INPUT_PULLUP);
  pinMode(pn532_scl, INPUT_PULLUP);
  Wire.begin(pn532_sda, pn532_scl);
  Wire.setClock(100000);

#if I2C_DEBUG
  // Scan I2C bus to verify PN532 is visible
  i2c_scan(pn532_sda, pn532_scl);
#endif

  // Initialize PN532
  nfc.begin();

  // Check firmware version
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    log_println("[NFC] ERROR: No PN532 found - check wiring!");
    nfc_ready = false;
    return;
  }

  // Print version info
  char version_msg[64];
  snprintf(version_msg, sizeof(version_msg),
           "[NFC] Found PN5%02X, Firmware v%d.%d",
           (versiondata >> 24) & 0xFF,
           (versiondata >> 16) & 0xFF,
           (versiondata >> 8) & 0xFF);
  log_println(version_msg);

  // Configure board to read RFID tags
  nfc.SAMConfig();

  log_println("[NFC] Ready - waiting for ISO14443A tags...");
  nfc_ready = true;
}

void nfc_pn532_poll() {
  if (!nfc_ready) return;

  // Poll every 250ms
  static uint32_t last_poll = 0;
  uint32_t now = millis();
  if (now - last_poll < 250) return;
  last_poll = now;

  // Try to read a tag
  uint8_t uid[7];
  uint8_t uidLength;

  // Use passive mode with 100ms timeout
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100)) {
    // Tag detected
    if (!tag_present) {
      // Tag just appeared (was absent, now present)
      print_uid(uid, uidLength);
      store_last_uid(uid, uidLength);
      tag_present = true;
    } else if (!uid_equals_last(uid, uidLength)) {
      // Different tag detected while previous was present
      print_uid(uid, uidLength);
      store_last_uid(uid, uidLength);
    }
    // Same tag still present - don't spam
  } else {
    // No tag detected
    if (tag_present) {
      log_println("[NFC] Tag removed");
      tag_present = false;
    }
  }
}

#endif
