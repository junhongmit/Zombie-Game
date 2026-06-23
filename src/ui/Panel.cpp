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
{
    set_rect(x, y, w, h);
}

Panel& Panel::set_title(const char* title)
{
    title_ = title != nullptr ? title : "";
    return *this;
}

Panel& Panel::set_enabled(bool enabled)
{
    enabled_ = enabled;
    Item::set_enabled(enabled);
    return *this;
}

Panel& Panel::set_header_height(float height)
{
    header_height_ = std::max(0.0f, height);
    return *this;
}

Panel& Panel::set_footer_height(float height)
{
    footer_height_ = std::max(0.0f, height);
    return *this;
}

Panel& Panel::set_style(const ControlStyle* skin)
{
    skin_ = skin;
    return *this;
}

Panel& Panel::set_font(TTF_Font* font)
{
    font_ = font;
    return *this;
}

Panel& Panel::set_text_color(SDL_Color color)
{
    text_color_ = color;
    return *this;
}

Panel& Panel::set_alpha(Uint8 alpha)
{
    alpha_ = alpha;
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
    const SDL_FRect screen_rect = ui_logical_to_present_rect(logical_rect(), presentation_rect);
    skin.render(renderer, screen_rect, state, alpha);
    if (!title_.empty()) {
        text_color.a = alpha;
        render_panel_title(renderer, skin, font, title_.c_str(), screen_rect, state, text_color);
    }

    RenderContext context;
    context.renderer = renderer;
    context.presentation_rect = presentation_rect;
    context.alpha = alpha;
    const SDL_FRect logical_content = ui_present_to_logical_rect(skin.content_rect(screen_rect), presentation_rect);
    render_children(context, logical_content);
}

void Panel::render(const RenderContext& context, const SDL_FRect& parent_rect) const
{
    if (context.renderer == nullptr) {
        return;
    }
    const SDL_FRect logical_rect = resolve_rect(parent_rect);
    if (skin_ == nullptr) {
        render_children(context, logical_rect);
        return;
    }

    const ControlVisualState state = enabled_ ? ControlVisualState::Normal : ControlVisualState::Disabled;
    const Uint8 resolved_alpha = static_cast<Uint8>(std::round((alpha_ / 255.0f) * context.alpha));
    const SDL_FRect screen_rect = ui_logical_to_present_rect(logical_rect, context.presentation_rect);
    skin_->render(context.renderer, screen_rect, state, resolved_alpha);
    if (!title_.empty()) {
        SDL_Color text_color = text_color_;
        text_color.a = resolved_alpha;
        render_panel_title(context.renderer, *skin_, font_, title_.c_str(), screen_rect, state, text_color);
    }

    RenderContext child_context = context;
    child_context.alpha = resolved_alpha;
    const SDL_FRect logical_content = ui_present_to_logical_rect(skin_->content_rect(screen_rect), context.presentation_rect);
    render_children(child_context, logical_content);
}

SDL_FRect Panel::content_rect(const ControlStyle& skin, const SDL_FRect& presentation_rect) const
{
    return skin.content_rect(ui_logical_to_present_rect(logical_rect(), presentation_rect));
}

SDL_FRect Panel::slot_rect(const std::string& slot_name, const SDL_FRect& resolved_rect) const
{
    const float header_height = std::min(header_height_, resolved_rect.h);
    const float footer_height = std::min(footer_height_, std::max(0.0f, resolved_rect.h - header_height));
    const float body_height = std::max(0.0f, resolved_rect.h - header_height - footer_height);

    if (slot_name == "header" || slot_name == "title" || slot_name == "header_action") {
        return SDL_FRect{resolved_rect.x, resolved_rect.y, resolved_rect.w, header_height};
    }
    if (slot_name == "footer") {
        return SDL_FRect{resolved_rect.x, resolved_rect.y + resolved_rect.h - footer_height, resolved_rect.w, footer_height};
    }
    if (slot_name == "body" || slot_name.empty()) {
        return SDL_FRect{resolved_rect.x, resolved_rect.y + header_height, resolved_rect.w, body_height};
    }
    return resolved_rect;
}

} // namespace ui
} // namespace zg
