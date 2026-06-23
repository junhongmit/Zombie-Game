#pragma once

#include "Item.h"

namespace zg {

class Texture;

namespace ui {

class IconItem : public Item {
public:
    IconItem() = default;

    IconItem& set_texture(const Texture* texture);
    IconItem& set_preserve_aspect(bool preserve_aspect);

    void render(const RenderContext& context, const SDL_FRect& parent_rect) const override;

private:
    const Texture* texture_ = nullptr;
    bool preserve_aspect_ = true;
};

} // namespace ui
} // namespace zg
