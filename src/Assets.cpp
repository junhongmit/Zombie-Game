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
    bool ok = sky.load(renderer, "assets/scenes/sky1.png", true) &&
        backcity1.load(renderer, "assets/scenes/backcity1.png", true) &&
        backcity2.load(renderer, "assets/scenes/backcity2.png", true) &&
        backcity3.load(renderer, "assets/scenes/backcity3.png", true) &&
        building.load(renderer, "assets/scenes/building1.png", true) &&
        bench.load(renderer, "assets/scenes/bench.png", true) &&
        hero.load(renderer, "assets/characters/man1.png", true) &&
        bullet_icon.load(renderer, "assets/ui/icons/bull.png", true) &&
        title_button_skin.load(
            renderer,
            "assets/ui/buttons/button1.png",
            "assets/ui/buttons/ui_button1.json",
            "button_square_bronze") &&
        weapon_card_style.load(
            renderer,
            "assets/ui/cards/card1.png",
            "assets/ui/cards/ui_card1.json",
            "card1_weapon_row") &&
        scrollbar_vertical_track_style.load(
            renderer,
            "assets/ui/scrolls/scroll1.png",
            "assets/ui/scrolls/ui_scroll1.json",
            "scrollbar_vertical_track") &&
        scrollbar_vertical_fill_style.load(
            renderer,
            "assets/ui/scrolls/scroll1.png",
            "assets/ui/scrolls/ui_scroll1.json",
            "scrollbar_vertical_fill") &&
        scrollbar_vertical_thumb_style.load(
            renderer,
            "assets/ui/scrolls/scroll1.png",
            "assets/ui/scrolls/ui_scroll1.json",
            "scrollbar_vertical_thumb") &&
        scrollbar_horizontal_track_style.load(
            renderer,
            "assets/ui/scrolls/scroll1.png",
            "assets/ui/scrolls/ui_scroll1.json",
            "scrollbar_horizontal_track") &&
        scrollbar_horizontal_fill_style.load(
            renderer,
            "assets/ui/scrolls/scroll1.png",
            "assets/ui/scrolls/ui_scroll1.json",
            "scrollbar_horizontal_fill") &&
        scrollbar_horizontal_thumb_style.load(
            renderer,
            "assets/ui/scrolls/scroll1.png",
            "assets/ui/scrolls/ui_scroll1.json",
            "scrollbar_horizontal_thumb") &&
        progressbar_horizontal_track_style.load(
            renderer,
            "assets/ui/progress/progress1.png",
            "assets/ui/progress/ui_progress1.json",
            "progressbar_horizontal_track") &&
        progressbar_horizontal_fill_style.load(
            renderer,
            "assets/ui/progress/progress1.png",
            "assets/ui/progress/ui_progress1.json",
            "progressbar_horizontal_fill") &&
        progressbar_vertical_track_style.load(
            renderer,
            "assets/ui/progress/progress1.png",
            "assets/ui/progress/ui_progress1.json",
            "progressbar_vertical_track") &&
        progressbar_vertical_fill_style.load(
            renderer,
            "assets/ui/progress/progress1.png",
            "assets/ui/progress/ui_progress1.json",
            "progressbar_vertical_fill") &&
        panel_skin.load(
            renderer,
            "assets/ui/panels/panel1.png",
            "assets/ui/panels/ui_panel1.json",
            "panel_square_bronze") &&
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
