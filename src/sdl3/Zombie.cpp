#include "Zombie.h"

#include "Constants.h"
#include "MathUtil.h"

#include <cmath>

namespace zg {

Zombie::Zombie(float x_in, float y_in, float speed_in, bool walking_right_in, int walk_frame_in)
    : x(x_in),
      y(y_in),
      speed(speed_in),
      walking_right(walking_right_in),
      walk_frame(walk_frame_in),
      walk_frame_distance(0.0f)
{
}

void Zombie::update(float dt)
{
    const float old_x = x;
    x += (walking_right ? speed : -speed) * dt;
    if (x < 0.0f) {
        x = 0.0f;
        walking_right = true;
    } else if (x > kWorldWidth - 18.0f) {
        x = kWorldWidth - 18.0f;
        walking_right = false;
    }

    walk_frame_distance += std::abs(x - old_x);
    while (walk_frame_distance >= kWalkPixelsPerFrame) {
        walk_frame_distance -= kWalkPixelsPerFrame;
        walk_frame = (walk_frame + 1) % 24;
    }
}

} // namespace zg
