#include "Button.h"

#include "../Presentation.h"
#include "../Texture.h"

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

void render_text_sprite(SDL_Renderer* renderer, const TextSprite& sprite, const SDL_FRect& dst)
{
    if (sprite.texture == nullptr || dst.w <= 0.0f || dst.h <= 0.0f) {
        return;
    }

    SDL_RenderTexture(renderer, sprite.texture, nullptr, &dst);
}

float icon_left_x(const SDL_FRect& content_rect, float icon_width, float scale)
{
    return content_rect.x + std::floor(4.0f * scale);
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

SDL_FRect square_left_rect(const SDL_FRect& rect, float size, float margin)
{
    return SDL_FRect{
        rect.x + margin,
        rect.y + std::floor((rect.h - size) * 0.5f),
        size,
        size
    };
}

} // namespace

Button::Button(const char* label, float x, float y, float w, float h, bool enabled)
    : label_(label != nullptr ? label : ""),
      enabled_(enabled)
{
    set_rect(x, y, w, h);
    Item::set_enabled(enabled);
}

Button& Button::set_label(const char* label)
{
    label_ = label != nullptr ? label : "";
    return *this;
}

Button& Button::set_enabled(bool enabled)
{
    enabled_ = enabled;
    Item::set_enabled(enabled);
    return *this;
}

Button& Button::set_icon(
    const Texture* icon,
    float width,
    float height,
    ButtonIconAlignment alignment,
    float gap)
{
    icon_ = icon;
    icon_width_ = width;
    icon_height_ = height;
    icon_alignment_ = alignment;
    icon_gap_ = gap;
    return *this;
}

Button& Button::clear_icon()
{
    icon_ = nullptr;
    icon_width_ = 0.0f;
    icon_height_ = 0.0f;
    icon_gap_ = 6.0f;
    icon_alignment_ = ButtonIconAlignment::Left;
    return *this;
}

Button& Button::set_style(const ControlStyle* skin)
{
    skin_ = skin;
    return *this;
}

Button& Button::set_font(TTF_Font* font)
{
    font_ = font;
    return *this;
}

Button& Button::set_text_colors(SDL_Color enabled_text_color, SDL_Color disabled_text_color)
{
    enabled_text_color_ = enabled_text_color;
    disabled_text_color_ = disabled_text_color;
    return *this;
}

Button& Button::set_alpha(Uint8 alpha)
{
    alpha_ = alpha;
    return *this;
}

Button& Button::set_visual_override(bool enabled, ControlVisualState state)
{
    visual_override_enabled_ = enabled;
    visual_override_state_ = state;
    return *this;
}

bool Button::contains(float x, float y, bool mouse_in_view) const
{
    return enabled_ && mouse_in_view && point_in_rect(x, y, logical_rect());
}

bool Button::process_pointer(
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_pressed,
    bool mouse_released,
    bool& armed) const
{
    if (!enabled_) {
        armed = false;
        return false;
    }

    const bool hovered = contains(mouse_x, mouse_y, mouse_in_view);
    if (mouse_pressed) {
        armed = hovered;
    }

    bool activated = false;
    if (mouse_released) {
        activated = armed && hovered;
        armed = false;
    }

    return activated;
}

