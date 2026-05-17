#pragma once

#include "SpriteSheet.h"
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
    Texture explosion_sheet;
    Texture heavy_explosion_sheet;
    Texture grenade_effect_sheet;
    SpriteSheet explosion_sheet_meta;
    SpriteSheet heavy_explosion_sheet_meta;
    SpriteSheet grenade_effect_sheet_meta;
    bool has_explosion_sheet = false;
    bool has_heavy_explosion_sheet = false;
    bool has_grenade_effect_sheet = false;
    Texture explosions[kExplosionFrameCount];
    SurfaceMask zombie_mask;

    bool load(SDL_Renderer* renderer);
};

} // namespace zg
