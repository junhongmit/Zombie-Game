#pragma once

#include "Container.h"
#include "ControlStyle.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

namespace zg {
namespace ui {

class Panel : public Container {
public:
    Panel() = default;
    Panel(float x, float y, float w, float h);

    Panel& set_title(const char* title);
    Panel& set_enabled(bool enabled);
    Panel& set_header_height(float height);
    Panel& set_footer_height(float height);
    Panel& set_style(const ControlStyle* skin);
    Panel& set_font(TTF_Font* font);
    Panel& set_text_color(SDL_Color color);
    Panel& set_alpha(Uint8 alpha);

    void render(
        SDL_Renderer* renderer,
        const ControlStyle& skin,
        TTF_Font* font,
        const SDL_FRect& presentation_rect,
        SDL_Color text_color,
        Uint8 alpha = 255) const;
    void render(const RenderContext& context, const SDL_FRect& parent_rect) const override;

    SDL_FRect content_rect(const ControlStyle& skin, const SDL_FRect& presentation_rect) const;
    SDL_FRect slot_rect(const std::string& slot_name, const SDL_FRect& resolved_rect) const override;
    const std::string& title() const { return title_; }
    bool enabled() const { return enabled_; }

private:
    std::string title_;
    bool enabled_ = true;
    float header_height_ = 36.0f;
    float footer_height_ = 0.0f;
    const ControlStyle* skin_ = nullptr;
    TTF_Font* font_ = nullptr;
    SDL_Color text_color_{255, 255, 255, 255};
    Uint8 alpha_ = 255;
};

} // namespace ui
} // namespace zg
