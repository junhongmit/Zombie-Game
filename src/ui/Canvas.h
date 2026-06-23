#pragma once

#include "Container.h"

namespace zg {
namespace ui {

class Canvas : public Container {
public:
    Canvas() = default;
    Canvas(float x, float y, float w, float h)
    {
        set_rect(x, y, w, h);
    }

    void render(const RenderContext& context, const SDL_FRect& parent_rect) const override
    {
        if (context.renderer == nullptr) {
            return;
        }
        render_children(context, resolve_rect(parent_rect));
    }
};

} // namespace ui
} // namespace zg
