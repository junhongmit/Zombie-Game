#include "Panel.h"

#include "../Presentation.h"

#include <cmath>

namespace zg {
namespace ui {

namespace {

void render_panel_title(
    SDL_Renderer* renderer,
    const ControlStyle& skin,
    TTF_Font* font,
    const char* text,
    const SDL_FRect& screen_rect,
    ControlVisualState state,
    SDL_Color color)
{
    if (font == nullptr || text == nullptr || text[0] == '\0') {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, color);
    if (surface == nullptr) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    const SDL_FRect title_rect = skin.title_region() != nullptr
        ? skin.map_region(*skin.title_region(), screen_rect, state)
        : skin.content_rect(screen_rect);
    const SDL_FRect dst{
        title_rect.x,
        title_rect.y + std::floor((title_rect.h - static_cast<float>(surface->h)) * 0.5f),
        std::min(title_rect.w, static_cast<float>(surface->w)),
        static_cast<float>(surface->h)
    };
    SDL_RenderTexture(renderer, texture, nullptr, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

} // namespace

Panel::Panel(float x, float y, float w, float h)
    : logical_rect_{x, y, w, h}
{
}

Panel& Panel::set_title(const char* title)
{
    title_ = title != nullptr ? title : "";
    return *this;
}

Panel& Panel::set_enabled(bool enabled)
{
    enabled_ = enabled;
    return *this;
}

void Panel::render(
    SDL_Renderer* renderer,
    const ControlStyle& skin,
    TTF_Font* font,
    const SDL_FRect& presentation_rect,
    SDL_Color text_color,
    Uint8 alpha) const
{
    const ControlVisualState state = enabled_ ? ControlVisualState::Normal : ControlVisualState::Disabled;
    const SDL_FRect screen_rect = ui_logical_to_present_rect(logical_rect_, presentation_rect);
    skin.render(renderer, screen_rect, state, alpha);
    if (!title_.empty()) {
        text_color.a = alpha;
        render_panel_title(renderer, skin, font, title_.c_str(), screen_rect, state, text_color);
    }
}

SDL_FRect Panel::content_rect(const ControlStyle& skin, const SDL_FRect& presentation_rect) const
{
    return skin.content_rect(ui_logical_to_present_rect(logical_rect_, presentation_rect));
}

} // namespace ui
} // namespace zg
