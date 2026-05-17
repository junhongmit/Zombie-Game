#pragma once

#include <algorithm>

namespace zg {

inline float clamp_float(float value, float low, float high)
{
    return std::max(low, std::min(value, high));
}

} // namespace zg
