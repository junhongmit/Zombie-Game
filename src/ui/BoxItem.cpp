#include "BoxItem.h"

#include "../Presentation.h"

#include <algorithm>
#include <cmath>

namespace zg {
namespace ui {

BoxItem& BoxItem::set_fill_color(SDL_Color color)
{
    fill_color_ = color;
    return *this;
}

BoxItem& BoxItem::set_border_color(SDL_Color color)
{
    border_color_ = color;
    return *this;
}

BoxItem& BoxItem::set_border_width(float width)
{
    border_width_ = std::max(0.0f, width);
    return *this;
}

BoxItem& BoxItem::set_fill_enabled(bool enabled)
{
    fill_enabled_ = enabled;
    return *this;
}

BoxItem& BoxItem::set_border_enabled(bool enabled)
{
    border_enabled_ = enabled;
    return *this;
}

void BoxItem::render(const RenderContext& context, const SDL_FRect& parent_rect) const
{
    if (context.renderer == nullptr) {
        return;
    }

    const SDL_FRect logical_rect = resolve_rect(parent_rect);
    const SDL_FRect screen_rect = ui_logical_to_present_rect(logical_rect, context.presentation_rect);
    SDL_SetRenderDrawBlendMode(context.renderer, SDL_BLENDMODE_BLEND);
    if (fill_enabled_) {
        SDL_Color fill = fill_color_;
        fill.a = static_cast<Uint8>(std::round((fill.a / 255.0f) * context.alpha));
        SDL_SetRenderDrawColor(context.renderer, fill.r, fill.g, fill.b, fill.a);
        SDL_RenderFillRect(context.renderer, &screen_rect);
    }
    if (border_enabled_ && border_width_ > 0.0f) {
        SDL_Color border = border_color_;
        border.a = static_cast<Uint8>(std::round((border.a / 255.0f) * context.alpha));
        SDL_SetRenderDrawColor(context.renderer, border.r, border.g, border.b, border.a);
        const int iterations = std::max(1, static_cast<int>(std::round(border_width_)));
        SDL_FRect border_rect = screen_rect;
        for (int i = 0; i < iterations; ++i) {
            SDL_RenderRect(context.renderer, &border_rect);
            border_rect.x += 1.0f;
            border_rect.y += 1.0f;
            border_rect.w = std::max(0.0f, border_rect.w - 2.0f);
            border_rect.h = std::max(0.0f, border_rect.h - 2.0f);
        }
    }
    SDL_SetRenderDrawBlendMode(context.renderer, SDL_BLENDMODE_NONE);
}

} // namespace ui
} // namespace zg
