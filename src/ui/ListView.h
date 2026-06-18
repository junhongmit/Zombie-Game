#pragma once

#include "Panel.h"
#include "Scrollbar.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace zg {
namespace ui {

enum class ListViewOrientation {
    Vertical,
    Horizontal
};

class ListView {
public:
    ListView() = default;
    ListView(float x, float y, float w, float h, ListViewOrientation orientation = ListViewOrientation::Vertical);

    ListView& set_rect(float x, float y, float w, float h);
    ListView& set_orientation(ListViewOrientation orientation);
    ListView& set_title(const char* title);
    ListView& set_enabled(bool enabled);
    ListView& set_content_height(float content_height);
    ListView& set_content_extent(float content_extent);
    ListView& set_scrollbar_width(float width);
    ListView& set_always_show_scrollbar(bool always_show);
    ListView& set_content_padding(float horizontal, float vertical);
    ListView& set_scrollbar_gap(float gap);
    ListView& set_draw_panel(bool draw_panel);
    ListView& set_draw_title(bool draw_title);
    ListView& nudge_scroll(float delta);

    void update_and_render(
        SDL_Renderer* renderer,
        const ControlStyle& panel_style,
        const ControlStyle& scrollbar_track_style,
        const ControlStyle& scrollbar_fill_style,
        const ControlStyle& scrollbar_thumb_style,
        TTF_Font* font,
        const SDL_FRect& presentation_rect,
        SDL_Color title_color,
        float dt,
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_down,
        bool mouse_pressed,
        bool mouse_released,
        Uint8 alpha = 255);

    const SDL_FRect& viewport_rect() const { return logical_viewport_rect_; }
    float scroll_offset() const { return scroll_offset_; }
    float progress() const { return scrollbar_.progress(); }
    bool contains(float x, float y, bool mouse_in_view) const;

private:
    float resolve_scrollbar_thickness(const ControlStyle& track_style, const ControlStyle& thumb_style) const;

    Panel panel_{};
    Scrollbar scrollbar_{0.0f, 0.0f, 1.0f, 1.0f, ScrollbarOrientation::Vertical};
    SDL_FRect logical_viewport_rect_{};
    float content_extent_ = 0.0f;
    float scroll_offset_ = 0.0f;
    float scrollbar_width_override_ = 0.0f;
    float content_padding_x_ = 0.0f;
    float content_padding_y_ = 0.0f;
    float scrollbar_gap_ = 0.0f;
    ListViewOrientation orientation_ = ListViewOrientation::Vertical;
    bool always_show_scrollbar_ = false;
    bool draw_panel_ = true;
    bool draw_title_ = true;
    bool enabled_ = true;
};

} // namespace ui
} // namespace zg
