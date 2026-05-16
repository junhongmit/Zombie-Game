#include "Input.h"

#include <SDL3/SDL.h>

namespace zg {

void InputState::poll(SDL_Renderer* renderer)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            quit = true;
        } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            quit = true;
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

    float window_x = 0.0f;
    float window_y = 0.0f;
    const SDL_MouseButtonFlags buttons = SDL_GetMouseState(&window_x, &window_y);
    fire_down = (buttons & SDL_BUTTON_LMASK) != 0;

    float render_x = 0.0f;
    float render_y = 0.0f;
    mouse_in_view = SDL_RenderCoordinatesFromWindow(renderer, window_x, window_y, &render_x, &render_y);
    if (mouse_in_view) {
        mouse_x = render_x;
        mouse_y = render_y;
    }
}

} // namespace zg
