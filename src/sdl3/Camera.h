#pragma once

namespace zg {

struct Camera {
    float x = 200.0f;
    float shake_x = 0.0f;
    float shake_y = 0.0f;
    float shake_time = 0.0f;
    float shake_duration = 0.0f;
    float shake_magnitude = 0.0f;

    void follow(float target_x);
    void update(float dt);
    void add_shake(float duration, float magnitude);
    float render_x() const;
    float render_y() const;
};

} // namespace zg
