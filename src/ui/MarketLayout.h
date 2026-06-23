#pragma once

#include "LayoutTypes.h"

#include <SDL3/SDL.h>

namespace zg {

class MarketLayout {
public:
    bool load(const char* asset_path);

    SDL_FRect resource_strip_rect() const;
    const std::string& resource_strip_skin() const { return resource_strip_.skin; }
    SDL_FRect merchant_panel_rect() const;
    const std::string& merchant_panel_skin() const { return merchant_panel_.skin; }
    SDL_FRect cart_panel_rect() const;
    const std::string& cart_panel_skin() const { return cart_panel_.skin; }
    SDL_FRect timer_panel_rect() const;
    const std::string& timer_panel_skin() const { return timer_panel_.skin; }
    SDL_FRect note_panel_rect() const;
    const std::string& note_panel_skin() const { return note_panel_.skin; }
    SDL_FRect notebook_rect() const;
    SDL_FRect close_button_rect() const;
    SDL_FRect category_button_rect(int index) const;
    SDL_FRect page_button_rect(int index) const;
    SDL_FRect catalog_item_rect(int index) const;

    int category_button_count() const { return category_button_count_; }
    int catalog_item_count() const { return catalog_item_count_; }

private:
    SDL_FRect to_logical_rect(const NormalizedRect& rect) const;
    float normalized_x(float value) const;
    float normalized_y(float value) const;

    LayoutPanelNode resource_strip_{NormalizedRect{0.20f, 0.02f, 0.60f, 0.06f}, "panel_square_bronze"};
    LayoutPanelNode merchant_panel_{NormalizedRect{0.02f, 0.08f, 0.20f, 0.28f}, "panel_square_bronze"};
    LayoutPanelNode cart_panel_{NormalizedRect{0.02f, 0.73f, 0.22f, 0.22f}, "panel_square_bronze"};
    LayoutPanelNode timer_panel_{NormalizedRect{0.79f, 0.79f, 0.18f, 0.15f}, "panel_square_bronze"};
    LayoutPanelNode note_panel_{NormalizedRect{0.53f, 0.79f, 0.22f, 0.15f}, "panel_square_bronze"};
    NormalizedRect notebook_{0.32f, 0.10f, 0.58f, 0.72f};
    NormalizedRect close_button_{0.88f, 0.02f, 0.09f, 0.05f};
    NormalizedRect category_button_start_{0.91f, 0.20f, 0.06f, 0.08f};
    float category_button_gap_ = 0.012f;
    int category_button_count_ = 6;
    NormalizedRect page_button_start_{0.55f, 0.72f, 0.07f, 0.05f};
    float page_button_gap_ = 0.10f;
    NormalizedRect catalog_item_start_{0.37f, 0.20f, 0.235f, 0.16f};
    float catalog_item_gap_x_ = 0.28f;
    float catalog_item_gap_y_ = 0.19f;
    int catalog_item_count_ = 6;
};

} // namespace zg
