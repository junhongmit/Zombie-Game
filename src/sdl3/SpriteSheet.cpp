#include "SpriteSheet.h"

namespace zg {

SpriteSheet::SpriteSheet(int frame_width, int frame_height, int stride_x)
    : frame_width_(frame_width),
      frame_height_(frame_height),
      stride_x_(stride_x)
{
}

SDL_FRect SpriteSheet::frame_rect(int frame_index) const
{
    return SDL_FRect{
        static_cast<float>(frame_index * stride_x_),
        0.0f,
        static_cast<float>(frame_width_),
        static_cast<float>(frame_height_)
    };
}

} // namespace zg
