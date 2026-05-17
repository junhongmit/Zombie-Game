#include "SurfaceMask.h"
#include "AssetPaths.h"

#include <SDL3_image/SDL_image.h>

#include <cstdio>

namespace zg {

SurfaceMask::SurfaceMask()
    : surface_(nullptr)
{
}

SurfaceMask::~SurfaceMask()
{
    reset();
}

SurfaceMask::SurfaceMask(SurfaceMask&& other) noexcept
    : surface_(other.surface_)
{
    other.surface_ = nullptr;
}

SurfaceMask& SurfaceMask::operator=(SurfaceMask&& other) noexcept
{
    if (this != &other) {
        reset();
        surface_ = other.surface_;
        other.surface_ = nullptr;
    }
    return *this;
}

bool SurfaceMask::load(const char* path)
{
    reset();

    const std::string resolved = resolve_asset_path(path);
    surface_ = IMG_Load(resolved.c_str());
    if (surface_ == nullptr) {
        std::fprintf(stderr, "Failed to load surface mask %s: %s\n", resolved.c_str(), SDL_GetError());
        return false;
    }

    return true;
}

void SurfaceMask::reset()
{
    if (surface_ != nullptr) {
        SDL_DestroySurface(surface_);
        surface_ = nullptr;
    }
}

bool SurfaceMask::is_opaque_nonwhite(int x, int y) const
{
    if (surface_ == nullptr || x < 0 || y < 0 || x >= surface_->w || y >= surface_->h) {
        return false;
    }

    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 0;
    if (!SDL_ReadSurfacePixel(surface_, x, y, &r, &g, &b, &a) || a == 0) {
        return false;
    }

    return !(r == 255 && g == 255 && b == 255);
}

int SurfaceMask::width() const
{
    return surface_ != nullptr ? surface_->w : 0;
}

int SurfaceMask::height() const
{
    return surface_ != nullptr ? surface_->h : 0;
}

} // namespace zg
