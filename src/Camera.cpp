#include "Camera.h"

#include "Constants.h"
#include "MathUtil.h"

#include <cstdlib>

namespace zg {

void Camera::follow(float focus_x, float mouse_screen_x, bool has_mouse, float dt)
{
    const float mouse_offset = has_mouse
        ? (mouse_screen_x - kLogicalWidth * 0.5f) * kCameraLookAheadStrength
        : 0.0f;
    const float desired = focus_x - kLogicalWidth * 0.5f + mouse_offset;
    const float max_x = kWorldWidth - kLogicalWidth;
    target_x = clamp_float(desired, 0.0f, max_x);
    const float blend = clamp_float(kCameraLookAheadResponsiveness * dt, 0.0f, 1.0f);
    x += (target_x - x) * blend;
    x = clamp_float(x, 0.0f, max_x);
}

void Camera::update(float dt)
{
    const float zoom_blend = clamp_float(kCameraZoomResponsiveness * dt, 0.0f, 1.0f);
    zoom += (target_zoom - zoom) * zoom_blend;

    if (shake_time > 0.0f) {
        shake_time -= dt;
        const float remaining = shake_time > 0.0f ? shake_time : 0.0f;
        const float falloff = shake_duration > 0.0f ? remaining / shake_duration : 0.0f;
        const float magnitude = shake_magnitude * falloff;
        shake_x = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * magnitude;
        shake_y = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f) * magnitude;
    } else {
        shake_time = 0.0f;
        shake_duration = 0.0f;
        shake_x = 0.0f;
        shake_y = 0.0f;
        shake_magnitude = 0.0f;
    }
}

void Camera::add_shake(float duration, float magnitude)
{
    if (duration > shake_time) {
        shake_time = duration;
        shake_duration = duration;
    }
    if (magnitude > shake_magnitude) {
        shake_magnitude = magnitude;
    }
}

void Camera::set_zoom(float zoom_value)
{
    zoom = zoom_value;
    target_zoom = zoom_value;
}

float Camera::render_x() const
{
    return x + shake_x;
}

float Camera::render_y() const
{
    return shake_y;
}

float Camera::render_zoom() const
{
    return zoom;
}

} // namespace zg
