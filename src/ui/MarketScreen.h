#pragma once

#include "LayoutHotReload.h"
#include "MarketLayout.h"
#include "TabBar.h"
#include "../LocalizationTable.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include <vector>

namespace zg {

struct Assets;
class InventoryState;
class WeaponCatalog;
struct WeaponDefinition;

class MarketScreen {
public:
    explicit MarketScreen(SDL_Renderer* renderer);
    ~MarketScreen();

    bool render(
        const Assets& assets,
        const InventoryState& inventory,
        const WeaponCatalog& weapon_catalog,
        const SDL_FRect& presentation_rect,
        float dt,
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_down,
        bool mouse_pressed,
        bool mouse_released);

private:
    enum class MarketCategory {
        All = 0,
        Pistol,
        Shotgun,
        Rifle,
        Sniper,
        Other
    };

    struct CatalogEntry {
        const WeaponDefinition* definition = nullptr;
        int price = 0;
    };

    void ensure_font(float ui_scale);
    void rebuild_catalog(const WeaponCatalog& weapon_catalog);
    bool matches_category(const WeaponDefinition& definition, MarketCategory category) const;
    void render_text(TTF_Font* font, const char* text, float x, float y, SDL_Color color) const;
    void render_text_centered(TTF_Font* font, const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset = 0.0f) const;
    void render_resource_strip(const Assets& assets, const InventoryState& inventory) const;
    void render_left_panels(const Assets& assets, const InventoryState& inventory) const;
    void render_notebook(const Assets& assets, const WeaponCatalog& weapon_catalog, float mouse_x, float mouse_y, bool mouse_in_view, bool mouse_down, bool mouse_pressed, bool mouse_released);
    void render_category_buttons(const Assets& assets, float mouse_x, float mouse_y, bool mouse_in_view, bool mouse_down, bool mouse_pressed, bool mouse_released);
    const std::string& tr(const char* key, const char* fallback) const;
    std::string weapon_category_label(const WeaponDefinition& definition) const;
    float stat_value_for_row(const WeaponDefinition& definition, int row) const;
    float stat_ratio_for_row(const WeaponDefinition& definition, int row) const;
    float to_screen_x(float logical_x) const;
    float to_screen_y(float logical_y) const;
    SDL_FRect to_screen_rect(const SDL_FRect& logical_rect) const;

    SDL_Renderer* renderer_;
    TTF_Font* font_ = nullptr;
    TTF_Font* title_font_ = nullptr;
    TTF_Font* body_font_ = nullptr;
    TTF_Font* small_font_ = nullptr;
    int font_point_size_ = 0;
    SDL_FRect presentation_rect_{};
    MarketLayout layout_{};
    ui::LayoutHotReload layout_hot_reload_{};
    ui::TabBar category_tabs_{};
    LocalizationTable strings_{};
    bool close_armed_ = false;
    bool prev_page_armed_ = false;
    bool next_page_armed_ = false;
    MarketCategory active_category_ = MarketCategory::All;
    int current_page_ = 0;
    std::vector<CatalogEntry> filtered_entries_{};
};

} // namespace zg
