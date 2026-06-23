#include "IconItem.h"

#include "../Presentation.h"
#include "../Texture.h"

#include <algorithm>

namespace zg {
namespace ui {

IconItem& IconItem::set_texture(const Texture* texture)
{
    texture_ = texture;
    return *this;
}

IconItem& IconItem::set_preserve_aspect(bool preserve_aspect)
{
    preserve_aspect_ = preserve_aspect;
    return *this;
}

void IconItem::render(const RenderContext& context, const SDL_FRect& parent_rect) const
{
    if (context.renderer == nullptr || texture_ == nullptr || !texture_->valid()) {
        return;
    }

    const SDL_FRect logical_rect = resolve_rect(parent_rect);
    SDL_FRect screen_rect = ui_logical_to_present_rect(logical_rect, context.presentation_rect);
    if (preserve_aspect_) {
        const float scale = std::min(
            screen_rect.w / std::max(1.0f, texture_->width()),
            screen_rect.h / std::max(1.0f, texture_->height()));
        const float draw_w = texture_->width() * scale;
        const float draw_h = texture_->height() * scale;
        screen_rect.x += (screen_rect.w - draw_w) * 0.5f;
        screen_rect.y += (screen_rect.h - draw_h) * 0.5f;
        screen_rect.w = draw_w;
        screen_rect.h = draw_h;
    }
    SDL_SetTextureAlphaMod(texture_->get(), context.alpha);
    SDL_RenderTexture(context.renderer, texture_->get(), nullptr, &screen_rect);
    SDL_SetTextureAlphaMod(texture_->get(), 255);
}

} // namespace ui
} // namespace zg
