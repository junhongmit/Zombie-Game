#pragma once

#include "Container.h"
#include "ControlStyle.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

namespace zg {

class Texture;

namespace ui {

class Card : public Container {
public:
    Card() = default;
    Card(float x, float y, float w, float h, bool enabled = true);
    Card(const Card&) = delete;
    Card& operator=(const Card&) = delete;
    Card(Card&&) = default;
    Card& operator=(Card&&) = default;

    Card& set_enabled(bool enabled);
    Card& set_selected(bool selected);
    Card& set_title(const char* title);
    Card& set_subtitle(const char* subtitle);
    Card& set_meta(const char* meta);
    Card& set_icon(const Texture* icon);
    Card& set_style(const ControlStyle* skin);
    Card& set_fonts(TTF_Font* title_font, TTF_Font* subtitle_font);
    Card& set_text_colors(SDL_Color title_color, SDL_Color subtitle_color, SDL_Color meta_color);
    Card& set_alpha(Uint8 alpha);

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
    void render(const RenderContext& context, const SDL_FRect& parent_rect) const override;
    SDL_FRect slot_rect(const std::string& slot_name, const SDL_FRect& resolved_rect) const override;

private:
    bool enabled_ = true;
    bool selected_ = false;
    std::string title_;
    std::string subtitle_;
    std::string meta_;
    const Texture* icon_ = nullptr;
    const ControlStyle* skin_ = nullptr;
    TTF_Font* title_font_ = nullptr;
    TTF_Font* subtitle_font_ = nullptr;
    SDL_Color title_color_{255, 255, 255, 255};
    SDL_Color subtitle_color_{220, 220, 220, 255};
    SDL_Color meta_color_{180, 180, 180, 255};
    Uint8 alpha_ = 255;
};

} // namespace ui
} // namespace zg
