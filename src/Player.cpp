#include "Player.h"

#include "CollisionMap.h"
#include "Constants.h"
#include "FloorUtil.h"
#include "HumanoidBody.h"
#include "MathUtil.h"

#include <cmath>

namespace zg {

namespace {

const HumanoidBody kPlayerBody(kPlayerWidth, kPlayerHeight, 1, 31, 5, 12);

} // namespace

Player::Player()
    : Actor(520.0f, kGroundY, 100.0f)
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
    if (!airborne && !kPlayerBody.has_floor_support(collision_map, x, y)) {
        airborne = true;
    }

    if (jump_pressed && !airborne) {
        airborne = true;
        vy = -kPlayerJumpSpeed;
        walk_frame = 0;
        walk_frame_distance = 0.0f;
    }

    if (std::fabs(vx) > 0.01f) {
        kPlayerBody.move_by_velocity(collision_map, dt, &x, &vx, y, kWorldWidth);
        vx *= std::pow(0.18f, dt);
        if (std::fabs(vx) < 4.0f) {
            vx = 0.0f;
        }
    }

    const bool wants_move = axis != 0.0f;
    if (wants_move) {
        const float old_x = x;
        kPlayerBody.move_horizontally(collision_map, axis, kPlayerMoveSpeed, dt, &x, y, kWorldWidth);
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
        kPlayerBody.move_vertically(collision_map, kGravity, -kPlayerJumpSpeed, kTerminalVelocity, dt, x, &y, &vy, &airborne, static_cast<float>(kLogicalHeight));
        if (!airborne) {
            y = kPlayerBody.snap_to_floor(collision_map, x, y, static_cast<float>(kLogicalHeight));
            vy = 0.0f;
        }
    }

    moving = wants_move && !airborne;

    if (has_aim) {
        aim_world_x = aim_x;
        aim_world_y = aim_y;
        const float pivot_x = x + 9.0f;
        const float pivot_y = y + 13.0f;
        aim_angle_radians = std::atan2(pivot_y - aim_world_y, aim_world_x - pivot_x);
        facing_right = aim_world_x >= pivot_x;
    }
}

void Player::damage(float amount)
{
    hp -= amount;
    clamp_hp_zero();
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

    y = kPlayerBody.snap_to_floor(collision_map, x, floor_y_from_index(destination_floor), static_cast<float>(kLogicalHeight));
    vy = 0.0f;
    airborne = false;
    walk_frame = 0;
    walk_frame_distance = 0.0f;
    return true;
}

} // namespace zg
