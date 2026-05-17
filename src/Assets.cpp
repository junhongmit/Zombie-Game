#include "Assets.h"

#include <cstdio>

namespace zg {

namespace {

bool load_optional_sheet(SDL_Renderer* renderer, Texture* texture, SpriteSheet* meta, bool* loaded, const char* image_path, const char* meta_path)
{
    *loaded = texture->load(renderer, image_path, true) && meta->load_metadata(meta_path);
    if (*loaded) {
        return true;
    }

    texture->reset();
    *meta = SpriteSheet{};
    *loaded = false;
    return false;
}

} // namespace

bool Assets::load(SDL_Renderer* renderer)
{
    bool ok = sky.load(renderer, "assets/backgrounds/sky1.png", true) &&
        backcity1.load(renderer, "assets/backgrounds/backcity1.png", true) &&
        backcity2.load(renderer, "assets/backgrounds/backcity2.png", true) &&
        backcity3.load(renderer, "assets/backgrounds/backcity3.png", true) &&
        building.load(renderer, "assets/backgrounds/building1.png", true) &&
        hero.load(renderer, "assets/characters/man1.png", true) &&
        bullet_icon.load(renderer, "assets/ui/icons/bull.png", true) &&
        smoke.load(renderer, "assets/effects/smog.png", true) &&
        zombie.load(renderer, "assets/characters/zom1.png", true) &&
        zombie_mask.load("assets/characters/zom1.png");

    if (!ok) {
        return false;
    }

    load_optional_sheet(
        renderer,
        &explosion_sheet,
        &explosion_sheet_meta,
        &has_explosion_sheet,
        "assets/effects/boom1_strip.png",
        "assets/effects/boom1_strip.sheet");
    load_optional_sheet(
        renderer,
        &heavy_explosion_sheet,
        &heavy_explosion_sheet_meta,
        &has_heavy_explosion_sheet,
        "assets/effects/boom2_strip.png",
        "assets/effects/boom2_strip.sheet");
    load_optional_sheet(
        renderer,
        &grenade_effect_sheet,
        &grenade_effect_sheet_meta,
        &has_grenade_effect_sheet,
        "assets/effects/gre_strip.png",
        "assets/effects/gre_strip.sheet");

    return ok && has_explosion_sheet;
}

} // namespace zg
