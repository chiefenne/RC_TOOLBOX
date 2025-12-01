// fonts.h
#pragma once
#include <lvgl.h>

// Arial fonts (proportional) with German umlauts (ÄÖÜäöüß)
extern const lv_font_t arial_12;
extern const lv_font_t arial_14;
extern const lv_font_t arial_16;
extern const lv_font_t arial_18;
extern const lv_font_t arial_20;
extern const lv_font_t arial_24;
extern const lv_font_t arial_28;

#define FONT_DEFAULT   (&arial_14)
#define FONT_FOOTER    (&arial_18)
#define FONT_HEADER    (&arial_24)

// Courier New fonts (monospace/fixed-width) for numeric displays
extern const lv_font_t courier_new_12;
extern const lv_font_t courier_new_14;
extern const lv_font_t courier_new_16;
extern const lv_font_t courier_new_18;
extern const lv_font_t courier_new_20;
extern const lv_font_t courier_new_24;
extern const lv_font_t courier_new_28;

#define FONT_MONO_SM   (&courier_new_14)
#define FONT_MONO_MD   (&courier_new_18)
#define FONT_MONO_LG   (&courier_new_24)
#define FONT_MONO_XL   (&courier_new_28)

// Legacy Roboto Mono fonts
extern const lv_font_t lv_font_roboto_mono_40;
extern const lv_font_t lv_font_roboto_mono_46;

#define FONT_ROBOTO_40 (&lv_font_roboto_mono_40)
#define FONT_ROBOTO_46 (&lv_font_roboto_mono_46)
