#pragma once

struct SDL_Renderer;

namespace zg {

struct InputState {
    float move_axis = 0.0f;
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    float ui_mouse_x = 0.0f;
    float ui_mouse_y = 0.0f;
    float wheel_x = 0.0f;
    float wheel_y = 0.0f;
    bool quit = false;
    bool back_pressed = false;
    bool mouse_in_view = false;
    bool ui_mouse_in_view = false;
    bool fire_down = false;
    bool fire_pressed = false;
    bool fire_released = false;
    bool confirm_pressed = false;
    bool jump_down = false;
    bool jump_pressed = false;
    bool stair_down = false;
    bool stair_pressed = false;
    bool reload_pressed = false;
    bool grenade_pressed = false;
    bool inventory_pressed = false;
    int switch_mode = -1;
    int switch_slot = -1;
    int cycle_weapon = 0;

    void poll(SDL_Renderer* renderer);

private:
    bool previous_fire_down_ = false;
    bool previous_back_down_ = false;
    bool previous_confirm_down_ = false;
    bool previous_jump_down_ = false;
    bool previous_stair_down_ = false;
    bool previous_reload_down_ = false;
    bool previous_grenade_down_ = false;
    bool previous_inventory_down_ = false;
};

} // namespace zg
