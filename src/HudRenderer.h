#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace zg {

struct Camera;
struct Player;
class Texture;
class ControlStyle;
struct Assets;
class InventoryState;
class LocalizationTable;
struct WeaponDefinition;
struct WeaponState;
class GameHudLayout;

enum class TitleMenuAction {
    None,
    Start,
    Loadout,
    Options,
    Exit
};

class HudRenderer {
public:
    explicit HudRenderer(SDL_Renderer* renderer);
    ~HudRenderer();

    void render_weapon_status(
        const Texture& bullet_icon,
        const Player& player,
        const WeaponDefinition* weapon_definition,
        const WeaponState& weapon,
        const Camera& camera,
        const SDL_FRect& presentation_rect);
    void render_top_bar(const WeaponState& weapon, int wave, int alive_count, const SDL_FRect& presentation_rect);
    void render_gameplay_hud(
        const Assets& assets,
        const Player& player,
        const InventoryState& inventory,
        const WeaponState& weapon,
        int wave,
        int alive_count,
        const SDL_FRect& presentation_rect);
    void render_title_screen(
        const ControlStyle& button_skin,
        float ui_alpha,
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_down,
        TitleMenuAction pressed_action,
        const SDL_FRect& presentation_rect);
    TitleMenuAction hit_test_title_menu(float mouse_x, float mouse_y, bool mouse_in_view) const;

private:
    void ensure_hud_font(float ui_scale);
    void render_text(const char* text, float x, float y);
    void render_text_colored(const char* text, float x, float y, SDL_Color color);
    void render_text_centered(const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset = 0.0f);
    void render_ui_text(const char* text, float x, float y, SDL_Color color);
    void render_ui_text_centered(const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset = 0.0f);
    void render_slot_bar(const WeaponState& weapon, float x, float y);
    void render_panel_title_value(const char* title, const char* value, const SDL_FRect& rect, SDL_Color title_color, SDL_Color value_color);
    void render_progress_label_value(const char* label, const char* value, float x, float y, float width, SDL_Color color);
    float to_screen_x(float logical_x) const;
    float to_screen_y(float logical_y) const;
    SDL_FRect to_screen_rect(const SDL_FRect& logical_rect) const;
    float ui_to_screen_x(float logical_x) const;
    float ui_to_screen_y(float logical_y) const;
    SDL_FRect ui_to_screen_rect(const SDL_FRect& logical_rect) const;

    SDL_Renderer* renderer_;
    TTF_Font* hud_font_ = nullptr;
    int hud_font_point_size_ = 0;
    SDL_FRect presentation_rect_{};
    GameHudLayout* gameplay_layout_ = nullptr;
    LocalizationTable* gameplay_strings_ = nullptr;
};

} // namespace zg
