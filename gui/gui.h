// gui/gui.h – ultra-minimal version for simulator
#pragma once
#include "lvgl.h"

enum GuiPage {
    PAGE_HOME = 0,
    PAGE_SERVO,
    PAGE_LIPO,
    PAGE_CG_SCALE,
    PAGE_DEFLECTION,
    PAGE_ANGLE,
    PAGE_SETTINGS,
    PAGE_ABOUT,
    PAGE_COUNT
};

// Background color presets
enum BgColorPreset {
    BG_COLOR_LIGHT_GRAY = 0,  // Default - GUI_COLOR_GRAYS[8]
    BG_COLOR_WHITE,
    BG_COLOR_LIGHT_BLUE,
    BG_COLOR_LIGHT_GREEN,
    BG_COLOR_CREAM,
    BG_COLOR_COUNT
};

void gui_init();
void gui_set_page(GuiPage page);
GuiPage gui_get_current_page();
void gui_set_bg_color(BgColorPreset preset);
BgColorPreset gui_get_bg_color();
