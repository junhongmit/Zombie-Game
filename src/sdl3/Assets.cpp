#include "Assets.h"

#include <cstdio>

namespace zg {

bool Assets::load(SDL_Renderer* renderer)
{
    bool ok = sky.load(renderer, "image/sky1.png", true) &&
        backcity1.load(renderer, "image/backcity1.png", true) &&
        backcity2.load(renderer, "image/backcity2.png", true) &&
        backcity3.load(renderer, "image/backcity3.png", true) &&
        building.load(renderer, "image/building1.png", true) &&
        hero.load(renderer, "image/man1.png", true) &&
        bullet_icon.load(renderer, "image/bull.png", true) &&
        smoke.load(renderer, "image/smog.png", true) &&
        zombie.load(renderer, "image/zom1.png", true) &&
        zombie_mask.load("image/zom1.png");

    for (int i = 0; ok && i < kExplosionFrameCount; ++i) {
        char path[64];
        std::snprintf(path, sizeof(path), "image/boom/boom1_%d.bmp", i + 1);
        ok = explosions[i].load(renderer, path, true);
    }

    return ok;
}

} // namespace zg
