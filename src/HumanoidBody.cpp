#include "HumanoidBody.h"

#include "CollisionMap.h"
#include "MathUtil.h"

#include <cmath>

namespace zg {

HumanoidBody::HumanoidBody(float width, float height, int probe_left, int probe_right, int foot_probe_start, int foot_probe_end)
    : width_(width),
      height_(height),
      probe_left_(probe_left),
      probe_right_(probe_right),
      foot_probe_start_(foot_probe_start),
      foot_probe_end_(foot_probe_end)
{
}

float HumanoidBody::width() const
{
    return width_;
}

float HumanoidBody::height() const
{
    return height_;
}

bool HumanoidBody::overlaps_solid(const CollisionMap& collision_map, float x, float y) const
{
    for (int px = foot_probe_start_; px <= foot_probe_end_; ++px) {
        const int probe_x = static_cast<int>(std::round(x + px));
        for (int py = 0; py <= static_cast<int>(height_) - 1; ++py) {
            const int probe_y = static_cast<int>(std::round(y + py));
            if (collision_map.is_solid(probe_x, probe_y)) {
                return true;
            }
        }
    }
    return false;
}

bool HumanoidBody::has_floor_support(const CollisionMap& collision_map, float x, float y) const
{
    const int foot_y = static_cast<int>(std::round(y + height_ + 1.0f));
    for (int probe = foot_probe_start_; probe <= foot_probe_end_; ++probe) {
        const int probe_x = static_cast<int>(std::round(x + probe));
        if (collision_map.is_solid(probe_x, foot_y)) {
            return true;
        }
    }
    return false;
}

bool HumanoidBody::collides_head(const CollisionMap& collision_map, float x, float y) const
{
    const int head_y = static_cast<int>(std::round(y));
    for (int probe = foot_probe_start_; probe <= foot_probe_end_; ++probe) {
        const int probe_x = static_cast<int>(std::round(x + probe));
        if (collision_map.is_solid(probe_x, head_y)) {
            return true;
        }
    }
    return false;
}

bool HumanoidBody::collides_left(const CollisionMap& collision_map, float x, float y) const
{
    const int left_x = static_cast<int>(std::round(x));
    for (int probe = probe_left_; probe <= probe_right_; ++probe) {
        const int probe_y = static_cast<int>(std::round(y + probe));
        if (collision_map.is_solid(left_x, probe_y)) {
            return true;
        }
    }
    return false;
}

bool HumanoidBody::collides_right(const CollisionMap& collision_map, float x, float y) const
{
    const int right_x = static_cast<int>(std::round(x + width_));
    for (int probe = probe_left_; probe <= probe_right_; ++probe) {
        const int probe_y = static_cast<int>(std::round(y + probe));
        if (collision_map.is_solid(right_x, probe_y)) {
            return true;
        }
    }
    return false;
}

float HumanoidBody::snap_to_floor(const CollisionMap& collision_map, float x, float y, float world_height_limit) const
{
    float snapped_y = std::round(y);

    while (snapped_y > 0.0f && overlaps_solid(collision_map, x, snapped_y)) {
        snapped_y -= 1.0f;
    }
    while (snapped_y < world_height_limit && !has_floor_support(collision_map, x, snapped_y)) {
        snapped_y += 1.0f;
    }

    return snapped_y;
}

void HumanoidBody::move_horizontally(const CollisionMap& collision_map, float axis, float speed, float dt, float* x, float y, float world_width_limit) const
{
    float remaining = axis * speed * dt;
    while (std::abs(remaining) > 0.0f) {
        const float step = clamp_float(remaining, -1.0f, 1.0f);
        const float trial_x = clamp_float(*x + step, 0.0f, world_width_limit - width_);
        const bool blocked = step < 0.0f
            ? collides_left(collision_map, trial_x, y)
            : collides_right(collision_map, trial_x, y);
        if (blocked) {
            break;
        }
        *x = trial_x;
        remaining -= step;
    }
}

void HumanoidBody::move_by_velocity(const CollisionMap& collision_map, float dt, float* x, float* vx, float y, float world_width_limit) const
{
    float remaining = *vx * dt;
    while (std::abs(remaining) > 0.0f) {
        const float step = clamp_float(remaining, -1.0f, 1.0f);
        const float trial_x = clamp_float(*x + step, 0.0f, world_width_limit - width_);
        const bool blocked = step < 0.0f
            ? collides_left(collision_map, trial_x, y)
            : collides_right(collision_map, trial_x, y);
        if (blocked) {
            *vx = 0.0f;
            break;
        }
        *x = trial_x;
        remaining -= step;
    }
}

void HumanoidBody::move_vertically(
    const CollisionMap& collision_map,
    float gravity,
    float min_velocity,
    float max_velocity,
    float dt,
    float x,
    float* y,
    float* vy,
    bool* airborne,
    float world_height_limit) const
{
    *vy = clamp_float(*vy + gravity * dt, min_velocity, max_velocity);
    float remaining = *vy * dt;
    while (std::abs(remaining) > 0.0f) {
        const float step = clamp_float(remaining, -1.0f, 1.0f);
        const float trial_y = *y + step;

        if (step < 0.0f) {
            if (collides_head(collision_map, x, trial_y)) {
                *vy = 0.0f;
                break;
            }
        } else if (step > 0.0f) {
            if (has_floor_support(collision_map, x, trial_y)) {
                *y = snap_to_floor(collision_map, x, *y, world_height_limit);
                *vy = 0.0f;
                *airborne = false;
                break;
            }
        }

        *y = trial_y;
        remaining -= step;
    }
}

} // namespace zg
