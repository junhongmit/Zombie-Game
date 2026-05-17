#pragma once

namespace zg {

class CollisionMap;

struct Player {
    float x = 520.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float hp = 100.0f;
    bool facing_right = true;
    bool moving = false;
    bool airborne = false;
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
    void apply_impulse(float impulse_x, float impulse_y);
    void damage(float amount);
    bool try_use_stairs(const CollisionMap& collision_map);
};

} // namespace zg
