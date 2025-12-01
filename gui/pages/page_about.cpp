#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include <cstdio>

void page_about_create(lv_obj_t* parent) {
    // Author
    lv_obj_t* author = lv_label_create(parent);
    char author_text[128];
    snprintf(author_text, sizeof(author_text), "%s\n%s", tr(STR_ABOUT_AUTHOR), tr(STR_ABOUT_AUTHOR_NAME));
    lv_label_set_text(author, author_text);
    lv_obj_set_style_text_font(author, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(author, lv_color_hex(GUI_COLOR_SHADES[10]), 0);
    lv_obj_set_style_text_align(author, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(author);

    // GitHub link
    lv_obj_t* github = lv_label_create(parent);
    char github_text[128];
    snprintf(github_text, sizeof(github_text), "%s\n%s", tr(STR_ABOUT_GITHUB), tr(STR_ABOUT_GITHUB_URL));
    lv_label_set_text(github, github_text);
    lv_obj_set_style_text_font(github, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(github, lv_color_hex(GUI_COLOR_MONO[0]), 0);
    lv_obj_set_style_text_align(github, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(github, author, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    // Version
    lv_obj_t* version = lv_label_create(parent);
    char version_text[64];
    snprintf(version_text, sizeof(version_text), "%s %s", tr(STR_ABOUT_VERSION), tr(STR_ABOUT_VERSION_NUM));
    lv_label_set_text(version, version_text);
    lv_obj_set_style_text_font(version, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(version, lv_color_hex(GUI_COLOR_SHADES[10]), 0);
    lv_obj_align_to(version, github, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
}
