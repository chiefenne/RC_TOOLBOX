#pragma once

// ============================================================================
// SERIAL LOGGING UTILITY
// ============================================================================
// General serial logging that outputs to both Serial port and TFT Serial Monitor page
//
// USAGE:
//   Instead of:  Serial.println("Message");
//   Use:         log_println("Message");
//
//   Instead of:  Serial.print("Value: "); Serial.println(value);
//   Use:         serial_printf("Value: %d\n", value);
//
// The Serial Monitor page (Home → Serial Monitor) will display all messages
// in a scrolling terminal-style view (green on black).
// ============================================================================

#if defined(ESP_PLATFORM) || defined(ARDUINO)
#include <Arduino.h>

// Add a message to the on-screen serial monitor
void serial_log_add_message(const char* msg);

// Print to both Serial and TFT display (single line, auto-newline)
void log_println(const char* msg);

// Print formatted string to both Serial and TFT display
void serial_printf(const char* format, ...);

// For building multi-part lines, call log_print() then log_println() to flush
void log_print(const char* msg);

#else
// Simulator stub
inline void serial_log_add_message(const char* msg) {}
inline void log_println(const char* msg) {}
inline void serial_printf(const char* format, ...) {}
inline void log_print(const char* msg) {}
#endif
