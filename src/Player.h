#pragma once

#include "Actor.h"

namespace zg {

class CollisionMap;

struct Player : Actor {
    bool facing_right = true;
    bool moving = false;
    int walk_frame = 0;
    float walk_frame_distance = 0.0f;
    float aim_world_x = 0.0f;
    float aim_world_y = 0.0f;
    float aim_angle_radians = 0.0f;

    Player();
    void update(
        const CollisionMap& collision_map,
        float axis,
        bool jump_pressed,
        float aim_x,
        float aim_y,
        bool has_aim,
        float dt);
    void damage(float amount);
    bool try_use_stairs(const CollisionMap& collision_map);
};

} // namespace zg
