#pragma once

#include <SDL3/SDL.h>

namespace zg {

class SurfaceMask {
public:
    SurfaceMask();
    ~SurfaceMask();

    SurfaceMask(const SurfaceMask&) = delete;
    SurfaceMask& operator=(const SurfaceMask&) = delete;

    SurfaceMask(SurfaceMask&& other) noexcept;
    SurfaceMask& operator=(SurfaceMask&& other) noexcept;

    bool load(const char* path);
    void reset();

    bool is_opaque_nonwhite(int x, int y) const;
    int width() const;
    int height() const;

private:
    SDL_Surface* surface_;
};

} // namespace zg
