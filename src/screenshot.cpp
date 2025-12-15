// src/screenshot.cpp - Screenshot server implementation
// Reads pixels from display, writes BMP to SPIFFS, serves via HTTP
// Uses AP mode - ESP32 creates its own WiFi network

#ifdef ESP32

#include "screenshot.h"
#include "gui/config/settings.h"

#include <WiFi.h>
#include <esp_http_server.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>

// External TFT instance from main.cpp
extern TFT_eSPI tft;

// AP configuration
#define AP_SSID     "RC_TOOLBOX"
#define AP_PASSWORD "rctoolbox"  // Min 8 characters
#define AP_CHANNEL  1
#define AP_MAX_CONN 2

// Screen dimensions
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// BMP file
#define BMP_HEADER_SIZE 54
#define BMP_DATA_SIZE   (SCREEN_WIDTH * SCREEN_HEIGHT * 3)
#define BMP_FILE_SIZE   (BMP_HEADER_SIZE + BMP_DATA_SIZE)
#define BMP_FILE_PATH   "/screenshot.bmp"

// Refresh interval lookup table (index -> seconds)
static const int REFRESH_INTERVALS[] = { 5, 10, 15, 20, 30 };

// State
static httpd_handle_t http_server = nullptr;
static bool server_running = false;
static char ip_address[16] = "";
static bool spiffs_mounted = false;

// HTML page template with %d placeholder for refresh interval
static const char* HTML_PAGE_TEMPLATE = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>RC TOOLBOX Screenshot</title>
    <meta http-equiv="refresh" content="%d">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background: #1a1a2e;
            color: #eee;
            padding: 20px;
        }
        h1 { color: #4a9eff; }
        img {
            border: 2px solid #4a9eff;
            border-radius: 8px;
            image-rendering: pixelated;
        }
        .info {
            margin: 20px;
            color: #888;
        }
        a {
            color: #4a9eff;
            text-decoration: none;
        }
        a:hover { text-decoration: underline; }
    </style>
</head>
<body>
    <h1>RC TOOLBOX Screenshot</h1>
    <img src="/screenshot.bmp" alt="Screenshot" width="640" height="480">
    <p class="info">Auto-refreshes every %d seconds</p>
    <p><a href="/screenshot.bmp" download="screenshot.bmp">Download BMP</a></p>
</body>
</html>
)rawliteral";

// Write BMP header to buffer
static void write_bmp_header(uint8_t* buf) {
    // BMP file header (14 bytes)
    buf[0] = 'B';
    buf[1] = 'M';
    // File size (little-endian)
    buf[2] = (BMP_FILE_SIZE) & 0xFF;
    buf[3] = (BMP_FILE_SIZE >> 8) & 0xFF;
    buf[4] = (BMP_FILE_SIZE >> 16) & 0xFF;
    buf[5] = (BMP_FILE_SIZE >> 24) & 0xFF;
    // Reserved
    buf[6] = 0; buf[7] = 0; buf[8] = 0; buf[9] = 0;
    // Pixel data offset
    buf[10] = BMP_HEADER_SIZE; buf[11] = 0; buf[12] = 0; buf[13] = 0;

    // DIB header (BITMAPINFOHEADER - 40 bytes)
    buf[14] = 40; buf[15] = 0; buf[16] = 0; buf[17] = 0;  // Header size
    // Width
    buf[18] = (SCREEN_WIDTH) & 0xFF;
    buf[19] = (SCREEN_WIDTH >> 8) & 0xFF;
    buf[20] = 0; buf[21] = 0;
    // Height (negative for top-down)
    int32_t height = -SCREEN_HEIGHT;
    buf[22] = height & 0xFF;
    buf[23] = (height >> 8) & 0xFF;
    buf[24] = (height >> 16) & 0xFF;
    buf[25] = (height >> 24) & 0xFF;
    // Planes
    buf[26] = 1; buf[27] = 0;
    // Bits per pixel (24)
    buf[28] = 24; buf[29] = 0;
    // Compression (0 = none)
    buf[30] = 0; buf[31] = 0; buf[32] = 0; buf[33] = 0;
    // Image size (can be 0 for uncompressed)
    buf[34] = 0; buf[35] = 0; buf[36] = 0; buf[37] = 0;
    // Pixels per meter X (2835 = 72 DPI)
    buf[38] = 0x13; buf[39] = 0x0B; buf[40] = 0; buf[41] = 0;
    // Pixels per meter Y
    buf[42] = 0x13; buf[43] = 0x0B; buf[44] = 0; buf[45] = 0;
    // Colors in palette
    buf[46] = 0; buf[47] = 0; buf[48] = 0; buf[49] = 0;
    // Important colors
    buf[50] = 0; buf[51] = 0; buf[52] = 0; buf[53] = 0;
}

