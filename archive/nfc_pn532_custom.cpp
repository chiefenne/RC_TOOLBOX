// nfc_pn532.cpp - PN532 NFC driver using I2C
// Based on Elechouse PN532_I2C library with Adafruit IRQ pin usage
// GitHub: https://github.com/elechouse/PN532/tree/PN532_HSU/PN532_I2C
//
// Key Protocol Details:
// - Uses IRQ pin (GPIO41) to detect when data is ready (more reliable than I2C polling)
// - IRQ goes LOW when PN532 has response data available
// - Reads include STATUS/RDY byte first (bit 0: 1=ready, 0=busy)
// - Elechouse module V3 uses I2C address 0x24 (0x48 in 8-bit notation)

#include "nfc_pn532.h"

#if defined(ESP_PLATFORM) || defined(ARDUINO)

#include <Arduino.h>
#include <Wire.h>
#include "pins.h"
#include "gui/serial_log.h"

// PN532 I2C address - standard is 0x24 (Elechouse V3, Adafruit)
// HW-147C uses 0x28 - uncomment if using that board
// #define PN532_I2C_ADDR 0x28
#define PN532_I2C_ADDR 0x24 // Elechouse PN532 NFC RFID Module V3

// PN532 frame constants
#define PN532_PREAMBLE    0x00
#define PN532_STARTCODE1  0x00
#define PN532_STARTCODE2  0xFF
#define PN532_POSTAMBLE   0x00
#define PN532_TFI_HOST2PN 0xD4  // Host to PN532
#define PN532_TFI_PN2HOST 0xD5  // PN532 to Host

// PN532 commands
#define PN532_CMD_GETFIRMWAREVERSION  0x02
#define PN532_CMD_SAMCONFIGURATION    0x14
#define PN532_CMD_INLISTPASSIVETARGET 0x4A

