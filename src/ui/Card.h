#pragma once

#include "ControlStyle.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

namespace zg {

class Texture;

namespace ui {

class Card {
public:
    Card() = default;
    Card(float x, float y, float w, float h, bool enabled = true);

    Card& set_enabled(bool enabled);
    Card& set_selected(bool selected);
    Card& set_title(const char* title);
    Card& set_subtitle(const char* subtitle);
    Card& set_meta(const char* meta);
    Card& set_icon(const Texture* icon);

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
        Uint8 alpha = 255) const;

private:
    SDL_FRect logical_rect_{};
    bool enabled_ = true;
    bool selected_ = false;
    std::string title_;
    std::string subtitle_;
    std::string meta_;
    const Texture* icon_ = nullptr;
};

} // namespace ui
} // namespace zg
