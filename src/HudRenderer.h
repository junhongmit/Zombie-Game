#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace zg {

struct Camera;
struct Player;
class Texture;
struct WeaponDefinition;
struct WeaponState;

class HudRenderer {
public:
    explicit HudRenderer(SDL_Renderer* renderer);
    ~HudRenderer();

    void render_weapon_status(
        const Texture& bullet_icon,
        const Player& player,
        const WeaponDefinition* weapon_definition,
        const WeaponState& weapon,
        const Camera& camera);
    void render_top_bar(const WeaponState& weapon, int wave, int alive_count);
    void render_title_screen(float ui_alpha, bool show_prompt);

private:
    void render_text(const char* text, float x, float y);
    void render_slot_bar(const WeaponState& weapon, float x, float y);

    SDL_Renderer* renderer_;
    TTF_Font* hud_font_ = nullptr;
};

} // namespace zg