// ACK frame
static const uint8_t PN532_ACK[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

namespace {

bool nfc_ready = false;
uint8_t last_uid[10] = {0};
uint8_t last_uid_len = 0;
bool debug_enabled = true;

// Write raw data to PN532
bool writeRawData(const uint8_t* data, size_t len) {
  Wire.beginTransmission(PN532_I2C_ADDR);
  Wire.write(data, len);
  return Wire.endTransmission() == 0;
}

// Read raw data from PN532 with retries
bool readRawData(uint8_t* buffer, size_t len, int retries = 6, int retryDelayMs = 10) {
  for (int i = 0; i < retries; i++) {
    size_t received = Wire.requestFrom((uint8_t)PN532_I2C_ADDR, (uint8_t)len);
    if (received == len) {
      for (size_t j = 0; j < len; j++) {
        buffer[j] = Wire.read();
      }
      return true;
    }
    // Drain any partial data
    while (Wire.available()) Wire.read();
    delay(retryDelayMs);
  }
  return false;
}

// Read single status byte from PN532
// Based on Elechouse PN532_I2C library polling method
// The first byte indicates status: bit 0 = 1 means ready
bool readStatusByte(uint8_t &status) {
  // Request exactly 1 byte
  uint8_t bytesReceived = Wire.requestFrom((uint8_t)PN532_I2C_ADDR, (uint8_t)1);
  if (bytesReceived != 1) {
    return false;
  }
  if (!Wire.available()) {
    return false;
  }
  status = Wire.read();
  return true;
}

// Wait for PN532 to be ready - using IRQ pin for reliability
// IRQ goes LOW when PN532 has data ready
// Much more reliable than I2C polling which fails on ESP32
bool waitReady(int maxRetries = 100, int retryDelayMs = 10) {
  // Use IRQ pin if available (more reliable than I2C status polling)
  if (debug_enabled) {
    char debug_msg[64];
    snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: waitReady start, IRQ=%d", digitalRead(PIN_PN532_IRQ));
    log_println(debug_msg);
  }

  for (int i = 0; i < maxRetries; i++) {
    // IRQ pin is LOW when ready
    int irq_state = digitalRead(PIN_PN532_IRQ);
    if (irq_state == LOW) {
      if (debug_enabled) {
        char debug_msg[64];
        snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: IRQ went LOW at retry %d", i);
        log_println(debug_msg);
      }
      return true;
    }

    // Log every 10th retry to avoid spam
    if (debug_enabled && (i % 10 == 0)) {
      char debug_msg[64];
      snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: waitReady retry %d, IRQ still %d", i, irq_state);
      log_println(debug_msg);
    }

    delay(retryDelayMs);
  }

  if (debug_enabled) {
    char debug_msg[64];
    snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: waitReady TIMEOUT after %d retries, IRQ=%d", maxRetries, digitalRead(PIN_PN532_IRQ));
    log_println(debug_msg);
  }

  return false;
}

// Write a command to PN532
bool writeCommand(uint8_t cmd, const uint8_t* params = nullptr, uint8_t paramLen = 0) {
  // Build frame: PREAMBLE, START1, START2, LEN, LCS, TFI, CMD, [params], DCS, POSTAMBLE
  uint8_t dataLen = 2 + paramLen;  // TFI + CMD + params
  uint8_t lcs = (uint8_t)(0x100 - dataLen);  // Length checksum

  // Calculate data checksum
  uint8_t sum = PN532_TFI_HOST2PN + cmd;
  for (uint8_t i = 0; i < paramLen; i++) {
    sum += params[i];
  }
  uint8_t dcs = (uint8_t)(0x100 - (sum & 0xFF));

  // Build frame
  uint8_t frame[64];
  uint8_t idx = 0;
  frame[idx++] = PN532_PREAMBLE;
  frame[idx++] = PN532_STARTCODE1;
  frame[idx++] = PN532_STARTCODE2;
  frame[idx++] = dataLen;
  frame[idx++] = lcs;
  frame[idx++] = PN532_TFI_HOST2PN;
  frame[idx++] = cmd;
  for (uint8_t i = 0; i < paramLen; i++) {
    frame[idx++] = params[i];
  }
  frame[idx++] = dcs;
  frame[idx++] = PN532_POSTAMBLE;

  // Send frame
  if (!writeRawData(frame, idx)) {
    return false;
  }

  // CRITICAL: Adafruit adds small delay after write to let PN532 process
  delay(1);

  // DEBUG: Check IRQ immediately after write
  if (debug_enabled) {
    char debug_msg[64];
    snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: IRQ after writeRawData: %d (cmd=0x%02X)", digitalRead(PIN_PN532_IRQ), cmd);
    log_println(debug_msg);
  }

  // Wait for PN532 to be ready with ACK (Adafruit does this!)
  // Poll I2C for RDY byte instead of using IRQ (ACK comes fast, IRQ is for response)
  bool ack_ready = false;
  for (int i = 0; i < 20; i++) {  // Max 200ms wait
    uint8_t status;
    if (Wire.requestFrom((uint8_t)PN532_I2C_ADDR, (uint8_t)1) == 1) {
      status = Wire.read();
      if (status == 0x01) {  // RDY bit set
        ack_ready = true;
        if (debug_enabled) {
          char debug_msg[64];
          snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: ACK ready after %dms", i * 10);
          log_println(debug_msg);
        }
        break;
      }
    }
    delay(10);
  }

  if (!ack_ready) {
    if (debug_enabled) {
      log_println("[NFC] DEBUG: Timeout waiting for ACK ready");
    }
    return false;
  }

  // Read and verify ACK (7 bytes: ready byte + 6 ACK bytes)
  // NOTE: ACK comes quickly, IRQ is NOT used for ACK detection!
  // IRQ only signals when RESPONSE data is ready (after ACK)
  uint8_t ack[7];
  if (!readRawData(ack, 7)) {
    if (debug_enabled) {
      log_println("[NFC] DEBUG: Failed to read ACK frame");
    }
    return false;
  }

  // DEBUG: Check IRQ after ACK read and show ACK bytes
  if (debug_enabled) {
    char debug_msg[128];
    snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: IRQ after ACK=%d, ACK bytes: %02X %02X %02X %02X %02X %02X %02X",
             digitalRead(PIN_PN532_IRQ), ack[0], ack[1], ack[2], ack[3], ack[4], ack[5], ack[6]);
    log_println(debug_msg);
  }

  // Verify ACK: first byte is ready (0x01), then ACK frame
  if (ack[0] != 0x01) {
    if (debug_enabled) {
      char debug_msg[64];
      snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: ACK byte 0 wrong: 0x%02X (expected 0x01)", ack[0]);
      log_println(debug_msg);
    }
    return false;
  }
  if (memcmp(&ack[1], PN532_ACK, 6) != 0) {
    if (debug_enabled) {
      log_println("[NFC] DEBUG: ACK frame mismatch");
    }
    return false;
  }

  return true;
}

// Read response frame from PN532
// Returns data length, or -1 on error. Data starts after TFI byte.
int readFrame(uint8_t* buffer, uint8_t maxLen) {
  // DEBUG: Check IRQ before waiting
  if (debug_enabled) {
    char debug_msg[64];
    snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: readFrame start, IRQ=%d", digitalRead(PIN_PN532_IRQ));
    log_println(debug_msg);
  }

  // Wait for device ready
  if (!waitReady()) {
    return -1;
  }

  // Read frame header (ready + preamble + start codes + length info)
  // First byte is ALWAYS the RDY status byte (bit 0 = 1 means ready)
  // This is the Elechouse PN532_I2C protocol requirement
  uint8_t header[7];  // ready, preamble, sc1, sc2, len, lcs, tfi
  if (!readRawData(header, 7)) {
    log_println("[NFC] ERROR: Frame header read failed - check power supply!");
    return -1;
  }

  // Check RDY byte: bit 0 must be 1
  if ((header[0] & 0x01) == 0) {
    log_println("[NFC] ERROR: Frame not ready");
    return -1;
  }
  if (header[1] != PN532_PREAMBLE || header[2] != PN532_STARTCODE1 || header[3] != PN532_STARTCODE2) {
    log_println("[NFC] ERROR: Invalid frame header");
    return -1;
  }

  uint8_t len = header[4];
  uint8_t lcs = header[5];
  if (((len + lcs) & 0xFF) != 0) {
    log_println("[NFC] ERROR: Length checksum error");
    return -1;
  }

  if (header[6] != PN532_TFI_PN2HOST) {
    log_println("[NFC] ERROR: Invalid TFI");
    return -1;
  }

  // Data length is len - 1 (TFI already read)
  uint8_t dataLen = len - 1;
  if (dataLen > maxLen) {
    log_println("[NFC] ERROR: Buffer too small");
    return -1;
  }

  // Read remaining data + DCS + POSTAMBLE
  uint8_t remaining[dataLen + 2];
  if (!readRawData(remaining, dataLen + 2)) {
    log_println("[NFC] ERROR: Frame data read failed - check power supply!");
    return -1;
  }

  // Copy data to buffer
  memcpy(buffer, remaining, dataLen);

  // Verify data checksum (sum of TFI + data + DCS should be 0)
  uint8_t sum = PN532_TFI_PN2HOST;
  for (uint8_t i = 0; i < dataLen; i++) {
    sum += remaining[i];
  }
  sum += remaining[dataLen];  // DCS
  if ((sum & 0xFF) != 0) {
    return -1;  // Checksum failed - silent
  }

  return dataLen;
}

// Send wakeup signal - reset device and configure I2C
void wakeup() {
  // IRQ pin already configured as INPUT in nfc_pn532_init()

  // Hardware reset
  pinMode(PIN_PN532_RST, OUTPUT);
  digitalWrite(PIN_PN532_RST, HIGH);
  delay(10);
  digitalWrite(PIN_PN532_RST, LOW);
  delay(400);  // PN532 needs at least 400ms in reset
  digitalWrite(PIN_PN532_RST, HIGH);

  // Set I2C clock to 100kHz
  Wire.setClock(100000);

  // Wait for device to boot (IRQ should go HIGH after boot)
  delay(500);
}

// Test IRQ pin connectivity - monitor for 2 seconds
void testIrqPin() {
  log_println("[NFC] Testing IRQ pin connectivity...");
  pinMode(PIN_PN532_IRQ, INPUT_PULLUP);

  char debug_msg[64];
  bool seen_low = false;
  bool seen_high = false;

  for (int i = 0; i < 20; i++) {
    int irq_state = digitalRead(PIN_PN532_IRQ);
    snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: IRQ sample %d: %d", i, irq_state);
    log_println(debug_msg);

    if (irq_state == LOW) seen_low = true;
    if (irq_state == HIGH) seen_high = true;

    delay(100);
  }

  if (!seen_low && !seen_high) {
    log_println("[NFC] WARNING: IRQ pin appears disconnected (no readings)");
  } else if (!seen_low) {
    log_println("[NFC] WARNING: IRQ pin stuck HIGH - may not be connected to PN532");
  } else if (!seen_high) {
    log_println("[NFC] WARNING: IRQ pin stuck LOW - check hardware");
  } else {
    log_println("[NFC] IRQ pin appears functional (saw both HIGH and LOW)");
  }
}

// Get firmware version
uint32_t getFirmwareVersion() {
  if (!writeCommand(PN532_CMD_GETFIRMWAREVERSION)) {
    log_println("[NFC] ERROR: Failed to send firmware version command");
    return 0;
  }

  uint8_t response[5];
  int len = readFrame(response, sizeof(response));

  // Debug: show what we received
  char debug_msg[128];
  snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: readFrame returned %d bytes", len);
  log_println(debug_msg);

  if (len > 0) {
    char hex_data[128] = "[NFC] DEBUG: Data: ";
    char hex_byte[8];
    for (int i = 0; i < len && i < 5; i++) {
      snprintf(hex_byte, sizeof(hex_byte), "%02X ", response[i]);
      strcat(hex_data, hex_byte);
    }
    log_println(hex_data);
  }

  if (len < 5) {
    log_println("[NFC] ERROR: Firmware response too short - check power supply!");
    return 0;
  }

  // Response: CMD+1, IC, Ver, Rev, Support
  if (response[0] != (PN532_CMD_GETFIRMWAREVERSION + 1)) {
    snprintf(debug_msg, sizeof(debug_msg), "[NFC] ERROR: Invalid firmware response (expected 0x03, got 0x%02X)", response[0]);
    log_println(debug_msg);
    return 0;
  }

  return ((uint32_t)response[1] << 24) | ((uint32_t)response[2] << 16) |
         ((uint32_t)response[3] << 8) | response[4];
}

// Configure SAM (Security Access Module)
bool samConfig() {
  char debug_msg[64];
  snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: IRQ pin state before SAM: %d", digitalRead(PIN_PN532_IRQ));
  log_println(debug_msg);

  // CRITICAL: 3rd parameter (0x01) enables IRQ pin usage!
  uint8_t params[] = {0x01, 0x14, 0x01};  // Normal mode, timeout 1 sec, USE IRQ PIN!
  if (!writeCommand(PN532_CMD_SAMCONFIGURATION, params, sizeof(params))) {
    log_println("[NFC] DEBUG: writeCommand failed for SAM");
    return false;
  }

  snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: IRQ pin state after write: %d", digitalRead(PIN_PN532_IRQ));
  log_println(debug_msg);

  uint8_t response[2];
  int len = readFrame(response, sizeof(response));

  snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: readFrame returned %d bytes", len);
  log_println(debug_msg);

  if (len < 1) {
    log_println("[NFC] DEBUG: SAM response too short");
    return false;
  }

  snprintf(debug_msg, sizeof(debug_msg), "[NFC] DEBUG: SAM response[0] = 0x%02X (expected 0x15)", response[0]);
  log_println(debug_msg);

  return response[0] == (PN532_CMD_SAMCONFIGURATION + 1);
}

// List passive target (read card UID)
bool readPassiveTarget(uint8_t* uid, uint8_t* uidLen, uint16_t timeoutMs) {
  uint8_t params[] = {0x01, 0x00};  // 1 target, 106 kbps type A
  if (!writeCommand(PN532_CMD_INLISTPASSIVETARGET, params, sizeof(params))) {
    return false;
  }

  // Wait longer for card detection
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    uint8_t status;
    if (readRawData(&status, 1, 1, 0) && status == 0x01) {
      break;
    }
    delay(10);
  }
  if (millis() - start >= timeoutMs) {
    // No card - send ACK to abort
    writeRawData(PN532_ACK, sizeof(PN532_ACK));
    return false;
  }

  uint8_t response[32];
  int len = readFrame(response, sizeof(response));
  if (len < 6) {
    return false;
  }

  // Response: CMD+1, NbTg, Tg, SENS_RES[2], SEL_RES, NFCIDLength, NFCID1[n]
  if (response[0] != (PN532_CMD_INLISTPASSIVETARGET + 1)) {
    return false;
  }
  if (response[1] == 0) {
    return false;  // No target found
  }

  uint8_t nfcidLen = response[6];
  if (nfcidLen > 10 || nfcidLen > (uint8_t)(len - 7)) {
    return false;
  }

  *uidLen = nfcidLen;
  memcpy(uid, &response[7], nfcidLen);
  return true;
}

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

} // namespace

