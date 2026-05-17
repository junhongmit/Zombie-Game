#include "SpriteSheet.h"
#include "AssetPaths.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace zg {

namespace {

int parse_int_value(const char* text)
{
    return std::atoi(text);
}

} // namespace

SpriteSheet::SpriteSheet(int frame_width, int frame_height, int stride_x, int frame_count)
    : frame_width_(frame_width),
      frame_height_(frame_height),
      stride_x_(stride_x),
      frame_count_(frame_count)
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

int SpriteSheet::frame_count() const
{
    return frame_count_;
}

bool SpriteSheet::load_metadata(const char* path)
{
    const std::string resolved = resolve_asset_path(path);
    std::FILE* file = std::fopen(resolved.c_str(), "r");
    if (file == nullptr) {
        return false;
    }

    int frame_width = 0;
    int frame_height = 0;
    int stride_x = 0;
    int frame_count = 0;
    char line[256];
    while (std::fgets(line, sizeof(line), file) != nullptr) {
        char* key = std::strtok(line, "=");
        char* value = std::strtok(nullptr, "\r\n");
        if (key == nullptr || value == nullptr) {
            continue;
        }

        if (std::strcmp(key, "frame_width") == 0) {
            frame_width = parse_int_value(value);
        } else if (std::strcmp(key, "frame_height") == 0) {
            frame_height = parse_int_value(value);
        } else if (std::strcmp(key, "stride_x") == 0) {
            stride_x = parse_int_value(value);
        } else if (std::strcmp(key, "frame_count") == 0) {
            frame_count = parse_int_value(value);
        }
    }
    std::fclose(file);

    if (frame_width <= 0 || frame_height <= 0 || stride_x <= 0 || frame_count <= 0) {
        return false;
    }

    frame_width_ = frame_width;
    frame_height_ = frame_height;
    stride_x_ = stride_x;
    frame_count_ = frame_count;
    return true;
}

} // namespace zg
