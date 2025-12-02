/*******************************************************************************
 * Size: 28 px
 * Bpp: 4
 * Opts: --size 28 --bpp 4 --no-compress --stride 1 --align 1 --font /System/Library/Fonts/Supplemental/STIXTwoMath.otf --format lvgl --range 0x2699 --output stixtwomath_28.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

#if !LV_VERSION_CHECK(9, 3, 0)
#error "At least LVGL v9.3 is required to use the stride attribute of the fonts"
#endif

#ifndef STIXTWOMATH_28
#define STIXTWOMATH_28 1
#endif

#if STIXTWOMATH_28

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+2699 "⚙" */
    0x00, 0x00, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7f, 0xf6, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x8f, 0x30, 0x9f, 0xf8, 0x04, 0xf6, 0x00, 0x00,
    0x00, 0x08, 0xff, 0xe8, 0xef, 0xfe, 0x8f, 0xff, 0x50, 0x00,
    0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00,
    0x02, 0x10, 0xcf, 0xf9, 0x30, 0x03, 0xaf, 0xf8, 0x01, 0x10,
    0x0e, 0xfe, 0xff, 0x30, 0x00, 0x00, 0x05, 0xff, 0xef, 0xb0,
    0x3f, 0xff, 0xf6, 0x00, 0x05, 0x50, 0x00, 0x9f, 0xff, 0xf1,
    0x1a, 0xff, 0xf0, 0x00, 0xdf, 0xfb, 0x00, 0x2f, 0xff, 0x80,
    0x00, 0x4f, 0xc0, 0x04, 0xff, 0xff, 0x20, 0x0f, 0xf1, 0x00,
    0x00, 0x8f, 0xc0, 0x03, 0xff, 0xff, 0x10, 0x0f, 0xf6, 0x00,
    0x3f, 0xff, 0xf0, 0x00, 0x7f, 0xf6, 0x00, 0x3f, 0xff, 0xe1,
    0x2f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xf0,
    0x0b, 0xd9, 0xff, 0x60, 0x00, 0x00, 0x08, 0xfe, 0xad, 0x90,
    0x00, 0x00, 0xbf, 0xfc, 0x63, 0x36, 0xdf, 0xf9, 0x00, 0x00,
    0x00, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x10, 0x00,
    0x00, 0x07, 0xff, 0xd5, 0xdf, 0xfd, 0x5e, 0xff, 0x50, 0x00,
    0x00, 0x00, 0x6c, 0x10, 0x9f, 0xf8, 0x03, 0xc4, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x6f, 0xf5, 0x00, 0x00, 0x00, 0x00
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 344, .box_w = 20, .box_h = 19, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 9881, .range_length = 1, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
    .stride = 1
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t stixtwomath_28 = {
#else
lv_font_t stixtwomath_28 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 19,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif

#if LV_VERSION_CHECK(9, 3, 0)
    .static_bitmap = 1,    /*Bitmaps are stored as const so they are always static if not compressed */
#endif

    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if STIXTWOMATH_28*/
