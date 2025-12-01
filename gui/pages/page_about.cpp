#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"

void page_about_create(lv_obj_t* parent) {
    // Author
    lv_obj_t* author = lv_label_create(parent);
    lv_label_set_text(author, "Author:\nDipl.-Ing. Andreas Ennemoser");
    lv_obj_set_style_text_font(author, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(author, lv_color_hex(GUI_COLOR_SHADES[7]), 0);
    lv_obj_set_style_text_align(author, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(author);

    // GitHub link
    lv_obj_t* github = lv_label_create(parent);
    lv_label_set_text(github, "GitHub:\ngithub.com/chiefenne/RC_TOOLBOX");
    lv_obj_set_style_text_font(github, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(github, lv_color_hex(GUI_COLOR_MONO[1]), 0);
    lv_obj_set_style_text_align(github, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(github, author, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    // Version
    lv_obj_t* version = lv_label_create(parent);
    lv_label_set_text(version, "Version: 0.1");
    lv_obj_set_style_text_font(version, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(version, lv_color_hex(GUI_COLOR_GRAYS[2]), 0);
    lv_obj_align_to(version, github, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
}
