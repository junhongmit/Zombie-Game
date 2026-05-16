#pragma once

namespace zg {

struct Zombie {
    float x = 0.0f;
    float y = 400.0f;
    float speed = 24.0f;
    bool walking_right = true;
    int walk_frame = 0;
    float walk_frame_distance = 0.0f;

    Zombie(float x_in, float y_in, float speed_in, bool walking_right_in, int walk_frame_in);
    void update(float dt);
};

} // namespace zg
