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

// Arial Bold fonts
extern const lv_font_t arial_bold_12;
extern const lv_font_t arial_bold_14;
extern const lv_font_t arial_bold_16;
extern const lv_font_t arial_bold_18;
extern const lv_font_t arial_bold_20;
extern const lv_font_t arial_bold_24;
extern const lv_font_t arial_bold_28;

#define FONT_DEFAULT   (&arial_14)
#define FONT_FOOTER    (&arial_18)
#define FONT_HEADER    (&arial_24)
#define FONT_BOLD_SM   (&arial_bold_14)
#define FONT_BOLD_MD   (&arial_bold_18)
#define FONT_BOLD_LG   (&arial_bold_24)

// Button/UI fonts
#define FONT_BUTTON_SM (&arial_14)
#define FONT_BUTTON_SMMD (&arial_16)
#define FONT_BUTTON_MD (&arial_18)
#define FONT_BUTTON_LG (&arial_20)
#define FONT_BUTTON_XL (&arial_24)

// Courier New fonts (monospace/fixed-width) for numeric displays
extern const lv_font_t courier_new_12;
extern const lv_font_t courier_new_14;
extern const lv_font_t courier_new_16;
extern const lv_font_t courier_new_18;
extern const lv_font_t courier_new_20;
extern const lv_font_t courier_new_24;
extern const lv_font_t courier_new_28;

// Courier New Bold fonts
extern const lv_font_t courier_new_bold_12;
extern const lv_font_t courier_new_bold_14;
extern const lv_font_t courier_new_bold_16;
extern const lv_font_t courier_new_bold_18;
extern const lv_font_t courier_new_bold_20;
extern const lv_font_t courier_new_bold_24;
extern const lv_font_t courier_new_bold_28;

#define FONT_MONO_SM   (&courier_new_14)
#define FONT_MONO_MD   (&courier_new_18)
#define FONT_MONO_LG   (&courier_new_24)
#define FONT_MONO_XL   (&courier_new_28)
#define FONT_MONO_BOLD_SM   (&courier_new_bold_14)
#define FONT_MONO_BOLD_MD   (&courier_new_bold_18)
#define FONT_MONO_BOLD_LG   (&courier_new_bold_24)
#define FONT_MONO_BOLD_XL   (&courier_new_bold_28)

// STIXTwoMath font (contains gear symbol ⚙ U+2699)
extern const lv_font_t stixtwomath_24;
#define FONT_SYMBOLS       (&stixtwomath_24)

// Custom symbols (UTF-8 encoded) - use these instead of LV_SYMBOL_* macros
// These must be included in your font's character range
#define SYM_LEFT      "\xE2\x97\x84"   // ◄ U+25C4 BLACK LEFT-POINTING POINTER
#define SYM_RIGHT     "\xE2\x96\xBA"   // ► U+25BA BLACK RIGHT-POINTING POINTER
#define SYM_SETTINGS  "\xE2\x9A\x99"   // ⚙ U+2699 GEAR
#define SYM_UP        "\xE2\x96\xB2"   // ▲ U+25B2 BLACK UP-POINTING TRIANGLE
#define SYM_DOWN      "\xE2\x96\xBC"   // ▼ U+25BC BLACK DOWN-POINTING TRIANGLE
#define SYM_COPYWRIGHT "\xC2\xA9"       // © U+00A9 COPYRIGHT SIGN

