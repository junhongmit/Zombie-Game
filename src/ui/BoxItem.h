#pragma once

#include "Item.h"

namespace zg {
namespace ui {

class BoxItem : public Item {
public:
    BoxItem() = default;

    BoxItem& set_fill_color(SDL_Color color);
    BoxItem& set_border_color(SDL_Color color);
    BoxItem& set_border_width(float width);
    BoxItem& set_fill_enabled(bool enabled);
    BoxItem& set_border_enabled(bool enabled);

    void render(const RenderContext& context, const SDL_FRect& parent_rect) const override;

private:
    SDL_Color fill_color_{0, 0, 0, 0};
    SDL_Color border_color_{0, 0, 0, 0};
    float border_width_ = 1.0f;
    bool fill_enabled_ = true;
    bool border_enabled_ = false;
};

} // namespace ui
} // namespace zg
