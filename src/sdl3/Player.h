#pragma once

namespace zg {

struct Player {
    float x = 520.0f;
    float y = 0.0f;
    bool facing_right = true;
    bool moving = false;
    int walk_frame = 0;
    float walk_frame_distance = 0.0f;
    float aim_world_x = 0.0f;
    float aim_world_y = 0.0f;
    float aim_angle_radians = 0.0f;

    Player();
    void update(float axis, float aim_x, float aim_y, bool has_aim, float dt);
};

} // namespace zg
