#pragma once

#include "Container.h"
#include "ControlStyle.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

namespace zg {

class Texture;

namespace ui {

enum class ButtonIconAlignment {
    Left,
    Center
};

class Button : public Container {
public:
    Button() = default;
    Button(const char* label, float x, float y, float w, float h, bool enabled = true);
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    Button(Button&&) = default;
    Button& operator=(Button&&) = default;

    Button& set_label(const char* label);
    Button& set_enabled(bool enabled);
    Button& set_icon(
        const Texture* icon,
        float width,
        float height,
        ButtonIconAlignment alignment = ButtonIconAlignment::Left,
        float gap = 6.0f);
    Button& clear_icon();
    Button& set_style(const ControlStyle* skin);
    Button& set_font(TTF_Font* font);
    Button& set_text_colors(SDL_Color enabled_text_color, SDL_Color disabled_text_color);
    Button& set_alpha(Uint8 alpha);
    Button& set_visual_override(bool enabled, ControlVisualState state);

    bool contains(float x, float y, bool mouse_in_view) const;
    bool process_pointer(
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_pressed,
        bool mouse_released,
        bool& armed) const;
    ControlVisualState visual_state(
        float mouse_x,
        float mouse_y,
        bool mouse_in_view,
        bool mouse_down,
        bool armed) const;

    void render(
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
        Uint8 alpha = 255) const;
    void render(const RenderContext& context, const SDL_FRect& parent_rect) const override;
    SDL_FRect slot_rect(const std::string& slot_name, const SDL_FRect& resolved_rect) const override;

    const std::string& label() const { return label_; }
    bool enabled() const { return enabled_; }
    bool has_icon() const { return icon_ != nullptr && icon_width_ > 0.0f && icon_height_ > 0.0f; }

private:
    std::string label_;
    bool enabled_ = true;
    const Texture* icon_ = nullptr;
    float icon_width_ = 0.0f;
    float icon_height_ = 0.0f;
    float icon_gap_ = 6.0f;
    ButtonIconAlignment icon_alignment_ = ButtonIconAlignment::Left;
    const ControlStyle* skin_ = nullptr;
    TTF_Font* font_ = nullptr;
    SDL_Color enabled_text_color_{255, 255, 255, 255};
    SDL_Color disabled_text_color_{148, 148, 148, 255};
    Uint8 alpha_ = 255;
    bool visual_override_enabled_ = false;
    ControlVisualState visual_override_state_ = ControlVisualState::Normal;
};

} // namespace ui
} // namespace zg
