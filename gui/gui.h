// gui/gui.h – ultra-minimal version for simulator
#pragma once
#include "lvgl.h"

enum GuiPage {
    PAGE_HOME = 0,
    PAGE_SERVO,
    PAGE_LIPO,
    PAGE_DEFLECTION,
    PAGE_ANGLE,
    PAGE_SETTINGS,
    PAGE_ABOUT,
    PAGE_COUNT
};

void gui_init();
void gui_set_page(GuiPage page);
