#pragma once

#include <string>

namespace zg {

struct NormalizedRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    NormalizedRect() {}
    NormalizedRect(float x_value, float y_value, float w_value, float h_value)
        : x(x_value), y(y_value), w(w_value), h(h_value)
    {
    }
};

struct LayoutTextNode {
    NormalizedRect rect;
    std::string text;
};

struct LayoutPanelNode {
    NormalizedRect rect;
    std::string skin;
};

struct LayoutContainerNode {
    NormalizedRect rect;
    std::string skin;
    LayoutTextNode title;
    NormalizedRect list_view;
};

} // namespace zg
