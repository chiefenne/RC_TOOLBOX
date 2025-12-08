// gui/gui.cpp – Page management with registry-based lifecycle
#include "gui/gui.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"
#include "gui/version.h"
#include "gui/config/settings.h"
#include "style_utils.h"
#include "gui/pages/page_splash.h"
#include "gui/pages/page_home.h"
#include "gui/pages/page_servo.h"
#include "gui/pages/page_lipo.h"
#include "gui/pages/page_cg_scale.h"
#include "gui/pages/page_deflection.h"
#include "gui/pages/page_angle.h"
#include "gui/pages/page_settings.h"
#include "gui/pages/page_about.h"

// ============================================================================
// Page Registry - Uniform lifecycle for all pages
// ============================================================================

// Page entry with lifecycle function pointers (nullptr = no-op)
struct PageEntry {
    StringId       title_id;                    // Header title string ID
    void         (*create)(lv_obj_t* parent);   // Required: build page UI
    void         (*destroy)();                  // Optional: cleanup timers/state
    bool         (*is_busy)();                  // Optional: block navigation if true
    void         (*stop)();                     // Optional: graceful stop before destroy
    const char*  (*subtitle)();                 // Optional: header subtitle (e.g., protocol)
};

// Get servo protocol name for header display
static const char* get_servo_protocol_name() {
    static const char* names[] = {
        "Standard", "Extended", "Sanwa", "Futaba", "Fast", "Custom"
    };
    if (g_settings.servo_protocol < SERVO_PROTOCOL_COUNT) {
        return names[g_settings.servo_protocol];
    }
    return "";
}

// Page registry - must match GuiPage enum order
static const PageEntry PAGE_REGISTRY[PAGE_COUNT] = {
    // PAGE_HOME
    { STR_PAGE_HOME,       page_home_create,       nullptr,              nullptr,                 nullptr,           nullptr },
    // PAGE_SERVO
    { STR_PAGE_SERVO,      page_servo_create,      page_servo_destroy,   page_servo_is_running,   page_servo_stop,   get_servo_protocol_name },
    // PAGE_LIPO
    { STR_PAGE_LIPO,       page_lipo_create,       nullptr,              nullptr,                 nullptr,           nullptr },
    // PAGE_CG_SCALE
    { STR_PAGE_CG_SCALE,   page_cg_scale_create,   nullptr,              nullptr,                 nullptr,           nullptr },
    // PAGE_DEFLECTION
    { STR_PAGE_DEFLECTION, page_deflection_create, nullptr,              nullptr,                 nullptr,           nullptr },
    // PAGE_ANGLE
    { STR_PAGE_ANGLE,      page_angle_create,      nullptr,              nullptr,                 nullptr,           nullptr },
    // PAGE_SETTINGS
    { STR_PAGE_SETTINGS,   page_settings_create,   page_settings_destroy, nullptr,                nullptr,           nullptr },
    // PAGE_ABOUT
    { STR_PAGE_ABOUT,      page_about_create,      nullptr,              nullptr,                 nullptr,           nullptr },
};

// ============================================================================
// UI State
// ============================================================================

static lv_obj_t *header;
static lv_obj_t *header_title;
static lv_obj_t *header_subtitle;
static lv_obj_t *footer;
static lv_obj_t *content;
static lv_obj_t *btn_home;
static lv_obj_t *btn_prev;
static lv_obj_t *btn_next;
static lv_obj_t *btn_settings;
static lv_obj_t *splash_footer_left;
static lv_obj_t *splash_footer_right;

static GuiPage active_page = PAGE_COUNT; // sentinel so first gui_set_page runs
static bool splash_shown = false;
static BgColorPreset active_bg_color = BG_COLOR_LIGHT_GRAY;

// Check if current page is busy (blocks navigation)
static bool gui_page_is_busy() {
    if (active_page < PAGE_COUNT && PAGE_REGISTRY[active_page].is_busy) {
        return PAGE_REGISTRY[active_page].is_busy();
    }
    return false;
}

// Layout
static const lv_coord_t HEADER_HEIGHT = 36;
static const lv_coord_t FOOTER_HEIGHT = 32;

