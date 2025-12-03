# RC TOOLBOX - Copilot Instructions

## Project Overview
RC TOOLBOX is an embedded GUI application for RC enthusiasts running on ESP32-S3 with an ILI9341 TFT touch display (320x240). Built with **LVGL 9.4.0** and **C++17**.

## Architecture

### Dual Build Targets
- **ESP32 (PlatformIO)**: `src/main.cpp` → production hardware with TFT_eSPI + XPT2046 touch
- **macOS Simulator (SDL2)**: `simulator/main.cpp` → rapid GUI development without hardware

The `gui/` folder is shared between both targets—code must compile for both.

### Key Directories
- `gui/` – shared GUI code (pages, fonts, colors, i18n)
- `gui/pages/` – one file per page: `page_*.cpp` + `page_*.h`
- `gui/lang/` – translations: `strings_en.h`, `strings_de.h`
- `gui/fonts/` – pre-generated LVGL font files (Arial, Courier New, STIXTwoMath)
- `simulator/` – macOS simulator with SDL2
- `include/lv_conf.h` – ESP32 LVGL config
- `lvgl/lv_conf.h` – Simulator LVGL config

### Page Navigation Pattern
Pages follow this pattern (see `gui/pages/page_home.cpp`):
```cpp
#include "gui/gui.h"
void page_xxx_create(lv_obj_t* parent) {
    // Build UI inside parent container
    // Use gui_set_page(PAGE_XXX) for navigation
}
```
Register new pages in `gui/gui.h` (enum `GuiPage`) and `gui/gui.cpp` (switch in `gui_set_page`).

## Build Commands

### Simulator (macOS)
```bash
./simulator/build_sim.sh        # Release build → binaries/lvgl_simulator_macOS
./simulator/build_sim_debug.sh  # Debug build → binaries/lvgl_simulator_macOS_debug
```
Requires: SDL2 (`brew install sdl2`), pre-built `lvgl/liblvgl.a`

### Rebuilding LVGL Library (one-time or after LVGL updates)
The simulator links against `lvgl/liblvgl.a`. To rebuild it:
```bash
cd lvgl
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DLVGL_BUILD_STATIC=ON
make -j8
cd ..
ln -sf build/lib/liblvgl.a liblvgl.a
```
The library uses `lvgl/lv_conf.h` for simulator settings. Keep this in sync with `include/lv_conf.h` (ESP32) when changing LVGL features. See [LVGL_Simulator](https://github.com/chiefenne/LVGL_Simulator) for the full template.

### ESP32 (PlatformIO)
```bash
pio run                         # Build for esp32-s3-devkitc-1
pio run -t upload              # Flash to device
```

## Conventions

### Colors
Use `GUI_COLOR_*` arrays from `gui/color_palette.h`:
- `GUI_COLOR_MONO[]` – header/accent colors
- `GUI_COLOR_GRAYS[]` – neutral UI elements
- `GUI_COLOR_TRIAD[]` – interaction states (active/pressed)

### Fonts
Use semantic macros from `gui/fonts.h`:
- `FONT_DEFAULT`, `FONT_HEADER`, `FONT_FOOTER` – proportional (Arial)
- `FONT_MONO_*` – numeric displays (Courier New)
- `FONT_SYMBOLS` – gear icon (STIXTwoMath)

### Internationalization
1. Add string ID to `gui/lang.h` (enum `StringId`)
2. Add translations to `gui/lang/strings_en.h` and `strings_de.h`
3. Use `tr(STR_XXX)` to get translated text

### LVGL Patterns
- Layouts: prefer `lv_obj_set_flex_flow()` for responsive layouts
- Styling: use `lv_obj_set_style_*()` with `lv_color_hex(GUI_COLOR_*[n])`
- Events: `lv_obj_add_event_cb(obj, callback, LV_EVENT_CLICKED, nullptr)`

## Platform Differences
- ESP32: Touch input via XPT2046, real servo PWM output
- Simulator: Mouse = touch, keyboard arrows for servo PWM adjustment
- `sim_state.cpp` provides `gui_sim_init()` for simulator-specific setup

## Adding a New Page
1. Create `gui/pages/page_xxx.cpp` and `page_xxx.h`
2. Add `PAGE_XXX` to `GuiPage` enum in `gui/gui.h`
3. Add case in `gui_set_page()` in `gui/gui.cpp`
4. Add string IDs for page title in `gui/lang.h` and translations
5. Add navigation button in `page_home.cpp` if needed