// Convert RGB565 to RGB888 and write to BMP buffer
static void convert_rgb565_to_bmp(const uint16_t* src, uint8_t* dst, int width, int height) {
    // BMP rows must be padded to 4-byte boundary
    int row_size = width * 3;
    int padding = (4 - (row_size % 4)) % 4;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t pixel = src[y * width + x];

            // RGB565: RRRRRGGGGGGBBBBB
            uint8_t r = ((pixel >> 11) & 0x1F) << 3;  // 5 bits -> 8 bits
            uint8_t g = ((pixel >> 5) & 0x3F) << 2;   // 6 bits -> 8 bits
            uint8_t b = (pixel & 0x1F) << 3;          // 5 bits -> 8 bits

            // BMP stores as BGR
            int idx = y * (row_size + padding) + x * 3;
            dst[idx + 0] = b;
            dst[idx + 1] = g;
            dst[idx + 2] = r;
        }
        // Add padding bytes
        for (int p = 0; p < padding; p++) {
            dst[y * (row_size + padding) + row_size + p] = 0;
        }
    }
}

// HTTP handler: GET /
static esp_err_t handler_root(httpd_req_t* req) {
    // Get refresh interval from settings
    int interval_idx = g_settings.screenshot_interval;
    if (interval_idx < 0 || interval_idx >= (int)(sizeof(REFRESH_INTERVALS)/sizeof(REFRESH_INTERVALS[0]))) {
        interval_idx = 2;  // Default to 15 seconds
    }
    int refresh_sec = REFRESH_INTERVALS[interval_idx];

    // Build HTML with dynamic refresh interval
    char html_buf[2048];
    snprintf(html_buf, sizeof(html_buf), HTML_PAGE_TEMPLATE, refresh_sec, refresh_sec);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_buf, strlen(html_buf));
    return ESP_OK;
}

// HTTP handler: GET /screenshot.bmp
static esp_err_t handler_screenshot(httpd_req_t* req) {
    Serial.println("Screenshot: BMP request received");

    // Capture screenshot from display to SPIFFS
    File file = SPIFFS.open(BMP_FILE_PATH, FILE_WRITE);
    if (!file) {
        Serial.println("Screenshot: Failed to create file");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }

    // Write BMP header
    uint8_t header[BMP_HEADER_SIZE];
    write_bmp_header(header);
    file.write(header, BMP_HEADER_SIZE);

    // Buffer for one row of pixels (RGB565 from display + BGR888 for BMP)
    uint16_t* row_rgb565 = (uint16_t*)malloc(SCREEN_WIDTH * 2);
    uint8_t* row_bgr888 = (uint8_t*)malloc(SCREEN_WIDTH * 3);

    if (!row_rgb565 || !row_bgr888) {
        if (row_rgb565) free(row_rgb565);
        if (row_bgr888) free(row_bgr888);
        file.close();
        SPIFFS.remove(BMP_FILE_PATH);
        Serial.println("Screenshot: Row buffer allocation failed");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
        return ESP_FAIL;
    }

    Serial.println("Screenshot: Reading display...");

    // Read display line by line and write to file
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        // Read one row from display (uses TFT_eSPI readRect)
        tft.readRect(0, y, SCREEN_WIDTH, 1, row_rgb565);

        // Convert RGB565 to BGR888
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            uint16_t pixel = row_rgb565[x];
            uint8_t r = ((pixel >> 11) & 0x1F) << 3;
            uint8_t g = ((pixel >> 5) & 0x3F) << 2;
            uint8_t b = (pixel & 0x1F) << 3;

            row_bgr888[x * 3 + 0] = b;
            row_bgr888[x * 3 + 1] = g;
            row_bgr888[x * 3 + 2] = r;
        }

        file.write(row_bgr888, SCREEN_WIDTH * 3);
    }

    free(row_rgb565);
    free(row_bgr888);
    file.close();

    Serial.printf("Screenshot: BMP saved (%d bytes)\n", BMP_HEADER_SIZE + SCREEN_WIDTH * SCREEN_HEIGHT * 3);

    // Now serve the file
    file = SPIFFS.open(BMP_FILE_PATH, FILE_READ);
    if (!file) {
        Serial.println("Screenshot: Failed to open file for reading");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read file");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/bmp");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=\"screenshot.bmp\"");

    // Send file in chunks
    uint8_t chunk[1024];
    size_t bytes_read;
    while ((bytes_read = file.read(chunk, sizeof(chunk))) > 0) {
        if (httpd_resp_send_chunk(req, (const char*)chunk, bytes_read) != ESP_OK) {
            file.close();
            Serial.println("Screenshot: Send failed");
            return ESP_FAIL;
        }
    }
    httpd_resp_send_chunk(req, NULL, 0);  // End chunked response

    file.close();
    Serial.println("Screenshot: BMP sent successfully");
    return ESP_OK;
}

