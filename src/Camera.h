#pragma once

namespace zg {

struct Camera {
    float x = 200.0f;
    float target_x = 200.0f;
    float zoom = 1.0f;
    float target_zoom = 1.0f;
    float shake_x = 0.0f;
    float shake_y = 0.0f;
    float shake_time = 0.0f;
    float shake_duration = 0.0f;
    float shake_magnitude = 0.0f;

    void follow(float focus_x, float mouse_screen_x, bool has_mouse, float dt);
    void update(float dt);
    void add_shake(float duration, float magnitude);
    void set_zoom(float zoom_value);
    float render_x() const;
    float render_y() const;
    float render_zoom() const;
};

} // namespace zg
