#include "TextItem.h"

#include "../Presentation.h"

#include <algorithm>

namespace zg {
namespace ui {

TextItem& TextItem::set_text(const char* text)
{
    text_ = text != nullptr ? text : "";
    return *this;
}

TextItem& TextItem::set_font(TTF_Font* font)
{
    font_ = font;
    return *this;
}

TextItem& TextItem::set_color(SDL_Color color)
{
    color_ = color;
    return *this;
}

TextItem& TextItem::set_fit_to_bounds(bool fit)
{
    fit_to_bounds_ = fit;
    return *this;
}

TextItem& TextItem::set_linear_filter(bool linear)
{
    linear_filter_ = linear;
    return *this;
}

TextItem& TextItem::set_horizontal_align(HorizontalAlign align)
{
    horizontal_align_ = align;
    return *this;
}

TextItem& TextItem::set_vertical_align(VerticalAlign align)
{
    vertical_align_ = align;
    return *this;
}

TextItem& TextItem::set_padding(float x, float y)
{
    padding_x_ = x;
    padding_y_ = y;
    return *this;
}

void TextItem::render(const RenderContext& context, const SDL_FRect& parent_rect) const
{
    if (context.renderer == nullptr || font_ == nullptr || text_.empty()) {
        return;
    }

    const SDL_FRect logical_rect = resolve_rect(parent_rect);
    const SDL_FRect screen_rect = ui_logical_to_present_rect(logical_rect, context.presentation_rect);
    const SDL_FRect padded_rect{
        screen_rect.x + padding_x_,
        screen_rect.y + padding_y_,
        std::max(0.0f, screen_rect.w - padding_x_ * 2.0f),
        std::max(0.0f, screen_rect.h - padding_y_ * 2.0f)
    };

    SDL_Color color = color_;
    color.a = context.alpha;
    SDL_Surface* surface = TTF_RenderText_Blended(font_, text_.c_str(), 0, color);
    if (surface == nullptr) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(context.renderer, surface);
    if (texture == nullptr) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_SetTextureScaleMode(texture, linear_filter_ ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST);
    SDL_FRect dst = padded_rect;
    if (fit_to_bounds_) {
        const float scale = std::min(
            padded_rect.w / std::max(1.0f, static_cast<float>(surface->w)),
            padded_rect.h / std::max(1.0f, static_cast<float>(surface->h)));
        dst.w = surface->w * scale;
        dst.h = surface->h * scale;
    } else {
        dst.w = static_cast<float>(surface->w);
        dst.h = static_cast<float>(surface->h);
    }

    switch (horizontal_align_) {
    case HorizontalAlign::Center:
        dst.x = padded_rect.x + (padded_rect.w - dst.w) * 0.5f;
        break;
    case HorizontalAlign::Right:
        dst.x = padded_rect.x + padded_rect.w - dst.w;
        break;
    case HorizontalAlign::Left:
    default:
        dst.x = padded_rect.x;
        break;
    }

    switch (vertical_align_) {
    case VerticalAlign::Middle:
        dst.y = padded_rect.y + (padded_rect.h - dst.h) * 0.5f;
        break;
    case VerticalAlign::Bottom:
        dst.y = padded_rect.y + padded_rect.h - dst.h;
        break;
    case VerticalAlign::Top:
    default:
        dst.y = padded_rect.y;
        break;
    }

    SDL_RenderTexture(context.renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

} // namespace ui
} // namespace zg
