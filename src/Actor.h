#pragma once

namespace zg {

struct Actor {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float hp = 100.0f;
    bool active = true;
    bool airborne = false;

    Actor() = default;
    Actor(float x_in, float y_in, float hp_in)
        : x(x_in), y(y_in), hp(hp_in)
    {
    }

    void apply_impulse(float impulse_x, float impulse_y)
    {
        vx = impulse_x;
        vy = impulse_y;
        airborne = true;
    }

    void clamp_hp_zero()
    {
        if (hp < 0.0f) {
            hp = 0.0f;
        }
    }
};

} // namespace zg
