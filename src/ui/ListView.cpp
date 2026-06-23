#include "ListView.h"

#include "../Presentation.h"

#include <algorithm>
#include <cmath>

namespace zg {
namespace ui {

namespace {

float scaled_frame_width(const ControlStyle& style)
{
    const ControlStateStyle& state = style.resolved_state(ControlVisualState::Normal);
    return std::max(1.0f, state.frame.w * style.style_scale());
}

float scaled_frame_height(const ControlStyle& style)
{
    const ControlStateStyle& state = style.resolved_state(ControlVisualState::Normal);
    return std::max(1.0f, state.frame.h * style.style_scale());
}

} // namespace

ListView::ListView(float x, float y, float w, float h, ListViewOrientation orientation)
    : panel_(x, y, w, h),
      scrollbar_(0.0f, 0.0f, 1.0f, 1.0f, orientation == ListViewOrientation::Vertical ? ScrollbarOrientation::Vertical : ScrollbarOrientation::Horizontal),
      orientation_(orientation)
{
}

ListView& ListView::set_rect(float x, float y, float w, float h)
{
    panel_ = Panel(x, y, w, h);
    return *this;
}

ListView& ListView::set_orientation(ListViewOrientation orientation)
{
    if (orientation_ == orientation) {
        return *this;
    }
    orientation_ = orientation;
    scrollbar_ = Scrollbar(
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        orientation == ListViewOrientation::Vertical ? ScrollbarOrientation::Vertical : ScrollbarOrientation::Horizontal);
    scrollbar_.set_enabled(enabled_);
    return *this;
}

ListView& ListView::set_title(const char* title)
{
    panel_.set_title(title);
    return *this;
}

ListView& ListView::set_enabled(bool enabled)
{
    enabled_ = enabled;
    panel_.set_enabled(enabled);
    scrollbar_.set_enabled(enabled);
    return *this;
}

ListView& ListView::set_content_height(float content_height)
{
    content_extent_ = std::max(0.0f, content_height);
    return *this;
}

ListView& ListView::set_content_extent(float content_extent)
{
    content_extent_ = std::max(0.0f, content_extent);
    return *this;
}

ListView& ListView::set_scrollbar_width(float width)
{
    scrollbar_width_override_ = std::max(0.0f, width);
    return *this;
}

ListView& ListView::set_always_show_scrollbar(bool always_show)
{
    always_show_scrollbar_ = always_show;
    return *this;
}

ListView& ListView::set_content_padding(float horizontal, float vertical)
{
    content_padding_x_ = std::max(0.0f, horizontal);
    content_padding_y_ = std::max(0.0f, vertical);
    return *this;
}

ListView& ListView::set_scrollbar_gap(float gap)
{
    scrollbar_gap_ = std::max(0.0f, gap);
    return *this;
}

ListView& ListView::set_draw_panel(bool draw_panel)
{
    draw_panel_ = draw_panel;
    return *this;
}

ListView& ListView::set_draw_title(bool draw_title)
{
    draw_title_ = draw_title;
    return *this;
}

ListView& ListView::nudge_scroll(float delta)
{
    scrollbar_.nudge(delta);
    return *this;
}

bool ListView::contains(float x, float y, bool mouse_in_view) const
{
    if (!mouse_in_view) {
        return false;
    }
    const SDL_FRect rect = panel_.logical_rect();
    return x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h;
}

void ListView::update_and_render(
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
    Uint8 alpha)
{
    if (draw_panel_) {
        if (draw_title_) {
            panel_.render(renderer, panel_style, font, presentation_rect, title_color, alpha);
        } else {
            const SDL_FRect screen_rect = ui_logical_to_present_rect(panel_.logical_rect(), presentation_rect);
            panel_style.render(renderer, screen_rect, panel_.enabled() ? ControlVisualState::Normal : ControlVisualState::Disabled, alpha);
        }
    }

    SDL_FRect logical_content_rect = draw_panel_
        ? ui_present_to_logical_rect(panel_.content_rect(panel_style, presentation_rect), presentation_rect)
        : panel_.logical_rect();
    logical_content_rect.x += content_padding_x_;
    logical_content_rect.y += content_padding_y_;
    logical_content_rect.w = std::max(0.0f, logical_content_rect.w - content_padding_x_ * 2.0f);
    logical_content_rect.h = std::max(0.0f, logical_content_rect.h - content_padding_y_ * 2.0f);
    const float scrollbar_thickness = scrollbar_width_override_ > 0.0f
        ? scrollbar_width_override_
        : resolve_scrollbar_thickness(scrollbar_track_style, scrollbar_thumb_style);
    const float viewport_extent = orientation_ == ListViewOrientation::Vertical ? logical_content_rect.h : logical_content_rect.w;
    const bool needs_scrollbar = enabled_ && content_extent_ > viewport_extent + 0.5f;
    const bool show_scrollbar = enabled_ && (needs_scrollbar || always_show_scrollbar_);

    if (show_scrollbar) {
        const float reserved_thickness = scrollbar_thickness + scrollbar_gap_;
        if (orientation_ == ListViewOrientation::Vertical) {
            scrollbar_.set_rect(
                logical_content_rect.x + std::max(0.0f, logical_content_rect.w - scrollbar_thickness),
                logical_content_rect.y,
                scrollbar_thickness,
                logical_content_rect.h);
            logical_viewport_rect_ = SDL_FRect{
                logical_content_rect.x,
                logical_content_rect.y,
                std::max(0.0f, logical_content_rect.w - reserved_thickness),
                logical_content_rect.h
            };
        } else {
            scrollbar_.set_rect(
                logical_content_rect.x,
                logical_content_rect.y + std::max(0.0f, logical_content_rect.h - scrollbar_thickness),
                logical_content_rect.w,
                scrollbar_thickness);
            logical_viewport_rect_ = SDL_FRect{
                logical_content_rect.x,
                logical_content_rect.y,
                logical_content_rect.w,
                std::max(0.0f, logical_content_rect.h - reserved_thickness)
            };
        }
    } else {
        logical_viewport_rect_ = logical_content_rect;
    }

    const float visible_extent = orientation_ == ListViewOrientation::Vertical
        ? logical_viewport_rect_.h
        : logical_viewport_rect_.w;
    const float max_scroll = std::max(0.0f, content_extent_ - visible_extent);
    scrollbar_
        .set_visible_fraction(content_extent_ > 0.0f ? std::min(1.0f, visible_extent / content_extent_) : 1.0f)
        .set_enabled(show_scrollbar);

    if (show_scrollbar) {
        scrollbar_.update_and_render(
            renderer,
            scrollbar_track_style,
            scrollbar_fill_style,
            scrollbar_thumb_style,
            presentation_rect,
            dt,
            mouse_x,
            mouse_y,
            mouse_in_view,
            mouse_down,
            mouse_pressed,
            mouse_released,
            alpha);
    }

    scroll_offset_ = std::round(max_scroll * scrollbar_.progress());
}

float ListView::resolve_scrollbar_thickness(const ControlStyle& track_style, const ControlStyle& thumb_style) const
{
    float thickness = orientation_ == ListViewOrientation::Vertical
        ? std::max(scaled_frame_width(track_style), scaled_frame_width(thumb_style))
        : std::max(scaled_frame_height(track_style), scaled_frame_height(thumb_style));
    const SDL_Point track_min = track_style.min_size();
    const SDL_Point thumb_min = thumb_style.min_size();
    if (orientation_ == ListViewOrientation::Vertical) {
        if (track_min.x > 0) {
            thickness = std::max(thickness, static_cast<float>(track_min.x));
        }
        if (thumb_min.x > 0) {
            thickness = std::max(thickness, static_cast<float>(thumb_min.x));
        }
    } else {
        if (track_min.y > 0) {
            thickness = std::max(thickness, static_cast<float>(track_min.y));
        }
        if (thumb_min.y > 0) {
            thickness = std::max(thickness, static_cast<float>(thumb_min.y));
        }
    }
    return std::max(12.0f, std::ceil(thickness));
}

} // namespace ui
} // namespace zg
