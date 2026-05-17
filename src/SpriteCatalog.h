#pragma once

#include "SpriteSheet.h"

#include <SDL3/SDL.h>

namespace zg::sprites {

const SpriteSheet& hero_walk_sheet();
const SpriteSheet& zombie_walk_sheet();
SDL_FRect zombie_corpse_frame();
bool using_generated_hero_sheet();
bool using_generated_zombie_sheet();

} // namespace zg::sprites
