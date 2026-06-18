#pragma once

#include "ListView.h"
#include "ProgressBar.h"
#include "WorkbenchLayout.h"
#include "../LocalizationTable.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

namespace zg {

struct Assets;
struct WeaponDefinition;
struct WeaponState;

class WorkbenchScreen {
public:
    explicit WorkbenchScreen(SDL_Renderer* renderer);
    ~WorkbenchScreen();

    void render(
        const Assets& assets,
        WeaponState& weapon_state,
        const SDL_FRect& presentation_rect,
        float dt,
        float wheel_x,
        float wheel_y,
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_down,
        bool mouse_pressed,
        bool mouse_released);

private:
    void ensure_font(float ui_scale);
    void render_text(const char* text, float x, float y, SDL_Color color) const;
    void render_text_centered(const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset = 0.0f) const;
    void render_weapon_preview(const Assets& assets, const WeaponDefinition* definition, const SDL_FRect& logical_rect) const;
    void render_weapon_list(Assets const& assets, WeaponState& weapon_state, float dt, float wheel_y, float mouse_x, float mouse_y, bool mouse_in_view, bool mouse_down, bool mouse_pressed, bool mouse_released);
    void render_weapon_stats(const Assets& assets, const WeaponDefinition* definition, const SDL_FRect& logical_rect) const;
    void render_preview_nameplate(const Assets& assets, const WeaponDefinition* definition, const SDL_FRect& logical_rect) const;
    void render_resource_strip() const;
    void render_attachment_strip(
        const Assets& assets,
        float dt,
        float wheel_x,
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_down,
        bool mouse_pressed,
        bool mouse_released);
    void render_attachment_popup(
        const Assets& assets,
        float dt,
        float wheel_x,
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_down,
        bool mouse_pressed,
        bool mouse_released);
    const std::string& tr(const char* key, const char* fallback) const;

    float to_screen_x(float logical_x) const;
    float to_screen_y(float logical_y) const;
    SDL_FRect to_screen_rect(const SDL_FRect& logical_rect) const;

    SDL_Renderer* renderer_;
    TTF_Font* font_ = nullptr;
    int font_point_size_ = 0;
    SDL_FRect presentation_rect_{};
    WorkbenchLayout layout_{};
    LocalizationTable strings_{};
    ui::ListView weapon_list_view_;
    ui::ListView attachment_strip_view_;
    ui::ListView attachment_popup_view_;
    int armed_weapon_index_ = -1;
    int active_attachment_index_ = -1;
    int armed_attachment_index_ = -1;
    int armed_popup_option_index_ = -1;
    int selected_attachment_option_[6] = {0, 0, 0, 0, 0, -1};
    SDL_FRect attachment_anchor_rect_{};
};

} // namespace zg
