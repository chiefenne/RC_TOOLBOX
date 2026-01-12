#include "serial_log.h"

#if defined(ESP_PLATFORM) || defined(ARDUINO)

#include <Arduino.h>
#include <cstdarg>
#include "gui/pages/page_serial.h"

static char line_buffer[256] = "";

void serial_log_add_message(const char* msg) {
    page_serial_add_message(msg);
}

void log_println(const char* msg) {
    Serial.println(msg);
    page_serial_add_message(msg);
    line_buffer[0] = '\0';  // Clear any partial line
}

void serial_printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.print(buffer);

    // Accumulate in line buffer
    strncat(line_buffer, buffer, sizeof(line_buffer) - strlen(line_buffer) - 1);

    // If there's a newline, flush to display
    char* newline = strchr(line_buffer, '\n');
    while (newline) {
        *newline = '\0';  // Replace newline with null terminator
        page_serial_add_message(line_buffer);

        // Move remaining text to start of buffer
        size_t remaining_len = strlen(newline + 1);
        memmove(line_buffer, newline + 1, remaining_len + 1);

        newline = strchr(line_buffer, '\n');
    }
}

void log_print(const char* msg) {
    Serial.print(msg);

    // Accumulate in line buffer (no newline = no flush to display yet)
    strncat(line_buffer, msg, sizeof(line_buffer) - strlen(line_buffer) - 1);

    // If there's a newline, flush to display
    char* newline = strchr(line_buffer, '\n');
    if (newline) {
        *newline = '\0';
        page_serial_add_message(line_buffer);

        // Move remaining text to start
        size_t remaining_len = strlen(newline + 1);
        memmove(line_buffer, newline + 1, remaining_len + 1);
    }
}

#endif
