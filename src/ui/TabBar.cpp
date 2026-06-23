#include "TabBar.h"

#include <algorithm>

namespace zg {
namespace ui {

TabBar::TabBar(float x, float y, float w, float h, TabBarOrientation orientation)
    : orientation_(orientation)
{
    set_rect(x, y, w, h);
}

TabBar& TabBar::set_rect(float x, float y, float w, float h)
{
    logical_rect_ = SDL_FRect{x, y, w, h};
    return *this;
}

TabBar& TabBar::set_orientation(TabBarOrientation orientation)
{
    orientation_ = orientation;
    return *this;
}

TabBar& TabBar::set_tab_gap(float gap)
{
    tab_gap_ = gap;
    return *this;
}

TabBar& TabBar::set_labels(const std::vector<std::string>& labels)
{
    labels_ = labels;
    armed_tabs_.assign(labels_.size(), false);
    selected_index_ = std::max(0, std::min(selected_index_, static_cast<int>(labels_.size()) - 1));
    return *this;
}

TabBar& TabBar::set_selected_index(int index)
{
    selected_index_ = std::max(0, std::min(index, static_cast<int>(labels_.size()) - 1));
    return *this;
}

int TabBar::update_and_render(
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
    Uint8 alpha)
{
    if (armed_tabs_.size() != labels_.size()) {
        armed_tabs_.assign(labels_.size(), false);
    }

    int activated_index = -1;
    for (int i = 0; i < static_cast<int>(labels_.size()); ++i) {
        const SDL_FRect rect = tab_rect(i);
        Button button(labels_[static_cast<size_t>(i)].c_str(), rect.x, rect.y, rect.w, rect.h, true);
        bool armed = armed_tabs_[static_cast<size_t>(i)] != 0;
        if (button.process_pointer(mouse_x, mouse_y, mouse_in_view, mouse_pressed, mouse_released, armed)) {
            activated_index = i;
            selected_index_ = i;
        }
        armed_tabs_[static_cast<size_t>(i)] = armed ? 1 : 0;
        button.render(
            renderer,
            skin,
            font,
            presentation_rect,
            mouse_x,
            mouse_y,
            mouse_in_view,
            mouse_down,
            armed || selected_index_ == i,
            enabled_text_color,
            disabled_text_color,
            alpha);
    }
    return activated_index;
}

SDL_FRect TabBar::tab_rect(int index) const
{
    if (labels_.empty()) {
        return logical_rect_;
    }

    if (orientation_ == TabBarOrientation::Horizontal) {
        const float total_gap = tab_gap_ * std::max(0, static_cast<int>(labels_.size()) - 1);
        const float tab_width = (logical_rect_.w - total_gap) / static_cast<float>(labels_.size());
        return SDL_FRect{
            logical_rect_.x + index * (tab_width + tab_gap_),
            logical_rect_.y,
            tab_width,
            logical_rect_.h
        };
    }

    const float total_gap = tab_gap_ * std::max(0, static_cast<int>(labels_.size()) - 1);
    const float tab_height = (logical_rect_.h - total_gap) / static_cast<float>(labels_.size());
    return SDL_FRect{
        logical_rect_.x,
        logical_rect_.y + index * (tab_height + tab_gap_),
        logical_rect_.w,
        tab_height
    };
}

} // namespace ui
} // namespace zg
