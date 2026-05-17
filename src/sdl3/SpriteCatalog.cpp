#include "SpriteCatalog.h"

namespace zg::sprites {

namespace {

const SpriteSheet kHeroWalkSheet(18, 33, 17);
const SpriteSheet kZombieWalkSheet(18, 32, 17);

} // namespace

const SpriteSheet& hero_walk_sheet()
{
    return kHeroWalkSheet;
}

const SpriteSheet& zombie_walk_sheet()
{
    return kZombieWalkSheet;
}

SDL_FRect zombie_corpse_frame()
{
    return SDL_FRect{477.0f, 0.0f, 17.0f, 32.0f};
}

} // namespace zg::sprites
