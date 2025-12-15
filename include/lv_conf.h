// include/lv_conf.h - LVGL configuration for RC TOOLBOX (ESP32-S3)
// Compare with simulator's lv_conf.h and align settings as needed

#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/*====================
   MEMORY SETTINGS
 *====================*/
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (48 * 1024U)  // 48KB for LVGL

/*====================
   FONT SETTINGS
 *====================*/
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Enable Montserrat fonts */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0

/* Pixel perfect monospaced fonts */
#define LV_FONT_UNSCII_8  0
#define LV_FONT_UNSCII_16 0

/*=================
 *  TEXT SETTINGS
 *=================*/
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/*====================
   WIDGET SETTINGS
 *====================*/
#define LV_USE_SLIDER 1
#define LV_USE_BUTTON 1
#define LV_USE_LABEL 1
#define LV_USE_IMG 1
#define LV_USE_LINE 1
#define LV_USE_ARC 1
#define LV_USE_BAR 1

/*====================
   MISC SETTINGS
 *====================*/
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/* Enable snapshot API for screenshots */
#define LV_USE_SNAPSHOT 1

/* Enable animations */
#define LV_USE_ANIMATION 1

/* Enable flex layout */
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

#endif /* LV_CONF_H */
