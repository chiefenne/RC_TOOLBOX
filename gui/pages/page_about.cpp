#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include "gui/version.h"
#include "gui/images/images.h"
#include "gui/input.h"
#include "gui/gui.h"
#include <cstdio>

// =============================================================================
// Focus Order Configuration
// =============================================================================
enum FocusOrder {
    FO_BTN_HOME     = 0,
    FO_BTN_PREV     = 1,
    FO_BTN_NEXT     = 2,
    FO_BTN_SETTINGS = 3,
};

// Focus group builder for this page
static FocusOrderBuilder focus_builder;

void page_about_create(lv_obj_t* parent) {
    // Initialize focus builder
    focus_builder.init();

    // Record this page in navigation history
    input_push_page(PAGE_ABOUT);

    // Use flex layout for vertical centering
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(parent, 0, 0);  // No default gap, control per-element

    // Top row: Logo + App name side by side, vertically centered
    lv_obj_t* top_row = lv_obj_create(parent);
    lv_obj_remove_style_all(top_row);
    lv_obj_set_size(top_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(top_row, 16, 0);  // Gap between logo and text box
    lv_obj_set_style_pad_bottom(top_row, 12, 0);   // Gap after top row

    // Logo: scale=180 → 70%, size = 150*180/256=105, 140*180/256=98
    lv_obj_t* logo = lv_image_create(top_row);
    lv_image_set_src(logo, &RC_Toolbox_Logo_150px);
    lv_image_set_scale(logo, 180);  // 70% of original
    lv_obj_set_size(logo, 105, 98);
    lv_image_set_inner_align(logo, LV_IMAGE_ALIGN_CENTER);

    // App name + version box (vertically centered with logo)
    lv_obj_t* app_box = lv_obj_create(top_row);
    lv_obj_remove_style_all(app_box);
    lv_obj_set_size(app_box, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(app_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(app_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // App name
    lv_obj_t* app_name = lv_label_create(app_box);
    lv_label_set_text(app_name, APP_TITLE);
    lv_obj_set_style_text_font(app_name, &arial_18, 0);
    lv_obj_set_style_text_color(app_name, lv_color_hex(GUI_COLOR_MONO[0]), 0);

    // Version
    lv_obj_t* app_ver = lv_label_create(app_box);
    char ver_text[32];
    snprintf(ver_text, sizeof(ver_text), "v%s", APP_VERSION);
    lv_label_set_text(app_ver, ver_text);
    lv_obj_set_style_text_font(app_ver, &arial_14, 0);
    lv_obj_set_style_text_color(app_ver, lv_color_hex(GUI_COLOR_SHADES[3]), 0);

    // Author (in flex column)
    lv_obj_t* author = lv_label_create(parent);
    char author_text[128];
    snprintf(author_text, sizeof(author_text), "%s %s", tr(STR_ABOUT_AUTHOR), APP_AUTHOR_NAME);
    lv_label_set_text(author, author_text);
    lv_obj_set_style_text_font(author, FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(author, lv_color_hex(GUI_COLOR_SHADES[5]), 0);
    lv_obj_set_style_pad_bottom(author, 4, 0);  // Gap after author

    // GitHub link (in flex column)
    lv_obj_t* github = lv_label_create(parent);
    char github_text[128];
    snprintf(github_text, sizeof(github_text), "Source code: %s", APP_GITHUB_URL);
    lv_label_set_text(github, github_text);
    lv_obj_set_style_text_font(github, &arial_12, 0);
    lv_obj_set_style_text_color(github, lv_color_hex(GUI_COLOR_MONO[1]), 0);
    lv_obj_set_style_pad_bottom(github, 4, 0);  // Gap after GitHub

    // LVGL version (in flex column, smaller, at bottom)
    lv_obj_t* club_label = lv_label_create(parent);
    char club_text[48];
    snprintf(club_text, sizeof(club_text), "MHB Electronics %s 2026", SYM_COPYWRIGHT);
    lv_label_set_text(club_label, club_text);
    lv_obj_set_style_text_font(club_label, &arial_12, 0);
    lv_obj_set_style_text_color(club_label, lv_color_hex(GUI_COLOR_GRAYS[0]), 0);

    // Add footer buttons to focus order
    focus_builder.add(gui_get_btn_home(), FO_BTN_HOME);
    focus_builder.add(gui_get_btn_prev(), FO_BTN_PREV);
    focus_builder.add(gui_get_btn_next(), FO_BTN_NEXT);
    focus_builder.add(gui_get_btn_settings(), FO_BTN_SETTINGS);

    // Finalize focus builder
    focus_builder.finalize();
}

void page_about_destroy() {
    focus_builder.destroy();
}
