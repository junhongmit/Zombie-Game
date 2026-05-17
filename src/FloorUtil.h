#pragma once

#include "Constants.h"

namespace zg {

inline int floor_index_from_y(float y)
{
    if (y <= 200.0f) {
        return 4;
    }
    if (y <= 280.0f) {
        return 3;
    }
    if (y <= 360.0f) {
        return 2;
    }
    return 1;
}

inline float floor_y_from_index(int floor)
{
    switch (floor) {
    case 4:
        return kFloor4Y;
    case 3:
        return kFloor3Y;
    case 2:
        return kFloor2Y;
    case 1:
    default:
        return kFloor1Y;
    }
}

} // namespace zg
