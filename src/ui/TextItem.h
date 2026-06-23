#pragma once

#include "Item.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <string>

namespace zg {
namespace ui {

enum class HorizontalAlign {
    Left = 0,
    Center,
    Right
};

enum class VerticalAlign {
    Top = 0,
    Middle,
    Bottom
};

class TextItem : public Item {
public:
    TextItem() = default;

    TextItem& set_text(const char* text);
    TextItem& set_font(TTF_Font* font);
    TextItem& set_color(SDL_Color color);
    TextItem& set_fit_to_bounds(bool fit);
    TextItem& set_linear_filter(bool linear);
    TextItem& set_horizontal_align(HorizontalAlign align);
    TextItem& set_vertical_align(VerticalAlign align);
    TextItem& set_padding(float x, float y);

    void render(const RenderContext& context, const SDL_FRect& parent_rect) const override;

private:
    std::string text_;
    TTF_Font* font_ = nullptr;
    SDL_Color color_{255, 255, 255, 255};
    bool fit_to_bounds_ = true;
    bool linear_filter_ = true;
    HorizontalAlign horizontal_align_ = HorizontalAlign::Left;
    VerticalAlign vertical_align_ = VerticalAlign::Top;
    float padding_x_ = 0.0f;
    float padding_y_ = 0.0f;
};

} // namespace ui
} // namespace zg
