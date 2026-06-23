#pragma once

#include "InventoryLayout.h"
#include "LayoutHotReload.h"
#include "ControlStyle.h"
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

    bool update_and_render(
        const Assets& assets,
        InventoryState& inventory,
        const WeaponState& weapon_state,
        const SDL_FRect& presentation_rect,
        float dt,
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_down,
        bool mouse_pressed,
        bool mouse_released,
        bool sort_pressed,
        bool use_pressed,
        bool drop_pressed,
        bool split_pressed);

private:
    enum class DragSource {
        None = 0,
        Bag,
        Equipment,
        Backpack
    };

    enum class InventoryAction {
        None = 0,
        Sort,
        Use,
        Drop,
        Split,
        Close
    };

    void ensure_fonts(float ui_scale);
    void render_text(TTF_Font* font, const char* text, float x, float y, SDL_Color color) const;
    void render_text_centered(TTF_Font* font, const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset = 0.0f) const;
    void render_action_hint(const ControlStyle& skin, const SDL_FRect& rect, const char* key, const char* label, bool highlighted) const;
    void render_tooltip(const Assets& assets, const InventoryState& inventory, float mouse_x, float mouse_y) const;
    const std::string& tr(const char* key, const char* fallback) const;

    SDL_Renderer* renderer_;
    TTF_Font* section_font_ = nullptr;
    TTF_Font* title_font_ = nullptr;
    TTF_Font* body_font_ = nullptr;
    TTF_Font* small_font_ = nullptr;
    TTF_Font* key_font_ = nullptr;
    TTF_Font* number_font_ = nullptr;
    int base_point_size_ = 0;
    SDL_FRect presentation_rect_{};
    InventoryLayout layout_{};
    ui::LayoutHotReload layout_hot_reload_{};
    LocalizationTable strings_{};
    int selected_bag_index_ = 0;
    int selected_backpack_index_ = 0;
    DragSource press_source_ = DragSource::None;
    int press_index_ = -1;
    float press_start_x_ = 0.0f;
    float press_start_y_ = 0.0f;
    DragSource drag_source_ = DragSource::None;
    int drag_index_ = -1;
    DragSource tooltip_source_ = DragSource::None;
    int tooltip_index_ = -1;
    float tooltip_anchor_x_ = 0.0f;
    float tooltip_anchor_y_ = 0.0f;
    float tooltip_alpha_ = 0.0f;
    float close_flash_timer_ = 0.0f;
    bool close_pending_ = false;
    bool close_button_armed_ = false;
    InventoryAction armed_action_ = InventoryAction::None;
};

} // namespace zg
