// include/screenshot.h - Screenshot server for documentation
// Captures LVGL screen and serves via HTTP

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize screenshot module (call once at startup)
void screenshot_init(void);

// Start WiFi and HTTP server
// Returns true if started successfully, false on error
bool screenshot_start(void);

// Stop HTTP server and disconnect WiFi
void screenshot_stop(void);

// Check if screenshot server is running
bool screenshot_is_running(void);

// Get IP address string (valid only when running)
// Returns pointer to static buffer, e.g., "192.168.1.123"
const char* screenshot_get_ip(void);

#ifdef __cplusplus
}
#endif

#endif // SCREENSHOT_H