void nfc_pn532_init() {
    log_println("[NFC] Initializing Elechouse PN532 V3 @ 0x24...");
    Serial.flush();

    // Setup IRQ pin FIRST (Adafruit does this in constructor)
    pinMode(PIN_PN532_IRQ, INPUT);
    log_println("[NFC] IRQ pin configured as INPUT");

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(100000);  // Force 100kHz

    // Quick I2C scan
    Wire.beginTransmission(PN532_I2C_ADDR);
    if (Wire.endTransmission() != 0) {
        log_println("[NFC] ERROR: No device at 0x24");
        nfc_ready = false;
        return;
    }
    log_println("[NFC] Device found at 0x24");

    // Test IRQ pin before wakeup
    testIrqPin();

    // Wakeup PN532
    log_println("[NFC] Sending wakeup sequence...");
    wakeup();
    delay(300);  // Give more time after wakeup before first command
    log_println("[NFC] Wakeup complete");

    // Skip firmware version check - go straight to SAM config
    log_println("[NFC] Configuring SAM...");
    if (!samConfig()) {
        log_println("[NFC] WARNING: SAM config failed, trying anyway...");
    } else {
        log_println("[NFC] SAM configured");
    }

    log_println("[NFC] Ready - waiting for ISO14443A tags...");
    nfc_ready = true;
}

void nfc_pn532_poll() {
  if (!nfc_ready) return;

  static uint32_t last_poll = 0;
  uint32_t now = millis();
  if (now - last_poll < 250) return;
  last_poll = now;

  uint8_t uid[10];
  uint8_t uidLen = 0;

  if (readPassiveTarget(uid, &uidLen, 100)) {
    if (!uid_equals_last(uid, uidLen)) {
      print_uid(uid, uidLen);
      store_last_uid(uid, uidLen);
    }
  }
}

#endif
