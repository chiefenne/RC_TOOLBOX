// simulator/input_sim.cpp - Simulator input handling (keyboard → encoder events)
// Maps keyboard to LVGL encoder input device

#include "gui/input.h"
#include <SDL2/SDL.h>

// =============================================================================
// Keyboard State
// =============================================================================
static bool shift_held = false;
static uint32_t last_press_time = 0;
static int click_count = 0;
static constexpr uint32_t DOUBLE_CLICK_MS = 300;

// =============================================================================
// SDL Event Handler (called from simulator/main.cpp)
// =============================================================================

void input_handle_sdl_event(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN && !e.key.repeat) {
        uint32_t now = SDL_GetTicks();

        switch (e.key.keysym.sym) {
            // Encoder rotation simulation
            case SDLK_LEFT:
            case SDLK_DOWN:
                // Shift = faster rotation
                input_feed_encoder(shift_held ? -5 : -1);
                break;

            case SDLK_RIGHT:
            case SDLK_UP:
                input_feed_encoder(shift_held ? 5 : 1);
                break;

            // Encoder button simulation
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                // Detect double/triple-click
                if (now - last_press_time < DOUBLE_CLICK_MS) {
                    click_count++;
                    if (click_count >= 3) {
                        input_feed_button(INPUT_ENC_TRIPLE_CLICK);
                        click_count = 0;
                    } else if (click_count >= 2) {
                        input_feed_button(INPUT_ENC_DOUBLE_CLICK);
                        // Don't reset - allow triple-click to follow
                    }
                } else {
                    click_count = 1;
                    input_feed_button(INPUT_ENC_PRESS);
                }
                last_press_time = now;
                break;

            // Long press simulation (hold L key)
            case SDLK_l:
                input_feed_button(INPUT_ENC_LONG_PRESS);
                break;

            // Triple-click simulation (T key)
            case SDLK_t:
                input_feed_button(INPUT_ENC_TRIPLE_CLICK);
                break;

            // Escape = double-click (exit edit mode)
            case SDLK_ESCAPE:
                input_feed_button(INPUT_ENC_DOUBLE_CLICK);
                break;

            // Shift modifier
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                shift_held = true;
                break;

            default:
                break;
        }
    }
    else if (e.type == SDL_KEYUP) {
        switch (e.key.keysym.sym) {
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                shift_held = false;
                break;
            default:
                break;
        }
    }
}

// =============================================================================
// Platform stubs (input_init and input_poll handled by gui/input.cpp)
// =============================================================================

// Simulator doesn't need separate init - gui/input.cpp handles LVGL setup
// input_poll() is also in gui/input.cpp (returns INPUT_NONE, LVGL handles events)
