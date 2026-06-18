#pragma once

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

class Button {
public:
    Button() = default;
    Button(const char* label, float x, float y, float w, float h, bool enabled = true);

    Button& set_label(const char* label);
    Button& set_enabled(bool enabled);
    Button& set_icon(
        const Texture* icon,
        float width,
        float height,
        ButtonIconAlignment alignment = ButtonIconAlignment::Left,
        float gap = 6.0f);
    Button& clear_icon();

    bool contains(float x, float y, bool mouse_in_view) const;
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

    const SDL_FRect& logical_rect() const { return logical_rect_; }
    const std::string& label() const { return label_; }
    bool enabled() const { return enabled_; }
    bool has_icon() const { return icon_ != nullptr && icon_width_ > 0.0f && icon_height_ > 0.0f; }

private:
    std::string label_;
    SDL_FRect logical_rect_{};
    bool enabled_ = true;
    const Texture* icon_ = nullptr;
    float icon_width_ = 0.0f;
    float icon_height_ = 0.0f;
    float icon_gap_ = 6.0f;
    ButtonIconAlignment icon_alignment_ = ButtonIconAlignment::Left;
};

} // namespace ui
} // namespace zg
