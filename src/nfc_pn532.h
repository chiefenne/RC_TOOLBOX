// nfc_pn532.h - NFC driver interface using Adafruit PN532 library
#pragma once

#if defined(ESP_PLATFORM) || defined(ARDUINO)

// Initialize NFC reader
void nfc_pn532_init();

// Poll for NFC tags (call periodically)
void nfc_pn532_poll();

#endif
