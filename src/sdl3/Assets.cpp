#include "Assets.h"

namespace zg {

bool Assets::load(SDL_Renderer* renderer)
{
    return sky.load(renderer, "image/sky1.png", true) &&
        backcity1.load(renderer, "image/backcity1.png", true) &&
        backcity2.load(renderer, "image/backcity2.png", true) &&
        backcity3.load(renderer, "image/backcity3.png", true) &&
        building.load(renderer, "image/building1.png", true) &&
        hero.load(renderer, "image/man1.png", true) &&
        weapon_glock.load(renderer, "image/hold glock.png", true) &&
        zombie.load(renderer, "image/zom1.png", true) &&
        zombie_mask.load("image/zom1.png");
}

} // namespace zg
