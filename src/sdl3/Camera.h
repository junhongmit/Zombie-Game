#pragma once

namespace zg {

struct Camera {
    float x = 200.0f;

    void follow(float target_x);
};

} // namespace zg
