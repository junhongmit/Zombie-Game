#pragma once

#include <SDL3/SDL.h>

namespace zg {

class SpriteSheet {
public:
    SpriteSheet() = default;
    SpriteSheet(int frame_width, int frame_height, int stride_x);

    SDL_FRect frame_rect(int frame_index) const;

private:
    int frame_width_ = 0;
    int frame_height_ = 0;
    int stride_x_ = 0;
};

} // namespace zg
