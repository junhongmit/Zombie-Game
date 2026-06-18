#include "ProgressBar.h"

#include "../MathUtil.h"
#include "../Presentation.h"

#include <algorithm>

namespace zg {
namespace ui {

ProgressBar::ProgressBar(float x, float y, float w, float h, ProgressBarOrientation orientation)
    : logical_rect_{x, y, w, h},
      orientation_(orientation)
{
}

ProgressBar& ProgressBar::set_progress(float progress)
{
    progress_ = clamp_float(progress, 0.0f, 1.0f);
    return *this;
}

ProgressBar& ProgressBar::set_enabled(bool enabled)
{
    enabled_ = enabled;
    return *this;
}

ProgressBar& ProgressBar::set_fill_origin(ProgressBarFillOrigin origin)
{
    fill_origin_ = origin;
    return *this;
}

SDL_FRect ProgressBar::fill_present_rect(
    const ControlStyle& track_style,
    const ControlStyle& fill_style,
    const SDL_FRect& presentation_rect) const
{
    const ControlVisualState state = enabled_ ? ControlVisualState::Normal : ControlVisualState::Disabled;
    const SDL_FRect track_rect = ui_logical_to_present_rect(logical_rect_, presentation_rect);
    const ControlRect* base_region = track_style.track_region() != nullptr
        ? track_style.track_region()
        : (track_style.channel_region() != nullptr ? track_style.channel_region() : nullptr);
    const SDL_FRect base_fill_rect = base_region != nullptr
        ? track_style.map_region(*base_region, track_rect, state)
        : track_style.content_rect(track_rect);

    if (orientation_ == ProgressBarOrientation::Horizontal) {
        const float fill_w = std::round(base_fill_rect.w * progress_);
        if (fill_origin_ == ProgressBarFillOrigin::Start) {
            return SDL_FRect{base_fill_rect.x, base_fill_rect.y, fill_w, base_fill_rect.h};
        }
        return SDL_FRect{
            base_fill_rect.x + std::max(0.0f, base_fill_rect.w - fill_w),
            base_fill_rect.y,
            fill_w,
            base_fill_rect.h
        };
    }

    const float fill_h = std::round(base_fill_rect.h * progress_);
    if (fill_origin_ == ProgressBarFillOrigin::Start) {
        return SDL_FRect{base_fill_rect.x, base_fill_rect.y, base_fill_rect.w, fill_h};
    }
    return SDL_FRect{
        base_fill_rect.x,
        base_fill_rect.y + std::max(0.0f, base_fill_rect.h - fill_h),
        base_fill_rect.w,
        fill_h
    };
}

void ProgressBar::render(
    SDL_Renderer* renderer,
    const ControlStyle& track_style,
    const ControlStyle& fill_style,
    const SDL_FRect& presentation_rect,
    Uint8 alpha) const
{
    const ControlVisualState state = enabled_ ? ControlVisualState::Normal : ControlVisualState::Disabled;
    const SDL_FRect track_rect = ui_logical_to_present_rect(logical_rect_, presentation_rect);
    track_style.render(renderer, track_rect, state, alpha);

    const SDL_FRect fill_rect = fill_present_rect(track_style, fill_style, presentation_rect);
    if (fill_rect.w > 0.0f && fill_rect.h > 0.0f) {
        fill_style.render(renderer, fill_rect, state, alpha);
    }
}

} // namespace ui
} // namespace zg
