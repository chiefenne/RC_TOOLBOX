// gui/page_base.h - Base class for all pages
#pragma once

#include "lvgl.h"
#include "gui/fonts.h"
#include "gui/color_palette.h"
#include "gui/lang.h"

// Common button colors used across pages
namespace PageColors {
    static const lv_color_t BTN_ACTIVE   = lv_color_hex(GUI_COLOR_TRIAD[1]);    // Green
    static const lv_color_t BTN_INACTIVE = lv_color_hex(GUI_COLOR_GRAYS[6]);    // Gray
    static const lv_color_t BTN_STOP     = lv_color_hex(0xCC3333);              // Red
    static const lv_color_t BTN_PRIMARY  = lv_color_hex(GUI_COLOR_MONO[0]);     // Blue
    static const lv_color_t TEXT_DARK    = lv_color_hex(GUI_COLOR_SHADES[7]);   // Dark text
    static const lv_color_t TEXT_PRIMARY = lv_color_hex(GUI_COLOR_MONO[0]);     // Primary text
}

/**
 * @brief Abstract base class for all pages
 *
 * Pages should inherit from this class and implement:
 * - on_create(): Build the UI
 * - on_destroy(): Clean up timers, resources (optional)
 * - on_update(): Periodic updates if needed (optional)
 */
class PageBase {
public:
    virtual ~PageBase() = default;

    /**
     * @brief Create the page UI inside the given parent container
     * @param parent The LVGL container to build the page in
     */
    void create(lv_obj_t* parent) {
        m_parent = parent;
        m_active = true;
        on_create(parent);
    }

    /**
     * @brief Destroy the page and clean up resources
     * Called before the page is switched away from
     */
    void destroy() {
        m_active = false;
        on_destroy();
        m_parent = nullptr;
    }

    /**
     * @brief Check if this page is currently active
     */
    bool is_active() const { return m_active; }

    /**
     * @brief Check if the page has background work that blocks navigation
     * Override in subclasses that have running timers/processes
     */
    virtual bool is_busy() const { return false; }

    /**
     * @brief Stop any running processes (called before navigation)
     * Override in subclasses that need to stop timers
     */
    virtual void stop() {}

protected:
    // Pure virtual - must be implemented by subclasses
    virtual void on_create(lv_obj_t* parent) = 0;

    // Optional overrides
    virtual void on_destroy() {}
    virtual void on_update() {}

    // Helper: Create a transparent flex row container
    lv_obj_t* create_row(lv_obj_t* parent, lv_coord_t width_pct = 90, lv_coord_t height = 35) {
        lv_obj_t* row = lv_obj_create(parent);
        lv_obj_set_size(row, LV_PCT(width_pct), height);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(row, 10, 0);
        return row;
    }

    // Helper: Create a button with centered label
    lv_obj_t* create_button(lv_obj_t* parent, const char* text, lv_coord_t width, lv_coord_t height,
                            lv_color_t bg_color, lv_event_cb_t callback, void* user_data = nullptr) {
        lv_obj_t* btn = lv_button_create(parent);
        lv_obj_set_size(btn, width, height);
        lv_obj_set_style_bg_color(btn, bg_color, 0);

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, text);
        lv_obj_set_style_text_font(lbl, FONT_BUTTON_MD, 0);
        lv_obj_center(lbl);

        if (callback) {
            lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, user_data);
        }
        return btn;
    }

    // Helper: Get the label child of a button (for updating text)
    static lv_obj_t* get_button_label(lv_obj_t* btn) {
        return lv_obj_get_child(btn, 0);
    }

    // Helper: Update button label text
    static void set_button_text(lv_obj_t* btn, const char* text) {
        lv_obj_t* lbl = get_button_label(btn);
        if (lbl) lv_label_set_text(lbl, text);
    }

    // Helper: Clamp a value to a range
    static int clamp(int value, int min_val, int max_val) {
        if (value < min_val) return min_val;
        if (value > max_val) return max_val;
        return value;
    }

    lv_obj_t* m_parent = nullptr;
    bool m_active = false;
};
