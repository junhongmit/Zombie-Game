#pragma once

#include "Texture.h"
#include "SurfaceMask.h"

struct SDL_Renderer;

namespace zg {

struct Assets {
    static constexpr int kExplosionFrameCount = 15;

    Texture sky;
    Texture backcity1;
    Texture backcity2;
    Texture backcity3;
    Texture building;
    Texture hero;
    Texture bullet_icon;
    Texture smoke;
    Texture zombie;
    Texture explosions[kExplosionFrameCount];
    SurfaceMask zombie_mask;

    bool load(SDL_Renderer* renderer);
};

} // namespace zg
