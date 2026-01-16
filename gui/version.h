// gui/version.h - Central version and app information

#pragma once

// Application info (not translated)
#define APP_TITLE "RC TOOLBOX"
#define APP_VERSION "0.7"

// Author and project info (not translated)
#define APP_AUTHOR_NAME "Dipl.-Ing. Andreas Ennemoser"
#define APP_GITHUB_URL "github.com/chiefenne/RC_TOOLBOX"

// LVGL version - build string from major.minor.patch
#include "lvgl.h"
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LVGL_VERSION_STRING TOSTRING(LVGL_VERSION_MAJOR) "." TOSTRING(LVGL_VERSION_MINOR) "." TOSTRING(LVGL_VERSION_PATCH)
