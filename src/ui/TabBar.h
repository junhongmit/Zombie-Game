#pragma once

#include "Button.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include <vector>

namespace zg {
namespace ui {

enum class TabBarOrientation {
    Horizontal,
    Vertical
};

class TabBar {
public:
    TabBar() = default;
    TabBar(float x, float y, float w, float h, TabBarOrientation orientation = TabBarOrientation::Horizontal);

    TabBar& set_rect(float x, float y, float w, float h);
    TabBar& set_orientation(TabBarOrientation orientation);
    TabBar& set_tab_gap(float gap);
    TabBar& set_labels(const std::vector<std::string>& labels);
    TabBar& set_selected_index(int index);

    int update_and_render(
        SDL_Renderer* renderer,
        const ControlStyle& skin,
        TTF_Font* font,
        const SDL_FRect& presentation_rect,
        SDL_Color enabled_text_color,
        SDL_Color disabled_text_color,
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_down,
        bool mouse_pressed,
        bool mouse_released,
        Uint8 alpha = 255);

    int selected_index() const { return selected_index_; }

private:
    SDL_FRect tab_rect(int index) const;

    SDL_FRect logical_rect_{};
    TabBarOrientation orientation_ = TabBarOrientation::Horizontal;
    float tab_gap_ = 0.0f;
    std::vector<std::string> labels_{};
    int selected_index_ = 0;
    std::vector<unsigned char> armed_tabs_{};
};

} // namespace ui
} // namespace zg
