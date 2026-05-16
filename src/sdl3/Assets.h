#pragma once

#include "Texture.h"

struct SDL_Renderer;

namespace zg {

struct Assets {
    Texture sky;
    Texture backcity1;
    Texture backcity2;
    Texture backcity3;
    Texture building;
    Texture hero;
    Texture weapon_glock;
    Texture zombie;

    bool load(SDL_Renderer* renderer);
};

} // namespace zg