ControlVisualState Button::visual_state(
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool armed) const
{
    if (!enabled_) {
        return ControlVisualState::Disabled;
    }
    if (visual_override_enabled_) {
        return visual_override_state_;
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

void Button::render(
    SDL_Renderer* renderer,
    const ControlStyle& skin,
    TTF_Font* font,
    const SDL_FRect& presentation_rect,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool armed,
    SDL_Color enabled_text_color,
    SDL_Color disabled_text_color,
    Uint8 alpha) const
{
    const ControlVisualState state = visual_state(mouse_x, mouse_y, mouse_in_view, mouse_down, armed);
    const SDL_FRect screen_rect = ui_logical_to_present_rect(logical_rect(), presentation_rect);
    skin.render(renderer, screen_rect, state, alpha);

    SDL_Color text_color = enabled_ ? enabled_text_color : disabled_text_color;
    text_color.a = alpha;
    const TextSprite text_sprite = make_text_sprite(renderer, font, label_.c_str(), text_color);
    const SDL_FRect content_rect = skin.content_rect(screen_rect);
    const float scale = ui_presentation_scale(presentation_rect) * skin.style_scale();
    const SDL_FRect title_rect = skin.title_region() != nullptr
        ? skin.map_region(*skin.title_region(), screen_rect, state)
        : content_rect;
    const SDL_FRect icon_region_rect = skin.icon_region() != nullptr
        ? skin.map_region(*skin.icon_region(), screen_rect, state)
        : inset_rect(content_rect, std::round(4.0f * scale));

    const bool draw_icon = has_icon() && icon_->valid();
    const bool draw_text = text_sprite.texture != nullptr;
    const float icon_width = draw_icon ? std::round(icon_width_ * scale) : 0.0f;
    const float icon_height = draw_icon ? std::round(icon_height_ * scale) : 0.0f;
    const float icon_gap = (draw_icon && draw_text) ? std::round(icon_gap_ * scale) : 0.0f;

    SDL_FRect icon_dst{};
    SDL_FRect text_dst{};

    if (draw_icon && draw_text && icon_alignment_ == ButtonIconAlignment::Center) {
        const float group_width = icon_width + icon_gap + text_sprite.width;
        const float start_x = title_rect.x + std::floor((title_rect.w - group_width) * 0.5f);
        const float center_y = title_rect.y + std::floor((title_rect.h - std::max(icon_height, text_sprite.height)) * 0.5f);
        icon_dst = SDL_FRect{
            start_x,
            center_y + std::floor((std::max(icon_height, text_sprite.height) - icon_height) * 0.5f),
            icon_width,
            icon_height
        };
        text_dst = SDL_FRect{
            start_x + icon_width + icon_gap,
            center_y + std::floor((std::max(icon_height, text_sprite.height) - text_sprite.height) * 0.5f),
            text_sprite.width,
            text_sprite.height
        };
    } else {
        float text_center_left = title_rect.x;
        float text_center_width = title_rect.w;

        if (draw_icon) {
            const float resolved_icon_x = (icon_alignment_ == ButtonIconAlignment::Center && !draw_text)
                ? icon_region_rect.x + std::floor((icon_region_rect.w - icon_width) * 0.5f)
                : icon_left_x(icon_region_rect, icon_width, scale);
            icon_dst = SDL_FRect{
                resolved_icon_x,
                icon_region_rect.y + std::floor((icon_region_rect.h - icon_height) * 0.5f),
                icon_width,
                icon_height
            };

            if (draw_text) {
                const float reserved_width = std::max(0.0f, (icon_dst.x + icon_width + icon_gap) - title_rect.x);
                text_center_left = title_rect.x + reserved_width;
                text_center_width = std::max(0.0f, title_rect.w - reserved_width);
            }
        }

        if (draw_text) {
            text_dst = SDL_FRect{
                text_center_left + std::floor((text_center_width - text_sprite.width) * 0.5f),
                title_rect.y + std::floor((title_rect.h - text_sprite.height) * 0.5f),
                text_sprite.width,
                text_sprite.height
            };
        }
    }

    if (draw_icon) {
        SDL_SetTextureAlphaMod(icon_->get(), alpha);
        SDL_RenderTexture(renderer, icon_->get(), nullptr, &icon_dst);
        SDL_SetTextureAlphaMod(icon_->get(), 255);
    }

    if (draw_text) {
        render_text_sprite(renderer, text_sprite, text_dst);
        SDL_DestroyTexture(text_sprite.texture);
    }

    RenderContext context;
    context.renderer = renderer;
    context.presentation_rect = presentation_rect;
    context.mouse_x = mouse_x;
    context.mouse_y = mouse_y;
    context.mouse_in_view = mouse_in_view;
    context.mouse_down = mouse_down;
    context.alpha = alpha;
    const SDL_FRect logical_content = ui_present_to_logical_rect(content_rect, presentation_rect);
    render_children(context, logical_content);
}

void Button::render(const RenderContext& context, const SDL_FRect& parent_rect) const
{
    if (context.renderer == nullptr) {
        return;
    }
    const SDL_FRect logical_rect = resolve_rect(parent_rect);
    if (skin_ == nullptr || font_ == nullptr) {
        render_children(context, logical_rect);
        return;
    }

    render(
        context.renderer,
        *skin_,
        font_,
        context.presentation_rect,
        context.mouse_x,
        context.mouse_y,
        context.mouse_in_view,
        context.mouse_down,
        false,
        enabled_text_color_,
        disabled_text_color_,
        static_cast<Uint8>(std::round((alpha_ / 255.0f) * context.alpha)));
}

SDL_FRect Button::slot_rect(const std::string& slot_name, const SDL_FRect& resolved_rect) const
{
    const float margin = std::max(2.0f, std::round(std::min(resolved_rect.w, resolved_rect.h) * 0.08f));
    const float icon_size = std::max(0.0f, std::min(resolved_rect.h - margin * 2.0f, resolved_rect.w * 0.28f));
    const float gap = std::max(4.0f, std::round(icon_size * 0.15f));
    const SDL_FRect icon_rect = square_left_rect(resolved_rect, icon_size, margin);
    const SDL_FRect after_icon_rect{
        icon_rect.x + icon_rect.w + gap,
        resolved_rect.y + margin,
        std::max(0.0f, resolved_rect.x + resolved_rect.w - (icon_rect.x + icon_rect.w + gap) - margin),
        std::max(0.0f, resolved_rect.h - margin * 2.0f)
    };

    if (slot_name == "icon") {
        return icon_rect;
    }
    if (slot_name == "icon_center") {
        return SDL_FRect{
            resolved_rect.x + std::floor((resolved_rect.w - icon_size) * 0.5f),
            resolved_rect.y + std::floor((resolved_rect.h - icon_size) * 0.5f),
            icon_size,
            icon_size
        };
    }
    if (slot_name == "label_after_icon" || slot_name == "title_after_icon") {
        return after_icon_rect;
    }
    if (slot_name == "title" || slot_name == "label" || slot_name == "text" || slot_name == "content" || slot_name.empty()) {
        return inset_rect(resolved_rect, margin);
    }
    if (slot_name == "badge") {
        const float badge_size = std::max(10.0f, std::min(resolved_rect.h * 0.42f, resolved_rect.w * 0.18f));
        return SDL_FRect{
            resolved_rect.x + resolved_rect.w - badge_size - margin,
            resolved_rect.y + margin,
            badge_size,
            badge_size
        };
    }
    return resolved_rect;
}

} // namespace ui
} // namespace zg