// Start HTTP server
static bool start_http_server() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.stack_size = 8192;

    if (httpd_start(&http_server, &config) != ESP_OK) {
        Serial.println("Screenshot: Failed to start HTTP server");
        return false;
    }

    // Register URI handlers
    httpd_uri_t uri_root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = handler_root,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(http_server, &uri_root);

    httpd_uri_t uri_screenshot = {
        .uri = "/screenshot.bmp",
        .method = HTTP_GET,
        .handler = handler_screenshot,
        .user_ctx = nullptr
    };
    httpd_register_uri_handler(http_server, &uri_screenshot);

    return true;
}

// Stop HTTP server
static void stop_http_server() {
    if (http_server) {
        httpd_stop(http_server);
        http_server = nullptr;
    }
}

// Public API implementation
void screenshot_init(void) {
    // Nothing to initialize for AP mode
}

bool screenshot_start(void) {
    if (server_running) {
        return true;  // Already running
    }

    Serial.println("Screenshot: Starting...");

    // Mount SPIFFS for storing screenshots
    if (!spiffs_mounted) {
        if (!SPIFFS.begin(true)) {  // true = format if mount fails
            Serial.println("Screenshot: SPIFFS mount failed");
            return false;
        }
        spiffs_mounted = true;
        Serial.printf("Screenshot: SPIFFS mounted, total: %d, used: %d bytes\n",
                      SPIFFS.totalBytes(), SPIFFS.usedBytes());
    }

    // Start Access Point
    WiFi.mode(WIFI_AP);

    if (!WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CONN)) {
        Serial.println("Screenshot: Failed to start AP");
        return false;
    }

    // Get AP IP address (usually 192.168.4.1)
    IPAddress ip = WiFi.softAPIP();
    snprintf(ip_address, sizeof(ip_address), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    Serial.printf("Screenshot: AP started - SSID: %s, IP: %s\n", AP_SSID, ip_address);

    // Start HTTP server
    if (!start_http_server()) {
        WiFi.softAPdisconnect(true);
        return false;
    }

    Serial.printf("Screenshot: Connect to WiFi '%s' (password: %s)\n", AP_SSID, AP_PASSWORD);
    Serial.printf("Screenshot: Then open http://%s/\n", ip_address);
    server_running = true;
    return true;
}

void screenshot_stop(void) {
    if (!server_running) {
        return;
    }

    stop_http_server();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    ip_address[0] = '\0';
    server_running = false;

    // Remove screenshot file from SPIFFS
    if (spiffs_mounted && SPIFFS.exists(BMP_FILE_PATH)) {
        SPIFFS.remove(BMP_FILE_PATH);
    }

    Serial.println("Screenshot: AP stopped");
}

bool screenshot_is_running(void) {
    return server_running;
}

const char* screenshot_get_ip(void) {
    return ip_address;
}

#endif // ESP32
