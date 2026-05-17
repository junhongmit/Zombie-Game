#include "SpriteCatalog.h"

namespace zg::sprites {

namespace {

SpriteSheet load_sheet_with_fallback(const char* metadata_path, int frame_width, int frame_height, int stride_x, int frame_count)
{
    SpriteSheet sheet(frame_width, frame_height, stride_x, frame_count);
    sheet.load_metadata(metadata_path);
    return sheet;
}

const SpriteSheet kHeroWalkSheet = load_sheet_with_fallback("assets/characters/man1.sheet", 18, 33, 17, 24);
const SpriteSheet kZombieWalkSheet = load_sheet_with_fallback("assets/characters/zom1.sheet", 18, 32, 17, 28);

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

bool using_generated_hero_sheet()
{
    return kHeroWalkSheet.frame_count() > 0 &&
        !(kHeroWalkSheet.frame_rect(0).w == 18.0f &&
          kHeroWalkSheet.frame_rect(0).h == 33.0f &&
          kHeroWalkSheet.frame_rect(1).x == 17.0f);
}

bool using_generated_zombie_sheet()
{
    return kZombieWalkSheet.frame_count() > 0 &&
        !(kZombieWalkSheet.frame_rect(0).w == 18.0f &&
          kZombieWalkSheet.frame_rect(0).h == 32.0f &&
          kZombieWalkSheet.frame_rect(1).x == 17.0f);
}

} // namespace zg::sprites
