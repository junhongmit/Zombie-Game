#pragma once

#include "ControlStyle.h"

#include <SDL3/SDL.h>

namespace zg {
namespace ui {

enum class ScrollbarOrientation {
    Vertical,
    Horizontal
};

enum class ScrollbarFillOrigin {
    Start,
    End
};

class Scrollbar {
public:
    Scrollbar() = default;
    Scrollbar(float x, float y, float w, float h, ScrollbarOrientation orientation);

    Scrollbar& set_progress(float progress);
    Scrollbar& set_visible_fraction(float visible_fraction);
    Scrollbar& set_enabled(bool enabled);
    Scrollbar& set_rect(float x, float y, float w, float h);
    Scrollbar& set_track_cross_ratio(float ratio);
    Scrollbar& set_fill_origin(ScrollbarFillOrigin origin);
    Scrollbar& nudge(float delta);
    float progress() const { return progress_; }

    bool update_and_render(
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
        Uint8 alpha = 255);

private:
    SDL_FRect track_present_rect(const SDL_FRect& presentation_rect) const;
    SDL_FRect interaction_present_rect(
        const ControlStyle& track_style,
        const SDL_FRect& presentation_rect) const;
    SDL_FRect thumb_present_rect(
        const ControlStyle& track_style,
        const ControlStyle& thumb_style,
        const SDL_FRect& presentation_rect,
        float progress) const;

    SDL_FRect logical_rect_{};
    ScrollbarOrientation orientation_ = ScrollbarOrientation::Vertical;
    float progress_ = 0.0f;
    float visible_fraction_ = 0.25f;
    float track_cross_ratio_ = 1.0f;
    bool has_track_cross_ratio_override_ = false;
    float thumb_alpha_ = 0.0f;
    float drag_pointer_offset_ = 0.0f;
    float thumb_reveal_timer_ = 0.0f;
    bool enabled_ = true;
    bool dragging_ = false;
    ScrollbarFillOrigin fill_origin_ = ScrollbarFillOrigin::Start;
};

} // namespace ui
} // namespace zg
