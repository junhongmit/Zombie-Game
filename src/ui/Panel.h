#pragma once

#include "ControlStyle.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

namespace zg {
namespace ui {

class Panel {
public:
    Panel() = default;
    Panel(float x, float y, float w, float h);

    Panel& set_title(const char* title);
    Panel& set_enabled(bool enabled);

    void render(
        SDL_Renderer* renderer,
        const ControlStyle& skin,
        TTF_Font* font,
        const SDL_FRect& presentation_rect,
        SDL_Color text_color,
        Uint8 alpha = 255) const;

    SDL_FRect content_rect(const ControlStyle& skin, const SDL_FRect& presentation_rect) const;
    const SDL_FRect& logical_rect() const { return logical_rect_; }
    const std::string& title() const { return title_; }
    bool enabled() const { return enabled_; }

private:
    std::string title_;
    SDL_FRect logical_rect_{};
    bool enabled_ = true;
};

} // namespace ui
} // namespace zg
