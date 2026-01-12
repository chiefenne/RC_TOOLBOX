// nfc_pn532.cpp - NFC driver using Adafruit PN532 library
// Hardware: Elechouse PN532 V3 @ I2C address 0x24
// Connections: SDA=GPIO47, SCL=GPIO39, IRQ=GPIO41, RST=GPIO40

#include "nfc_pn532.h"

#if defined(ESP_PLATFORM) || defined(ARDUINO)

#include <Wire.h>
#include <Adafruit_PN532.h>
#include "pins.h"
#include "gui/serial_log.h"

// Create PN532 instance with I2C (IRQ and RESET pins)
Adafruit_PN532 nfc(PIN_PN532_IRQ, PIN_PN532_RST);

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
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

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