// Colors from color_palette.h
static const lv_color_t COLOR_HEADER       = lv_color_hex(GUI_COLOR_MONO[0]);      // 0x1C5C8C
static const lv_color_t COLOR_FOOTER       = lv_color_hex(GUI_COLOR_GRAYS[6]);     // 0xBCBCBC
static const lv_color_t COLOR_SURFACE      = lv_color_hex(GUI_COLOR_GRAYS[8]);     // 0xD0D0D0
static const lv_color_t COLOR_TAB_ACTIVE   = lv_color_hex(GUI_COLOR_TRIAD[1]);     // 0x86CC29
static const lv_color_t COLOR_TAB_INACTIVE = lv_color_hex(GUI_COLOR_GRAYS[8]);     // 0xD0D0D0
static const lv_color_t COLOR_TEXT_DARK    = lv_color_hex(GUI_COLOR_SHADES[7]);    // 0x0C283D
static const lv_color_t COLOR_TEXT_GRAY    = lv_color_hex(GUI_COLOR_GRAYS[0]);     // 0x808080
static const lv_color_t COLOR_TEXT_BLACK   = lv_color_hex(GUI_COLOR_SHADES[10]);   // 0x000000
static const lv_color_t COLOR_TEXT_WHITE   = lv_color_hex(GUI_COLOR_TINTS[10]);    // 0xFFFFFF

static void btn_home_event_cb(lv_event_t *e);
static void btn_prev_event_cb(lv_event_t *e);
static void btn_next_event_cb(lv_event_t *e);
static void btn_settings_event_cb(lv_event_t *e);
static void splash_timer_cb(lv_timer_t *timer);
static void create_nav_buttons();
static void create_splash_footer();

