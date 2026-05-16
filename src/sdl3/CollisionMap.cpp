#include "CollisionMap.h"

#include <SDL3_image/SDL_image.h>

#include <cstdio>

namespace zg {

CollisionMap::CollisionMap()
    : surface_(nullptr)
{
}

CollisionMap::~CollisionMap()
{
    reset();
}

CollisionMap::CollisionMap(CollisionMap&& other) noexcept
    : surface_(other.surface_)
{
    other.surface_ = nullptr;
}

CollisionMap& CollisionMap::operator=(CollisionMap&& other) noexcept
{
    if (this != &other) {
        reset();
        surface_ = other.surface_;
        other.surface_ = nullptr;
    }
    return *this;
}

bool CollisionMap::load(const char* path)
{
    reset();

    surface_ = IMG_Load(path);
    if (surface_ == nullptr) {
        std::fprintf(stderr, "Failed to load collision map %s: %s\n", path, SDL_GetError());
        return false;
    }

    return true;
}

void CollisionMap::reset()
{
    if (surface_ != nullptr) {
        SDL_DestroySurface(surface_);
        surface_ = nullptr;
    }
}

bool CollisionMap::is_solid(int x, int y) const
{
    if (surface_ == nullptr || x < 0 || y < 0 || x >= surface_->w || y >= surface_->h) {
        return false;
    }

    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 0;
    if (!SDL_ReadSurfacePixel(surface_, x, y, &r, &g, &b, &a)) {
        return false;
    }

    return a > 0 && r == 255 && g == 255 && b == 0;
}

TriggerType CollisionMap::trigger_at(int x, int y) const
{
    if (surface_ == nullptr || x < 0 || y < 0 || x >= surface_->w || y >= surface_->h) {
        return TriggerType::None;
    }

    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 0;
    if (!SDL_ReadSurfacePixel(surface_, x, y, &r, &g, &b, &a) || a == 0) {
        return TriggerType::None;
    }

    if (r == 255 && g == 0 && b == 0) {
        return TriggerType::StairRed;
    }
    if (r == 0 && g == 255 && b == 0) {
        return TriggerType::StairGreen;
    }
    if (r == 0 && g == 0 && b == 255) {
        return TriggerType::StairBlue;
    }

    return TriggerType::None;
}

int CollisionMap::width() const
{
    return surface_ != nullptr ? surface_->w : 0;
}

int CollisionMap::height() const
{
    return surface_ != nullptr ? surface_->h : 0;
}

} // namespace zg
