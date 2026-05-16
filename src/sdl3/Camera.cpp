#include "Camera.h"

#include "Constants.h"
#include "MathUtil.h"

namespace zg {

void Camera::follow(float target_x)
{
    const float desired = target_x - kLogicalWidth * 0.5f;
    const float max_x = kWorldWidth - kLogicalWidth;
    x = clamp_float(desired, 0.0f, max_x);
}

} // namespace zg