void gui_init()
{
    // Load saved settings
    settings_init();
    settings_load();

    // Apply loaded settings
    lang_set((Language)g_settings.language);
    active_bg_color = (BgColorPreset)g_settings.bg_color;

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, COLOR_SURFACE, 0);

    // Header
    header = lv_obj_create(scr);
    lv_obj_set_size(header, LV_PCT(100), HEADER_HEIGHT);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, COLOR_HEADER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(header, 0, 0); // Square corners

    header_title = lv_label_create(header);
    lv_label_set_text(header_title, APP_TITLE);
    lv_obj_set_style_text_color(header_title, lv_color_white(), 0);
    lv_obj_set_style_text_opa(header_title, LV_OPA_COVER, 0);
    lv_obj_set_style_text_font(header_title, FONT_HEADER, 0);
    lv_obj_center(header_title);

    // Header subtitle (right-aligned, smaller font - for protocol indicator etc.)
    header_subtitle = lv_label_create(header);
    lv_label_set_text(header_subtitle, "");
    lv_obj_set_style_text_color(header_subtitle, lv_color_hex(0xCCCCCC), 0);  // Light gray
    lv_obj_set_style_text_font(header_subtitle, FONT_DEFAULT, 0);  // Smaller font
    lv_obj_align(header_subtitle, LV_ALIGN_RIGHT_MID, -10, 0);

    // Content area - fixed height between header and footer
    lv_coord_t screen_height = lv_display_get_vertical_resolution(NULL);
    lv_coord_t content_height = screen_height - HEADER_HEIGHT - FOOTER_HEIGHT;

    content = lv_obj_create(scr);
    lv_obj_set_size(content, LV_PCT(100), content_height);
    lv_obj_set_pos(content, 0, HEADER_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(GUI_COLOR_BG[0]), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);  // No padding - let pages control their layout

    // Footer
    footer = lv_obj_create(scr);
    lv_obj_set_size(footer, LV_PCT(100), FOOTER_HEIGHT);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(footer, COLOR_FOOTER, 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_set_scrollbar_mode(footer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(footer, 0, 0);

    // Show splash first with splash footer
    create_splash_footer();
    page_splash_create(content);

    // Timer to switch to home after 6 seconds (3s logo + 3s text)
    lv_timer_create(splash_timer_cb, 6000, nullptr);
}

static void create_splash_footer()
{
    lv_obj_clean(footer);

    splash_footer_left = lv_label_create(footer);
    lv_label_set_text(splash_footer_left, "MHB Electronics");
    lv_obj_set_style_text_font(splash_footer_left, FONT_FOOTER, 0);
    lv_obj_set_style_text_color(splash_footer_left, COLOR_TEXT_BLACK, 0);
    lv_obj_align(splash_footer_left, LV_ALIGN_LEFT_MID, 10, 0);

    splash_footer_right = lv_label_create(footer);
    lv_label_set_text(splash_footer_right, SYM_COPYWRIGHT " 2025");  // Adjacent string literals are concatenated
    lv_obj_set_style_text_font(splash_footer_right, FONT_FOOTER, 0);
    lv_obj_set_style_text_color(splash_footer_right, COLOR_TEXT_BLACK, 0);
    lv_obj_align(splash_footer_right, LV_ALIGN_RIGHT_MID, -10, 0);
}

static void create_nav_buttons()
{
    lv_obj_clean(footer);
    lv_obj_set_flex_flow(footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footer, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // HOME button (text)
    btn_home = lv_button_create(footer);
    lv_obj_set_style_bg_color(btn_home, COLOR_TAB_ACTIVE, 0);
    lv_obj_set_style_border_width(btn_home, 2, 0);
    lv_obj_set_style_border_color(btn_home, COLOR_TEXT_GRAY, 0);
    lv_obj_t *lbl_home = lv_label_create(btn_home);
    lv_label_set_text(lbl_home, tr(STR_BTN_HOME));
    lv_obj_set_style_text_font(lbl_home, FONT_FOOTER, 0);
    lv_obj_set_style_text_color(lbl_home, lv_color_black(), 0);
    lv_obj_set_style_text_opa(lbl_home, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(btn_home, btn_home_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_set_style_pad_left(btn_home, 15, 0);
    lv_obj_set_style_pad_right(btn_home, 15, 0);
    lv_obj_set_style_pad_top(btn_home, 0, 0);
    lv_obj_set_style_pad_bottom(btn_home, 0, 0);

    // Previous button (◄ symbol)
    btn_prev = lv_button_create(footer);
    lv_obj_set_size(btn_prev, 50, 24);  // Fixed size
    lv_obj_set_style_bg_color(btn_prev, COLOR_TAB_INACTIVE, 0);
    lv_obj_set_style_border_width(btn_prev, 2, 0);
    lv_obj_set_style_border_color(btn_prev, COLOR_TEXT_GRAY, 0);
    lv_obj_t *lbl_prev = lv_label_create(btn_prev);
    lv_label_set_text(lbl_prev, SYM_LEFT);
    lv_obj_set_style_text_font(lbl_prev, FONT_FOOTER, 0);
    lv_obj_set_style_text_color(lbl_prev, lv_color_black(), 0);
    lv_obj_set_style_text_opa(lbl_prev, LV_OPA_COVER, 0);
    lv_obj_center(lbl_prev);  // Center the symbol in the button
    lv_obj_add_event_cb(btn_prev, btn_prev_event_cb, LV_EVENT_CLICKED, nullptr);

    // Next button (► symbol)
    btn_next = lv_button_create(footer);
    lv_obj_set_size(btn_next, 50, 24);  // Fixed size
    lv_obj_set_style_bg_color(btn_next, COLOR_TAB_INACTIVE, 0);
    lv_obj_set_style_border_width(btn_next, 2, 0);
    lv_obj_set_style_border_color(btn_next, COLOR_TEXT_GRAY, 0);
    lv_obj_t *lbl_next = lv_label_create(btn_next);
    lv_label_set_text(lbl_next, SYM_RIGHT);
    lv_obj_set_style_text_font(lbl_next, FONT_FOOTER, 0);
    lv_obj_set_style_text_color(lbl_next, lv_color_black(), 0);
    lv_obj_set_style_text_opa(lbl_next, LV_OPA_COVER, 0);
    lv_obj_center(lbl_next);  // Center the symbol in the button
    lv_obj_add_event_cb(btn_next, btn_next_event_cb, LV_EVENT_CLICKED, nullptr);

    // Settings button (⚙ symbol)
    btn_settings = lv_button_create(footer);
    lv_obj_set_size(btn_settings, 50, 24);  // Fixed size to match other buttons
    lv_obj_set_style_bg_color(btn_settings, COLOR_TAB_INACTIVE, 0);
    lv_obj_set_style_border_width(btn_settings, 2, 0);
    lv_obj_set_style_border_color(btn_settings, COLOR_TEXT_GRAY, 0);
    lv_obj_t *lbl_settings = lv_label_create(btn_settings);
    lv_label_set_text(lbl_settings, SYM_SETTINGS);
    lv_obj_set_style_text_font(lbl_settings, FONT_SYMBOLS, 0);  // Use STIXTwoMath for gear symbol
    lv_obj_set_style_text_color(lbl_settings, lv_color_black(), 0);
    lv_obj_set_style_text_opa(lbl_settings, LV_OPA_COVER, 0);
    lv_obj_center(lbl_settings);  // Center the symbol in the button
    lv_obj_add_event_cb(btn_settings, btn_settings_event_cb, LV_EVENT_CLICKED, nullptr);
}

static void splash_timer_cb(lv_timer_t *timer)
{
    lv_timer_delete(timer);
    page_splash_cleanup();  // Clean up splash phase timer before destroying page
    splash_shown = true;
    create_nav_buttons();
    gui_set_page(PAGE_HOME);
}

void gui_set_page(GuiPage p)
{
    if (p >= PAGE_COUNT) return;

    // Clean up previous page using registry
    if (active_page < PAGE_COUNT) {
        const PageEntry& prev = PAGE_REGISTRY[active_page];
        if (prev.stop) prev.stop();       // Graceful stop first
        if (prev.destroy) prev.destroy(); // Then cleanup
    }

    active_page = p;
    const PageEntry& curr = PAGE_REGISTRY[p];

    // Set header title and optional subtitle
    lv_label_set_text(header_title, tr(curr.title_id));
    if (curr.subtitle) {
        lv_label_set_text(header_subtitle, curr.subtitle());
    } else {
        lv_label_set_text(header_subtitle, "");
    }

    // Clean content and reset its layout state
    lv_obj_clean(content);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_style_pad_row(content, 0, 0);
    lv_obj_set_style_pad_column(content, 0, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);  // Default, pages can override
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to(content, 0, 0, LV_ANIM_OFF);  // Reset scroll position

    // Create new page
    curr.create(content);
}

static void btn_home_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if (gui_page_is_busy()) return;  // Block while page is busy
    gui_set_page(PAGE_HOME);
}

static void btn_prev_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if (gui_page_is_busy()) return;  // Block while page is busy
    // Navigate: HOME -> SETTINGS -> DATA -> HOME (skip splash)
    int next_page = (int)active_page - 1;
    if (next_page < PAGE_HOME) {
        next_page = PAGE_COUNT - 1;
    }
    gui_set_page((GuiPage)next_page);
}

static void btn_next_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if (gui_page_is_busy()) return;  // Block while page is busy
    // Navigate: HOME -> DATA -> SETTINGS -> HOME (skip splash)
    int next_page = (int)active_page + 1;
    if (next_page >= PAGE_COUNT) {
        next_page = PAGE_HOME;
    }
    gui_set_page((GuiPage)next_page);
}

static void btn_settings_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if (gui_page_is_busy()) return;  // Block while page is busy
    gui_set_page(PAGE_SETTINGS);
}

void gui_set_bg_color(BgColorPreset preset)
{
    if (preset >= BG_COLOR_COUNT) return;
    active_bg_color = preset;
    lv_color_t color = lv_color_hex(GUI_COLOR_BG[preset]);
    lv_obj_set_style_bg_color(content, color, 0);
}

BgColorPreset gui_get_bg_color()
{
    return active_bg_color;
}

GuiPage gui_get_current_page()
{
    return active_page;
}
