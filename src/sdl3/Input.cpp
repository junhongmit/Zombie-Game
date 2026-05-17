#include "Input.h"

#include <SDL3/SDL.h>

namespace zg {

void InputState::poll(SDL_Renderer* renderer)
{
    switch_slot = -1;
    cycle_weapon = 0;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            quit = true;
        } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            quit = true;
        } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
            if (event.key.key >= SDLK_1 && event.key.key <= SDLK_5) {
                switch_slot = static_cast<int>(event.key.key - SDLK_1);
            }
        } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            if (event.wheel.y > 0) {
                cycle_weapon = -1;
            } else if (event.wheel.y < 0) {
                cycle_weapon = 1;
            }
        }
    }

    const bool* keys = SDL_GetKeyboardState(nullptr);
    move_axis = 0.0f;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
        move_axis -= 1.0f;
    }
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
        move_axis += 1.0f;
    }
    jump_down = keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_SPACE];
    jump_pressed = jump_down && !previous_jump_down_;
    previous_jump_down_ = jump_down;
    stair_down = keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN];
    stair_pressed = stair_down && !previous_stair_down_;
    previous_stair_down_ = stair_down;
    const bool reload_down = keys[SDL_SCANCODE_R];
    reload_pressed = reload_down && !previous_reload_down_;
    previous_reload_down_ = reload_down;
    const bool grenade_down = keys[SDL_SCANCODE_G];
    grenade_pressed = grenade_down && !previous_grenade_down_;
    previous_grenade_down_ = grenade_down;

    float window_x = 0.0f;
    float window_y = 0.0f;
    const SDL_MouseButtonFlags buttons = SDL_GetMouseState(&window_x, &window_y);
    fire_down = (buttons & SDL_BUTTON_LMASK) != 0;

    float render_x = 0.0f;
    float render_y = 0.0f;
    mouse_in_view = SDL_RenderCoordinatesFromWindow(renderer, window_x, window_y, &render_x, &render_y);
    fire_pressed = fire_down && !previous_fire_down_;
    previous_fire_down_ = fire_down;
    if (mouse_in_view) {
        mouse_x = render_x;
        mouse_y = render_y;
    }
}

} // namespace zg
