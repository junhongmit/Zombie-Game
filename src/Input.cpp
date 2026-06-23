#include "Input.h"
#include "Constants.h"
#include "Presentation.h"

#include <SDL3/SDL.h>

namespace zg {

void InputState::poll(SDL_Renderer* renderer)
{
    switch_slot = -1;
    switch_mode = -1;
    cycle_weapon = 0;
    wheel_x = 0.0f;
    wheel_y = 0.0f;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            quit = true;
        } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
            if (event.key.key >= SDLK_1 && event.key.key <= SDLK_5) {
                switch_slot = static_cast<int>(event.key.key - SDLK_1);
            } else if (event.key.key >= SDLK_F1 && event.key.key <= SDLK_F4) {
                switch_mode = static_cast<int>(event.key.key - SDLK_F1);
            }
        } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            wheel_x += event.wheel.x;
            wheel_y += event.wheel.y;
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
    const bool inventory_down = keys[SDL_SCANCODE_I] || keys[SDL_SCANCODE_TAB];
    inventory_pressed = inventory_down && !previous_inventory_down_;
    previous_inventory_down_ = inventory_down;
    const bool use_down = keys[SDL_SCANCODE_E];
    use_pressed = use_down && !previous_use_down_;
    previous_use_down_ = use_down;
    const bool drop_down = keys[SDL_SCANCODE_T];
    drop_pressed = drop_down && !previous_drop_down_;
    previous_drop_down_ = drop_down;
    const bool split_down = keys[SDL_SCANCODE_F];
    split_pressed = split_down && !previous_split_down_;
    previous_split_down_ = split_down;
    const bool confirm_down = keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_KP_ENTER];
    confirm_pressed = confirm_down && !previous_confirm_down_;
    previous_confirm_down_ = confirm_down;
    const bool back_down = keys[SDL_SCANCODE_ESCAPE];
    back_pressed = back_down && !previous_back_down_;
    previous_back_down_ = back_down;

    float window_x = 0.0f;
    float window_y = 0.0f;
    const SDL_MouseButtonFlags buttons = SDL_GetMouseState(&window_x, &window_y);
    fire_down = (buttons & SDL_BUTTON_LMASK) != 0;

    int output_width = 0;
    int output_height = 0;
    SDL_GetRenderOutputSize(renderer, &output_width, &output_height);
    const SDL_FRect presentation = compute_presentation_rect(output_width, output_height);
    const SDL_FRect ui_presentation = compute_aspect_rect(
        output_width,
        output_height,
        kInternalRenderWidth,
        kInternalRenderHeight);
    float render_x = 0.0f;
    float render_y = 0.0f;
    float ui_render_x = 0.0f;
    float ui_render_y = 0.0f;
    mouse_in_view = window_to_logical(window_x, window_y, presentation, &render_x, &render_y);
    ui_mouse_in_view = window_to_ui_logical(window_x, window_y, ui_presentation, &ui_render_x, &ui_render_y);
    fire_pressed = fire_down && !previous_fire_down_;
    fire_released = !fire_down && previous_fire_down_;
    previous_fire_down_ = fire_down;
    if (mouse_in_view) {
        mouse_x = render_x;
        mouse_y = render_y;
    }
    if (ui_mouse_in_view) {
        ui_mouse_x = ui_render_x;
        ui_mouse_y = ui_render_y;
    }
}

} // namespace zg
