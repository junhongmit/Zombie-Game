#include "Scrollbar.h"

#include "../MathUtil.h"
#include "../Presentation.h"

#include <algorithm>
#include <cmath>

namespace zg {
namespace ui {

namespace {

constexpr float kScrollbarMinThumbPixels = 18.0f;
constexpr float kScrollbarFadeInSpeed = 10.0f;
constexpr float kScrollbarFadeOutSpeed = 7.0f;
constexpr float kScrollbarRevealDuration = 1.8f;

bool point_in_rect(float x, float y, const SDL_FRect& rect)
{
    return x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h;
}

float approach(float current, float target, float delta)
{
    if (current < target) {
        return std::min(target, current + delta);
    }
    return std::max(target, current - delta);
}

} // namespace

Scrollbar::Scrollbar(float x, float y, float w, float h, ScrollbarOrientation orientation)
    : logical_rect_{x, y, w, h},
      orientation_(orientation)
{
}

Scrollbar& Scrollbar::set_progress(float progress)
{
    progress_ = clamp_float(progress, 0.0f, 1.0f);
    return *this;
}

Scrollbar& Scrollbar::set_visible_fraction(float visible_fraction)
{
    visible_fraction_ = clamp_float(visible_fraction, 0.05f, 1.0f);
    return *this;
}

Scrollbar& Scrollbar::set_enabled(bool enabled)
{
    enabled_ = enabled;
    if (!enabled_) {
        dragging_ = false;
    }
    return *this;
}

Scrollbar& Scrollbar::set_rect(float x, float y, float w, float h)
{
    logical_rect_ = SDL_FRect{x, y, w, h};
    return *this;
}

Scrollbar& Scrollbar::set_track_cross_ratio(float ratio)
{
    track_cross_ratio_ = clamp_float(ratio, 0.1f, 1.0f);
    has_track_cross_ratio_override_ = true;
    return *this;
}

Scrollbar& Scrollbar::set_fill_origin(ScrollbarFillOrigin origin)
{
    fill_origin_ = origin;
    return *this;
}

Scrollbar& Scrollbar::nudge(float delta)
{
    progress_ = clamp_float(progress_ + delta, 0.0f, 1.0f);
    if (enabled_ && std::fabs(delta) > 0.0001f) {
        thumb_reveal_timer_ = kScrollbarRevealDuration;
    }
    return *this;
}

SDL_FRect Scrollbar::track_present_rect(const SDL_FRect& presentation_rect) const
{
    SDL_FRect rect = ui_logical_to_present_rect(logical_rect_, presentation_rect);
    const float ratio = clamp_float(track_cross_ratio_, 0.1f, 1.0f);
    if (orientation_ == ScrollbarOrientation::Vertical) {
        const float shrunk_w = std::round(rect.w * ratio);
        rect.x += std::floor((rect.w - shrunk_w) * 0.5f);
        rect.w = shrunk_w;
    } else {
        const float shrunk_h = std::round(rect.h * ratio);
        rect.y += std::floor((rect.h - shrunk_h) * 0.5f);
        rect.h = shrunk_h;
    }
    return rect;
}

SDL_FRect Scrollbar::interaction_present_rect(
    const ControlStyle& track_style,
    const SDL_FRect& presentation_rect) const
{
    const SDL_FRect track_rect = track_present_rect(presentation_rect);
    const ControlVisualState base_state = enabled_ ? ControlVisualState::Normal : ControlVisualState::Disabled;
    const ControlRect* thumb_bounds_region = track_style.thumb_bounds_region() != nullptr
        ? track_style.thumb_bounds_region()
        : (track_style.channel_region() != nullptr ? track_style.channel_region() : track_style.track_region());
    return thumb_bounds_region != nullptr
        ? track_style.map_region(*thumb_bounds_region, track_rect, base_state)
        : track_rect;
}

SDL_FRect Scrollbar::thumb_present_rect(
    const ControlStyle& track_style,
    const ControlStyle& thumb_style,
    const SDL_FRect& presentation_rect,
    float progress) const
{
    const SDL_FRect screen_rect = track_present_rect(presentation_rect);
    const ControlVisualState base_state = enabled_ ? ControlVisualState::Normal : ControlVisualState::Disabled;
    const ControlRect* travel_region = track_style.channel_region() != nullptr
        ? track_style.channel_region()
        : track_style.track_region();
    const ControlRect* thumb_bounds_region = track_style.thumb_bounds_region() != nullptr
        ? track_style.thumb_bounds_region()
        : travel_region;

    const SDL_FRect channel_rect = travel_region != nullptr
        ? track_style.map_region(*travel_region, screen_rect, base_state)
        : screen_rect;
    const SDL_FRect thumb_bounds_rect = thumb_bounds_region != nullptr
        ? track_style.map_region(*thumb_bounds_region, screen_rect, base_state)
        : channel_rect;

    const float scale = ui_presentation_scale(presentation_rect);
    const SDL_Point thumb_min = thumb_style.min_size();
    const SDL_Point thumb_max = thumb_style.max_size();
    const bool has_thumb_max = thumb_style.has_max_size();
    const float clamped_visible = clamp_float(visible_fraction_, 0.05f, 1.0f);
    const float clamped_progress = clamp_float(progress, 0.0f, 1.0f);

    if (orientation_ == ScrollbarOrientation::Vertical) {
        const float available_h = thumb_bounds_rect.h;
        float thumb_h = std::max(kScrollbarMinThumbPixels, std::round(available_h * clamped_visible));
        float thumb_w = std::max(channel_rect.w, thumb_min.x * scale);
        if (has_thumb_max && thumb_max.y > 0) {
            thumb_h = std::min(thumb_h, thumb_max.y * scale);
        }
        if (has_thumb_max && thumb_max.x > 0) {
            thumb_w = std::min(thumb_w, thumb_max.x * scale);
        }
        const float center_min = std::max(
            thumb_bounds_rect.y,
            screen_rect.y + thumb_h * 0.5f);
        const float center_max = std::min(
            thumb_bounds_rect.y + thumb_bounds_rect.h,
            screen_rect.y + screen_rect.h - thumb_h * 0.5f);
        const float travel = std::max(0.0f, center_max - center_min);
        const float thumb_center_y = center_min + std::round(travel * clamped_progress);
        const float thumb_y = thumb_center_y - thumb_h * 0.5f;
        const float thumb_x = channel_rect.x + std::floor((channel_rect.w - thumb_w) * 0.5f);
        return SDL_FRect{thumb_x, thumb_y, thumb_w, thumb_h};
    }

    const float available_w = thumb_bounds_rect.w;
    float thumb_w = std::max(kScrollbarMinThumbPixels, std::round(available_w * clamped_visible));
    float thumb_h = std::max(channel_rect.h, thumb_min.y * scale);
    if (has_thumb_max && thumb_max.x > 0) {
        thumb_w = std::min(thumb_w, thumb_max.x * scale);
    }
    if (has_thumb_max && thumb_max.y > 0) {
        thumb_h = std::min(thumb_h, thumb_max.y * scale);
    }
    const float center_min = std::max(
        thumb_bounds_rect.x,
        screen_rect.x + thumb_w * 0.5f);
    const float center_max = std::min(
        thumb_bounds_rect.x + thumb_bounds_rect.w,
        screen_rect.x + screen_rect.w - thumb_w * 0.5f);
    const float travel = std::max(0.0f, center_max - center_min);
    const float thumb_center_x = center_min + std::round(travel * clamped_progress);
    const float thumb_x = thumb_center_x - thumb_w * 0.5f;
    const float thumb_y = channel_rect.y + std::floor((channel_rect.h - thumb_h) * 0.5f);
    return SDL_FRect{thumb_x, thumb_y, thumb_w, thumb_h};
}

bool Scrollbar::update_and_render(
    SDL_Renderer* renderer,
    const ControlStyle& track_style,
    const ControlStyle& fill_style,
    const ControlStyle& thumb_style,
    const SDL_FRect& presentation_rect,
    float dt,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool mouse_pressed,
    bool mouse_released,
    Uint8 alpha)
{
    if (!has_track_cross_ratio_override_ && track_style.has_track_cross_ratio()) {
        track_cross_ratio_ = track_style.track_cross_ratio();
    }

    const SDL_FRect track_rect = track_present_rect(presentation_rect);
    const SDL_FRect interaction_rect = interaction_present_rect(track_style, presentation_rect);
    const SDL_FRect interaction_logical = ui_present_to_logical_rect(interaction_rect, presentation_rect);
    SDL_FRect thumb_rect = thumb_present_rect(track_style, thumb_style, presentation_rect, progress_);
    SDL_FRect thumb_logical = ui_present_to_logical_rect(thumb_rect, presentation_rect);

    const bool track_hovered = mouse_in_view && point_in_rect(mouse_x, mouse_y, interaction_logical);
    const bool thumb_hovered = mouse_in_view && point_in_rect(mouse_x, mouse_y, thumb_logical);

    if (enabled_ && mouse_pressed && thumb_hovered) {
        dragging_ = true;
        drag_pointer_offset_ = orientation_ == ScrollbarOrientation::Vertical
            ? (mouse_y - thumb_logical.y)
            : (mouse_x - thumb_logical.x);
    }
    if (mouse_released) {
        dragging_ = false;
    }
    if (thumb_hovered || track_hovered || dragging_) {
        thumb_reveal_timer_ = kScrollbarRevealDuration;
    } else if (thumb_reveal_timer_ > 0.0f) {
        thumb_reveal_timer_ = std::max(0.0f, thumb_reveal_timer_ - dt);
    }

    bool changed = false;
    const ControlVisualState base_state = enabled_ ? ControlVisualState::Normal : ControlVisualState::Disabled;
    const ControlRect* channel_region = track_style.channel_region() != nullptr
        ? track_style.channel_region()
        : track_style.track_region();
    const ControlRect* thumb_bounds_region = track_style.thumb_bounds_region() != nullptr
        ? track_style.thumb_bounds_region()
        : channel_region;
    const ControlRect* fill_region = fill_style.fill_track_region() != nullptr
        ? fill_style.fill_track_region()
        : (fill_style.channel_region() != nullptr ? fill_style.channel_region() : fill_style.track_region());

    const SDL_FRect channel_rect = channel_region != nullptr
        ? track_style.map_region(*channel_region, track_rect, base_state)
        : track_rect;
    const SDL_FRect thumb_bounds_rect = thumb_bounds_region != nullptr
        ? track_style.map_region(*thumb_bounds_region, track_rect, base_state)
        : channel_rect;
    const SDL_FRect fill_base_rect = channel_rect;

    if (enabled_ && dragging_) {
        const SDL_FRect bounds_logical = ui_present_to_logical_rect(thumb_bounds_rect, presentation_rect);
        if (orientation_ == ScrollbarOrientation::Vertical) {
            const float thumb_h = thumb_logical.h;
            const float desired_top = mouse_y - drag_pointer_offset_;
            const SDL_FRect track_logical = ui_present_to_logical_rect(track_rect, presentation_rect);
            const float center_min = std::max(
                bounds_logical.y,
                track_logical.y + thumb_h * 0.5f);
            const float center_max = std::min(
                bounds_logical.y + bounds_logical.h,
                track_logical.y + track_logical.h - thumb_h * 0.5f);
            const float top_min = center_min - thumb_h * 0.5f;
            const float available = std::max(0.0f, center_max - center_min);
            const float local = clamp_float(desired_top - top_min, 0.0f, available);
            const float next = available > 0.0f ? (local / available) : 0.0f;
            if (std::fabs(next - progress_) > 0.0001f) {
                progress_ = next;
                changed = true;
            }
        } else {
            const float thumb_w = thumb_logical.w;
            const float desired_left = mouse_x - drag_pointer_offset_;
            const SDL_FRect track_logical = ui_present_to_logical_rect(track_rect, presentation_rect);
            const float center_min = std::max(
                bounds_logical.x,
                track_logical.x + thumb_w * 0.5f);
            const float center_max = std::min(
                bounds_logical.x + bounds_logical.w,
                track_logical.x + track_logical.w - thumb_w * 0.5f);
            const float left_min = center_min - thumb_w * 0.5f;
            const float available = std::max(0.0f, center_max - center_min);
            const float local = clamp_float(desired_left - left_min, 0.0f, available);
            const float next = available > 0.0f ? (local / available) : 0.0f;
            if (std::fabs(next - progress_) > 0.0001f) {
                progress_ = next;
                changed = true;
            }
        }
        thumb_rect = thumb_present_rect(track_style, thumb_style, presentation_rect, progress_);
        thumb_logical = ui_present_to_logical_rect(thumb_rect, presentation_rect);
    }

    const bool thumb_visible = enabled_ && (track_hovered || thumb_hovered || dragging_ || thumb_reveal_timer_ > 0.0f);
    const float target_alpha = thumb_visible ? 1.0f : 0.0f;
    const float fade_speed = target_alpha > thumb_alpha_ ? kScrollbarFadeInSpeed : kScrollbarFadeOutSpeed;
    thumb_alpha_ = approach(thumb_alpha_, target_alpha, dt * fade_speed);

    track_style.render(renderer, track_rect, base_state, alpha);

    SDL_FRect fill_rect = fill_base_rect;
    if (orientation_ == ScrollbarOrientation::Vertical) {
        const float thumb_center = thumb_rect.y + thumb_rect.h * 0.5f;
        if (fill_origin_ == ScrollbarFillOrigin::Start) {
            const float fill_h = std::max(0.0f, std::min(fill_base_rect.y + fill_base_rect.h, thumb_center) - fill_base_rect.y);
            fill_rect = SDL_FRect{fill_base_rect.x, fill_base_rect.y, fill_base_rect.w, fill_h};
        } else {
            const float bottom = fill_base_rect.y + fill_base_rect.h;
            const float fill_y = std::min(bottom, std::max(fill_base_rect.y, thumb_center));
            const float fill_h = std::max(0.0f, bottom - fill_y);
            fill_rect = SDL_FRect{fill_base_rect.x, fill_y, fill_base_rect.w, fill_h};
        }
    } else {
        const float thumb_center = thumb_rect.x + thumb_rect.w * 0.5f;
        if (fill_origin_ == ScrollbarFillOrigin::Start) {
            const float fill_w = std::max(0.0f, std::min(fill_base_rect.x + fill_base_rect.w, thumb_center) - fill_base_rect.x);
            fill_rect = SDL_FRect{fill_base_rect.x, fill_base_rect.y, fill_w, fill_base_rect.h};
        } else {
            const float right = fill_base_rect.x + fill_base_rect.w;
            const float fill_x = std::min(right, std::max(fill_base_rect.x, thumb_center));
            const float fill_w = std::max(0.0f, right - fill_x);
            fill_rect = SDL_FRect{fill_x, fill_base_rect.y, fill_w, fill_base_rect.h};
        }
    }
    if (fill_region != nullptr) {
        const SDL_FRect fill_style_rect = fill_style.fit_region_to_rect(*fill_region, fill_base_rect, base_state);
        const SDL_Rect clip_rect{
            static_cast<int>(std::floor(fill_rect.x)),
            static_cast<int>(std::floor(fill_rect.y)),
            static_cast<int>(std::ceil(fill_rect.w)),
            static_cast<int>(std::ceil(fill_rect.h))
        };
        SDL_SetRenderClipRect(renderer, &clip_rect);
        fill_style.render(renderer, fill_style_rect, base_state, alpha);
        SDL_SetRenderClipRect(renderer, nullptr);
    } else {
        fill_style.render(renderer, fill_rect, base_state, alpha);
    }

    if (thumb_alpha_ > 0.001f) {
        ControlVisualState thumb_state = ControlVisualState::Normal;
        if (!enabled_) {
            thumb_state = ControlVisualState::Disabled;
        } else if (dragging_) {
            thumb_state = ControlVisualState::Pressed;
        } else if (thumb_hovered) {
            thumb_state = mouse_down ? ControlVisualState::Pressed : ControlVisualState::Hover;
        }
        const Uint8 thumb_alpha = static_cast<Uint8>(std::round(alpha * thumb_alpha_));
        thumb_style.render(renderer, thumb_rect, thumb_state, thumb_alpha);
        thumb_style.render_grip(renderer, thumb_rect, thumb_state, ui_presentation_scale(presentation_rect), thumb_alpha);
    }

    return changed;
}

} // namespace ui
} // namespace zg
