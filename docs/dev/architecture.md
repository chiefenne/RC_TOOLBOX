# Architecture

!!! note "Under Development"
    This developer documentation is being written.

## Overview

RC TOOLBOX is an embedded GUI application for ESP32-S3 with an ILI9341 TFT touch display (320×240). Built with **LVGL 9.4.0** and **C++17**.

## Dual Build Targets

- **ESP32 (PlatformIO)**: `src/main.cpp` → production hardware with TFT_eSPI + XPT2046 touch
- **macOS Simulator (SDL2)**: `simulator/main.cpp` → rapid GUI development without hardware

The `gui/` folder is shared between both targets.

## Key Directories

| Directory | Purpose |
|-----------|--------|
| `gui/` | Shared GUI code (pages, fonts, colors, i18n) |
| `gui/pages/` | One file per page: `page_*.cpp` + `page_*.h` |
| `gui/lang/` | Translations: `strings_en.h`, `strings_de.h`, etc. |
| `gui/fonts/` | Pre-generated LVGL font files |
| `simulator/` | macOS simulator with SDL2 |
| `include/` | Hardware-specific headers |

*More details coming soon.*
