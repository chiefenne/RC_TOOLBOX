#pragma once

#if defined(ESP_PLATFORM) || defined(ARDUINO)

void nfc_pn532_init();
void nfc_pn532_poll();

#endif
