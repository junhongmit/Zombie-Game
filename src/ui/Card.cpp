#include "Card.h"

#include "../Presentation.h"
#include "../Texture.h"

#include <algorithm>
#include <cmath>

namespace zg {
namespace ui {

namespace {

struct TextSprite {
    SDL_Texture* texture = nullptr;
    float width = 0.0f;
    float height = 0.0f;
};

bool point_in_rect(float x, float y, const SDL_FRect& rect)
{
    return x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h;
}

TextSprite make_text_sprite(
    SDL_Renderer* renderer,
    TTF_Font* font,
    const char* text,
    SDL_Color color)
{
    if (font == nullptr || text == nullptr || text[0] == '\0') {
        return {};
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, color);
    if (surface == nullptr) {
        return {};
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        SDL_DestroySurface(surface);
        return {};
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    TextSprite sprite;
    sprite.texture = texture;
    sprite.width = static_cast<float>(surface->w);
    sprite.height = static_cast<float>(surface->h);
    SDL_DestroySurface(surface);
    return sprite;
}

void destroy_text_sprite(TextSprite* sprite)
{
    if (sprite->texture != nullptr) {
        SDL_DestroyTexture(sprite->texture);
        sprite->texture = nullptr;
    }
}

void render_text_sprite(SDL_Renderer* renderer, const TextSprite& sprite, const SDL_FRect& dst)
{
    if (sprite.texture == nullptr || dst.w <= 0.0f || dst.h <= 0.0f) {
        return;
    }
    SDL_RenderTexture(renderer, sprite.texture, nullptr, &dst);
}

SDL_FRect inset_rect(const SDL_FRect& rect, float amount)
{
    return SDL_FRect{
        rect.x + amount,
        rect.y + amount,
        std::max(0.0f, rect.w - amount * 2.0f),
        std::max(0.0f, rect.h - amount * 2.0f)
    };
}

void render_text_in_region(SDL_Renderer* renderer, const TextSprite& sprite, const SDL_FRect& region)
{
    if (sprite.texture == nullptr || region.w <= 0.0f || region.h <= 0.0f) {
        return;
    }

    SDL_FRect dst{
        region.x,
        region.y + std::floor((region.h - sprite.height) * 0.5f),
        std::min(region.w, sprite.width),
        sprite.height
    };
    render_text_sprite(renderer, sprite, dst);
}

} // namespace

Card::Card(float x, float y, float w, float h, bool enabled)
    : logical_rect_{x, y, w, h},
      enabled_(enabled)
{
}

Card& Card::set_enabled(bool enabled)
{
    enabled_ = enabled;
    return *this;
}

Card& Card::set_selected(bool selected)
{
    selected_ = selected;
    return *this;
}

Card& Card::set_title(const char* title)
{
    title_ = title != nullptr ? title : "";
    return *this;
}

Card& Card::set_subtitle(const char* subtitle)
{
    subtitle_ = subtitle != nullptr ? subtitle : "";
    return *this;
}

Card& Card::set_meta(const char* meta)
{
    meta_ = meta != nullptr ? meta : "";
    return *this;
}

Card& Card::set_icon(const Texture* icon)
{
    icon_ = icon;
    return *this;
}

bool Card::contains(float x, float y, bool mouse_in_view) const
{
    return enabled_ && mouse_in_view && point_in_rect(x, y, logical_rect_);
}

ControlVisualState Card::visual_state(
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool armed) const
{
    if (!enabled_) {
        return ControlVisualState::Disabled;
    }
    if (selected_) {
        return ControlVisualState::Pressed;
    }

    const bool hovered = contains(mouse_x, mouse_y, mouse_in_view);
    if (mouse_down && armed && hovered) {
        return ControlVisualState::Pressed;
    }
    if (hovered) {
        return ControlVisualState::Hover;
    }
    return ControlVisualState::Normal;
}

void Card::render(
    SDL_Renderer* renderer,
    const ControlStyle& skin,
    TTF_Font* title_font,
    TTF_Font* subtitle_font,
    const SDL_FRect& presentation_rect,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool armed,
    SDL_Color title_color,
    SDL_Color subtitle_color,
    SDL_Color meta_color,
    Uint8 alpha) const
{
    const ControlVisualState state = visual_state(mouse_x, mouse_y, mouse_in_view, mouse_down, armed);
    const SDL_FRect screen_rect = ui_logical_to_present_rect(logical_rect_, presentation_rect);
    skin.render(renderer, screen_rect, state, alpha);

    const SDL_FRect content_rect = skin.content_rect(screen_rect);
    const SDL_FRect icon_rect = skin.icon_region() != nullptr
        ? skin.map_region(*skin.icon_region(), screen_rect, state)
        : inset_rect(content_rect, 4.0f);
    const SDL_FRect title_rect = skin.title_region() != nullptr
        ? skin.map_region(*skin.title_region(), screen_rect, state)
        : content_rect;
    const SDL_FRect subtitle_rect = skin.subtitle_region() != nullptr
        ? skin.map_region(*skin.subtitle_region(), screen_rect, state)
        : content_rect;
    const SDL_FRect meta_rect = skin.meta_region() != nullptr
        ? skin.map_region(*skin.meta_region(), screen_rect, state)
        : content_rect;

    if (icon_ != nullptr && icon_->valid() && icon_rect.w > 0.0f && icon_rect.h > 0.0f) {
        const float source_w = icon_->width() * 0.5f;
        const float source_h = icon_->height();
        const float icon_scale = std::min(icon_rect.w / std::max(1.0f, source_w), icon_rect.h / std::max(1.0f, source_h));
        const float draw_w = std::round(source_w * icon_scale);
        const float draw_h = std::round(source_h * icon_scale);
        const SDL_FRect src{0.0f, 0.0f, source_w, source_h};
        const SDL_FRect dst{
            icon_rect.x + std::floor((icon_rect.w - draw_w) * 0.5f),
            icon_rect.y + std::floor((icon_rect.h - draw_h) * 0.5f),
            draw_w,
            draw_h
        };
        SDL_SetTextureAlphaMod(icon_->get(), alpha);
        SDL_RenderTexture(renderer, icon_->get(), &src, &dst);
        SDL_SetTextureAlphaMod(icon_->get(), 255);
    }

    title_color.a = alpha;
    subtitle_color.a = alpha;
    meta_color.a = alpha;
    TextSprite title = make_text_sprite(renderer, title_font, title_.c_str(), title_color);
    TextSprite subtitle = make_text_sprite(renderer, subtitle_font, subtitle_.c_str(), subtitle_color);
    TextSprite meta = make_text_sprite(renderer, subtitle_font, meta_.c_str(), meta_color);
    render_text_in_region(renderer, title, title_rect);
    render_text_in_region(renderer, subtitle, subtitle_rect);
    render_text_in_region(renderer, meta, meta_rect);
    destroy_text_sprite(&title);
    destroy_text_sprite(&subtitle);
    destroy_text_sprite(&meta);
}

} // namespace ui
} // namespace zg
