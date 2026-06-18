#pragma once

#include "ControlStyle.h"

#include <SDL3/SDL.h>

namespace zg {
namespace ui {

enum class ProgressBarOrientation {
    Horizontal,
    Vertical
};

enum class ProgressBarFillOrigin {
    Start,
    End
};

class ProgressBar {
public:
    ProgressBar() = default;
    ProgressBar(float x, float y, float w, float h, ProgressBarOrientation orientation = ProgressBarOrientation::Horizontal);

    ProgressBar& set_progress(float progress);
    ProgressBar& set_enabled(bool enabled);
    ProgressBar& set_fill_origin(ProgressBarFillOrigin origin);

    void render(
        SDL_Renderer* renderer,
        const ControlStyle& track_style,
        const ControlStyle& fill_style,
        const SDL_FRect& presentation_rect,
        Uint8 alpha = 255) const;

    float progress() const { return progress_; }
    const SDL_FRect& logical_rect() const { return logical_rect_; }
    bool enabled() const { return enabled_; }

private:
    SDL_FRect fill_present_rect(
        const ControlStyle& track_style,
        const ControlStyle& fill_style,
        const SDL_FRect& presentation_rect) const;

    SDL_FRect logical_rect_{};
    ProgressBarOrientation orientation_ = ProgressBarOrientation::Horizontal;
    ProgressBarFillOrigin fill_origin_ = ProgressBarFillOrigin::Start;
    float progress_ = 0.0f;
    bool enabled_ = true;
};

} // namespace ui
} // namespace zg
