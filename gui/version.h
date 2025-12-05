// gui/version.h - Central version information

#pragma once

// Application version
#define APP_VERSION "0.2"

// LVGL version - build string from major.minor.patch
#include "lvgl.h"
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LVGL_VERSION_STRING TOSTRING(LVGL_VERSION_MAJOR) "." TOSTRING(LVGL_VERSION_MINOR) "." TOSTRING(LVGL_VERSION_PATCH)
