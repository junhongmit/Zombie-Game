#pragma once

#include "InventoryLayout.h"
#include "../LocalizationTable.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace zg {

struct Assets;
class InventoryState;
struct WeaponState;

class InventoryScreen {
public:
    explicit InventoryScreen(SDL_Renderer* renderer);
    ~InventoryScreen();

    void render(
        const Assets& assets,
        const InventoryState& inventory,
        const WeaponState& weapon_state,
        const SDL_FRect& presentation_rect);

private:
    void ensure_font(float ui_scale);
    void render_text(const char* text, float x, float y, SDL_Color color) const;
    void render_text_centered(const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset = 0.0f) const;
    const std::string& tr(const char* key, const char* fallback) const;

    SDL_Renderer* renderer_;
    TTF_Font* font_ = nullptr;
    int font_point_size_ = 0;
    SDL_FRect presentation_rect_{};
    InventoryLayout layout_{};
    LocalizationTable strings_{};
};

} // namespace zg
