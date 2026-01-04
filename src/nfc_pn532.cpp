#include "nfc_pn532.h"

#if defined(ESP_PLATFORM) || defined(ARDUINO)

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#include "pins.h"

namespace {
Adafruit_PN532 nfc(PIN_PN532_SS, &SPI);

bool nfc_ready = false;
uint8_t last_uid[10] = {0};
uint8_t last_uid_len = 0;

volatile bool pn532_irq_fired = false;

void IRAM_ATTR pn532_irq_isr() {
  pn532_irq_fired = true;
}

void reset_pn532_hw() {
  pinMode(PIN_PN532_RST, OUTPUT);
  digitalWrite(PIN_PN532_RST, HIGH);
  delay(5);
  digitalWrite(PIN_PN532_RST, LOW);
  delay(5);
  digitalWrite(PIN_PN532_RST, HIGH);
  delay(50);
}

bool uid_equals_last(const uint8_t *uid, uint8_t uid_len) {
  if (uid_len != last_uid_len) {
    return false;
  }
  for (uint8_t i = 0; i < uid_len; i++) {
    if (uid[i] != last_uid[i]) {
      return false;
    }
  }
  return true;
}

void store_last_uid(const uint8_t *uid, uint8_t uid_len) {
  last_uid_len = uid_len;
  for (uint8_t i = 0; i < sizeof(last_uid); i++) {
    last_uid[i] = (i < uid_len) ? uid[i] : 0;
  }
}

void print_uid(const uint8_t *uid, uint8_t uid_len) {
  Serial.print("[NFC] UID (");
  Serial.print(uid_len);
  Serial.print(" bytes): ");
  for (uint8_t i = 0; i < uid_len; i++) {
    if (uid[i] < 0x10) {
      Serial.print('0');
    }
    Serial.print(uid[i], HEX);
    if (i + 1 < uid_len) {
      Serial.print(':');
    }
  }
  Serial.println();
}

bool start_detection() {
  // Arms the PN532 to detect a tag and signal via IRQ.
  // IRQ goes low when data is ready.
  pn532_irq_fired = false;
  bool ok = nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
  if (!ok) {
    Serial.println("[NFC] ERROR: startPassiveTargetIDDetection failed");
    return false;
  }
  return true;
}
} // namespace

void nfc_pn532_init() {
  Serial.println("[NFC] Initializing PN532 (SPI)...");

  pinMode(PIN_PN532_SS, OUTPUT);
  digitalWrite(PIN_PN532_SS, HIGH);

  pinMode(PIN_PN532_IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_PN532_IRQ), pn532_irq_isr, FALLING);

  reset_pn532_hw();

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("[NFC] ERROR: PN532 not found (check wiring/DIP/SPI bus)");
    nfc_ready = false;
    return;
  }

  Serial.print("[NFC] Found PN5");
  Serial.print((versiondata >> 24) & 0xFF, HEX);
  Serial.print(" FW ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  if (!nfc.SAMConfig()) {
    Serial.println("[NFC] ERROR: SAMConfig failed");
    nfc_ready = false;
    return;
  }

  // With IRQ-driven detection we don't want the PN532 to block waiting for a card.
  // We'll arm detection once and wait for IRQ.
  nfc.setPassiveActivationRetries(0xFF);

  if (!start_detection()) {
    nfc_ready = false;
    return;
  }

  Serial.println("[NFC] Ready (IRQ mode). Present an ISO14443A tag...");
  nfc_ready = true;
}

void nfc_pn532_poll() {
  if (!nfc_ready) {
    return;
  }

  if (!pn532_irq_fired) {
    return;
  }

  // Clear flag early to avoid losing back-to-back events.
  pn532_irq_fired = false;

  uint8_t uid[10];
  uint8_t uidLength = 0;

  bool success = nfc.readDetectedPassiveTargetID(uid, &uidLength);
  if (success) {
    if (!uid_equals_last(uid, uidLength)) {
      print_uid(uid, uidLength);
      store_last_uid(uid, uidLength);
    }
  }

  // Re-arm detection for the next card.
  start_detection();
}

#endif
