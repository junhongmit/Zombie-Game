#include "Player.h"

#include "Constants.h"
#include "MathUtil.h"

#include <cmath>

namespace zg {

Player::Player()
    : y(kGroundY)
{
}

void Player::update(float axis, float aim_x, float aim_y, bool has_aim, float dt)
{
    moving = axis != 0.0f;
    if (moving) {
        const float old_x = x;
        x = clamp_float(x + axis * kPlayerMoveSpeed * dt, 0.0f, kWorldWidth - 18.0f);
        facing_right = axis > 0.0f;

        walk_frame_distance += std::abs(x - old_x);
        while (walk_frame_distance >= kWalkPixelsPerFrame) {
            walk_frame_distance -= kWalkPixelsPerFrame;
            walk_frame = (walk_frame + 1) % 24;
        }
    } else {
        walk_frame = 0;
        walk_frame_distance = 0.0f;
    }

    if (has_aim) {
        aim_world_x = aim_x;
        aim_world_y = aim_y;
        const float pivot_x = x + 9.0f;
        const float pivot_y = y + 13.0f;
        aim_angle_radians = std::atan2(aim_world_y - pivot_y, aim_world_x - pivot_x);
        facing_right = aim_world_x >= pivot_x;
    }
}

} // namespace zg
