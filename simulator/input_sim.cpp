// simulator/input_sim.cpp - Simulator input handling (keyboard → input events)

#include "gui/input.h"
#include <SDL2/SDL.h>

// Global encoder delta (accessed by gui/input.cpp)
int g_encoder_delta = 0;

// Pending event queue (simple single-event buffer)
static InputEvent pending_event = INPUT_NONE;

void input_init() {
    // Nothing special needed for simulator
    g_encoder_delta = 0;
    pending_event = INPUT_NONE;
}

// Called from main loop to translate SDL events to InputEvents
void input_handle_sdl_event(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            // Encoder rotation simulation
            case SDLK_LEFT:
            case SDLK_DOWN:
                g_encoder_delta -= 10;
                pending_event = INPUT_ENC_CCW;
                break;
            case SDLK_RIGHT:
            case SDLK_UP:
                g_encoder_delta += 10;
                pending_event = INPUT_ENC_CW;
                break;

            // Encoder press
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                pending_event = INPUT_ENC_PRESS;
                break;

            // Button shortcuts
            case SDLK_h:
                pending_event = INPUT_BTN_HOME;
                break;
            case SDLK_ESCAPE:
            case SDLK_BACKSPACE:
                pending_event = INPUT_BTN_BACK;
                break;
            case SDLK_SPACE:
                pending_event = INPUT_BTN_ACTION;
                break;

            default:
                break;
        }
    }
}

InputEvent input_poll() {
    InputEvent ev = pending_event;
    pending_event = INPUT_NONE;
    return ev;
}
