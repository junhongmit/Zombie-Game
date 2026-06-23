#include "MarketLayout.h"

#include "../Constants.h"
#include "LayoutJson.h"

namespace zg {

bool MarketLayout::load(const char* asset_path)
{
    std::string text;
    if (!read_layout_text_file(asset_path, &text)) {
        return false;
    }

    std::string rects_object;
    if (extract_object_value(text, "rects", &rects_object)) {
        parse_panel_node(rects_object, "resource_strip", &resource_strip_);
        parse_panel_node(rects_object, "merchant_panel", &merchant_panel_);
        parse_panel_node(rects_object, "cart_panel", &cart_panel_);
        parse_panel_node(rects_object, "timer_panel", &timer_panel_);
        parse_panel_node(rects_object, "note_panel", &note_panel_);
        parse_named_rect(rects_object, "notebook", &notebook_);
        parse_named_rect(rects_object, "close_button", &close_button_);
        parse_named_rect(rects_object, "category_button_start", &category_button_start_);
        parse_named_rect(rects_object, "page_button_start", &page_button_start_);
        parse_named_rect(rects_object, "catalog_item_start", &catalog_item_start_);
    }

    std::string metrics_object;
    if (extract_object_value(text, "metrics", &metrics_object)) {
        extract_float_value(metrics_object, "category_button_gap", &category_button_gap_);
        extract_int_value(metrics_object, "category_button_count", &category_button_count_);
        extract_float_value(metrics_object, "page_button_gap", &page_button_gap_);
        extract_float_value(metrics_object, "catalog_item_gap_x", &catalog_item_gap_x_);
        extract_float_value(metrics_object, "catalog_item_gap_y", &catalog_item_gap_y_);
        extract_int_value(metrics_object, "catalog_item_count", &catalog_item_count_);
    }

    return true;
}

SDL_FRect MarketLayout::resource_strip_rect() const { return to_logical_rect(resource_strip_.rect); }
SDL_FRect MarketLayout::merchant_panel_rect() const { return to_logical_rect(merchant_panel_.rect); }
SDL_FRect MarketLayout::cart_panel_rect() const { return to_logical_rect(cart_panel_.rect); }
SDL_FRect MarketLayout::timer_panel_rect() const { return to_logical_rect(timer_panel_.rect); }
SDL_FRect MarketLayout::note_panel_rect() const { return to_logical_rect(note_panel_.rect); }
SDL_FRect MarketLayout::notebook_rect() const { return to_logical_rect(notebook_); }
SDL_FRect MarketLayout::close_button_rect() const { return to_logical_rect(close_button_); }

SDL_FRect MarketLayout::category_button_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(category_button_start_);
    rect.y += index * (rect.h + normalized_y(category_button_gap_));
    return rect;
}

SDL_FRect MarketLayout::page_button_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(page_button_start_);
    rect.x += index * normalized_x(page_button_gap_);
    return rect;
}

SDL_FRect MarketLayout::catalog_item_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(catalog_item_start_);
    const int column = index % 2;
    const int row = index / 2;
    rect.x += normalized_x(catalog_item_gap_x_) * column;
    rect.y += normalized_y(catalog_item_gap_y_) * row;
    return rect;
}

SDL_FRect MarketLayout::to_logical_rect(const NormalizedRect& rect) const
{
    return SDL_FRect{
        normalized_x(rect.x),
        normalized_y(rect.y),
        normalized_x(rect.w),
        normalized_y(rect.h)
    };
}

float MarketLayout::normalized_x(float value) const
{
    return value * static_cast<float>(kUiDesignWidth);
}

float MarketLayout::normalized_y(float value) const
{
    return value * static_cast<float>(kUiDesignHeight);
}

} // namespace zg
