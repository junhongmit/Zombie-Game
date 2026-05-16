#pragma once

struct SDL_Renderer;

namespace zg {

struct InputState {
    float move_axis = 0.0f;
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    bool quit = false;
    bool mouse_in_view = false;
    bool fire_down = false;

    void poll(SDL_Renderer* renderer);
};

} // namespace zg
