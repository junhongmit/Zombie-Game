#include "Player.h"

#include "CollisionMap.h"
#include "Constants.h"
#include "MathUtil.h"

#include <cmath>

namespace zg {

namespace {

int floor_index_from_y(float y)
{
    if (y <= 200.0f) {
        return 4;
    }
    if (y <= 280.0f) {
        return 3;
    }
    if (y <= 360.0f) {
        return 2;
    }
    return 1;
}

float floor_y_from_index(int floor)
{
    switch (floor) {
    case 4:
        return kFloor4Y;
    case 3:
        return kFloor3Y;
    case 2:
        return kFloor2Y;
    case 1:
    default:
        return kFloor1Y;
    }
}

bool body_overlaps_solid(const CollisionMap& collision_map, float x, float y)
{
    for (int px = 5; px <= 12; ++px) {
        const int probe_x = static_cast<int>(std::round(x + px));
        for (int py = 0; py <= 32; ++py) {
            const int probe_y = static_cast<int>(std::round(y + py));
            if (collision_map.is_solid(probe_x, probe_y)) {
                return true;
            }
        }
    }
    return false;
}

bool has_floor_support(const CollisionMap& collision_map, float x, float y)
{
    const int foot_y = static_cast<int>(std::round(y + kPlayerHeight + 1.0f));
    for (int probe = 5; probe <= 12; ++probe) {
        const int probe_x = static_cast<int>(std::round(x + probe));
        if (collision_map.is_solid(probe_x, foot_y)) {
            return true;
        }
    }
    return false;
}

float snap_to_floor(const CollisionMap& collision_map, float x, float y)
{
    float snapped_y = std::round(y);

    while (snapped_y > 0.0f && body_overlaps_solid(collision_map, x, snapped_y)) {
        snapped_y -= 1.0f;
    }
    while (snapped_y < static_cast<float>(kLogicalHeight) &&
           !has_floor_support(collision_map, x, snapped_y)) {
        snapped_y += 1.0f;
    }

    return snapped_y;
}

bool collides_head(const CollisionMap& collision_map, float x, float y)
{
    const int head_y = static_cast<int>(std::round(y));
    for (int probe = 5; probe <= 12; ++probe) {
        const int probe_x = static_cast<int>(std::round(x + probe));
        if (collision_map.is_solid(probe_x, head_y)) {
            return true;
        }
    }
    return false;
}

bool collides_left(const CollisionMap& collision_map, float x, float y)
{
    const int left_x = static_cast<int>(std::round(x));
    for (int probe = 1; probe <= 31; ++probe) {
        const int probe_y = static_cast<int>(std::round(y + probe));
        if (collision_map.is_solid(left_x, probe_y)) {
            return true;
        }
    }
    return false;
}

bool collides_right(const CollisionMap& collision_map, float x, float y)
{
    const int right_x = static_cast<int>(std::round(x + kPlayerWidth));
    for (int probe = 1; probe <= 31; ++probe) {
        const int probe_y = static_cast<int>(std::round(y + probe));
        if (collision_map.is_solid(right_x, probe_y)) {
            return true;
        }
    }
    return false;
}

void move_horizontally(const CollisionMap& collision_map, float axis, float dt, float* x, float y)
{
    float remaining = axis * kPlayerMoveSpeed * dt;
    while (std::abs(remaining) > 0.0f) {
        const float step = clamp_float(remaining, -1.0f, 1.0f);
        const float trial_x = clamp_float(*x + step, 0.0f, kWorldWidth - kPlayerWidth);
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

void move_vertically(const CollisionMap& collision_map, float x, float dt, float* y, float* vy, bool* airborne)
{
    *vy = clamp_float(*vy + kGravity * dt, -kPlayerJumpSpeed, kTerminalVelocity);
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
                *y = snap_to_floor(collision_map, x, *y);
                *vy = 0.0f;
                *airborne = false;
                break;
            }
        }

        *y = trial_y;
        remaining -= step;
    }
}

} // namespace

Player::Player()
    : y(kGroundY)
{
}

void Player::update(
    const CollisionMap& collision_map,
    float axis,
    bool jump_pressed,
    float aim_x,
    float aim_y,
    bool has_aim,
    float dt)
{
    if (!airborne && !has_floor_support(collision_map, x, y)) {
        airborne = true;
    }

    if (jump_pressed && !airborne) {
        airborne = true;
        vy = -kPlayerJumpSpeed;
        walk_frame = 0;
        walk_frame_distance = 0.0f;
    }

    const bool wants_move = axis != 0.0f;
    if (wants_move) {
        const float old_x = x;
        move_horizontally(collision_map, axis, dt, &x, y);
        facing_right = axis > 0.0f;

        if (!airborne) {
            walk_frame_distance += std::abs(x - old_x);
            while (walk_frame_distance >= kWalkPixelsPerFrame) {
                walk_frame_distance -= kWalkPixelsPerFrame;
                walk_frame = (walk_frame + 1) % 24;
            }
        }
    } else if (!airborne) {
        walk_frame = 0;
        walk_frame_distance = 0.0f;
    }

    if (airborne) {
        move_vertically(collision_map, x, dt, &y, &vy, &airborne);
        if (!airborne) {
            y = snap_to_floor(collision_map, x, y);
            vy = 0.0f;
        }
    }

    moving = wants_move && !airborne;

    if (has_aim) {
        aim_world_x = aim_x;
        aim_world_y = aim_y;
        const float pivot_x = x + 9.0f;
        const float pivot_y = y + 13.0f;
        aim_angle_radians = std::atan2(aim_world_y - pivot_y, aim_world_x - pivot_x);
        facing_right = aim_world_x >= pivot_x;
    }
}

bool Player::try_use_stairs(const CollisionMap& collision_map)
{
    if (airborne) {
        return false;
    }

    const int trigger_x = static_cast<int>(std::round(x + 8.0f));
    const int trigger_y = static_cast<int>(std::round(y + 16.0f));
    const TriggerType trigger = collision_map.trigger_at(trigger_x, trigger_y);
    if (trigger == TriggerType::None) {
        return false;
    }

    const int floor = floor_index_from_y(y);
    int destination_floor = floor;
    switch (trigger) {
    case TriggerType::StairRed:
        if (floor == 1) {
            destination_floor = 2;
        } else if (floor == 2) {
            destination_floor = 1;
        }
        break;
    case TriggerType::StairGreen:
        if (floor == 2) {
            destination_floor = 3;
        } else if (floor == 3) {
            destination_floor = 2;
        }
        break;
    case TriggerType::StairBlue:
        if (floor == 3) {
            destination_floor = 4;
        } else if (floor == 4) {
            destination_floor = 3;
        }
        break;
    case TriggerType::None:
        break;
    }

    if (destination_floor == floor) {
        return false;
    }

    y = snap_to_floor(collision_map, x, floor_y_from_index(destination_floor));
    vy = 0.0f;
    airborne = false;
    walk_frame = 0;
    walk_frame_distance = 0.0f;
    return true;
}

} // namespace zg
