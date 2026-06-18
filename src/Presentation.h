#pragma once

#include "Constants.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

namespace zg {

inline SDL_FRect compute_aspect_rect(int output_width, int output_height, int design_width, int design_height)
{
    const float scale = std::min(
        static_cast<float>(output_width) / static_cast<float>(design_width),
        static_cast<float>(output_height) / static_cast<float>(design_height));
    const float width = static_cast<float>(design_width) * scale;
    const float height = static_cast<float>(design_height) * scale;
    return SDL_FRect{
        std::floor((static_cast<float>(output_width) - width) * 0.5f),
        std::floor((static_cast<float>(output_height) - height) * 0.5f),
        width,
        height
    };
}

inline SDL_FRect compute_presentation_rect(int output_width, int output_height)
{
    return compute_aspect_rect(
        output_width,
        output_height,
        kGameplayPresentationWidth,
        kGameplayPresentationHeight);
}

inline float ui_presentation_scale_x(const SDL_FRect& rect)
{
    return rect.w / static_cast<float>(kUiDesignWidth);
}

inline float ui_presentation_scale_y(const SDL_FRect& rect)
{
    return rect.h / static_cast<float>(kUiDesignHeight);
}

inline float ui_presentation_scale(const SDL_FRect& rect)
{
    return std::min(ui_presentation_scale_x(rect), ui_presentation_scale_y(rect));
}

inline float presentation_scale(const SDL_FRect& rect)
{
    return rect.w / kGameplayViewWidth;
}

inline bool window_to_logical(float window_x, float window_y, const SDL_FRect& rect, float* logical_x, float* logical_y)
{
    if (rect.w <= 0.0f || rect.h <= 0.0f) {
        return false;
    }
    if (window_x < rect.x || window_x > rect.x + rect.w || window_y < rect.y || window_y > rect.y + rect.h) {
        return false;
    }

    const float scale = presentation_scale(rect);
    *logical_x = (window_x - rect.x) / scale;
    *logical_y = (window_y - rect.y) / scale;
    return true;
}

inline bool window_to_ui_logical(float window_x, float window_y, const SDL_FRect& rect, float* logical_x, float* logical_y)
{
    if (rect.w <= 0.0f || rect.h <= 0.0f) {
        return false;
    }
    if (window_x < rect.x || window_x > rect.x + rect.w || window_y < rect.y || window_y > rect.y + rect.h) {
        return false;
    }

    const float x_scale = ui_presentation_scale_x(rect);
    const float y_scale = ui_presentation_scale_y(rect);
    if (x_scale <= 0.0f || y_scale <= 0.0f) {
        return false;
    }

    *logical_x = (window_x - rect.x) / x_scale;
    *logical_y = (window_y - rect.y) / y_scale;
    return true;
}

inline SDL_FRect logical_to_present_rect(const SDL_FRect& logical_rect, const SDL_FRect& presentation_rect)
{
    const float scale = presentation_scale(presentation_rect);
    return SDL_FRect{
        presentation_rect.x + logical_rect.x * scale,
        presentation_rect.y + logical_rect.y * scale,
        logical_rect.w * scale,
        logical_rect.h * scale
    };
}

inline SDL_FRect present_to_logical_rect(const SDL_FRect& present_rect, const SDL_FRect& presentation_rect)
{
    const float scale = presentation_scale(presentation_rect);
    if (scale <= 0.0f) {
        return SDL_FRect{};
    }
    return SDL_FRect{
        (present_rect.x - presentation_rect.x) / scale,
        (present_rect.y - presentation_rect.y) / scale,
        present_rect.w / scale,
        present_rect.h / scale
    };
}

inline SDL_FRect ui_logical_to_present_rect(const SDL_FRect& logical_rect, const SDL_FRect& presentation_rect)
{
    const float x_scale = ui_presentation_scale_x(presentation_rect);
    const float y_scale = ui_presentation_scale_y(presentation_rect);
    return SDL_FRect{
        presentation_rect.x + logical_rect.x * x_scale,
        presentation_rect.y + logical_rect.y * y_scale,
        logical_rect.w * x_scale,
        logical_rect.h * y_scale
    };
}

inline SDL_FRect ui_present_to_logical_rect(const SDL_FRect& present_rect, const SDL_FRect& presentation_rect)
{
    const float x_scale = ui_presentation_scale_x(presentation_rect);
    const float y_scale = ui_presentation_scale_y(presentation_rect);
    if (x_scale <= 0.0f || y_scale <= 0.0f) {
        return SDL_FRect{};
    }
    return SDL_FRect{
        (present_rect.x - presentation_rect.x) / x_scale,
        (present_rect.y - presentation_rect.y) / y_scale,
        present_rect.w / x_scale,
        present_rect.h / y_scale
    };
}

inline SDL_FPoint logical_to_present_point(float logical_x, float logical_y, const SDL_FRect& presentation_rect)
{
    const float scale = presentation_scale(presentation_rect);
    return SDL_FPoint{
        presentation_rect.x + logical_x * scale,
        presentation_rect.y + logical_y * scale
    };
}

} // namespace zg
