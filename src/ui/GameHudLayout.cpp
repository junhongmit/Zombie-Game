#include "GameHudLayout.h"

#include "../Constants.h"
#include "LayoutJson.h"

#include <cstdio>

namespace zg {

bool GameHudLayout::load(const char* asset_path)
{
    std::string text;
    if (!read_layout_text_file(asset_path, &text)) {
        return false;
    }

    std::string rects_object;
    if (extract_object_value(text, "rects", &rects_object)) {
        parse_named_rect(rects_object, "portrait_panel", &portrait_panel_);
        parse_named_rect(rects_object, "time_panel", &time_panel_);
        parse_named_rect(rects_object, "objectives_panel", &objectives_panel_);
        parse_named_rect(rects_object, "warning_panel", &warning_panel_);
        for (int i = 0; i < 4; ++i) {
            char key[32];
            std::snprintf(key, sizeof(key), "top_panel_%d", i);
            parse_named_rect(rects_object, key, &top_panels_[i]);
        }
        parse_named_rect(rects_object, "mode_button_start", &mode_button_start_);
        parse_named_rect(rects_object, "tool_slot_start", &tool_slot_start_);
        parse_named_rect(rects_object, "survivor_start", &survivor_start_);
    }

    std::string metrics_object;
    if (extract_object_value(text, "metrics", &metrics_object)) {
        extract_float_value(metrics_object, "mode_button_gap", &mode_button_gap_);
        extract_int_value(metrics_object, "mode_button_count", &mode_button_count_);
        extract_float_value(metrics_object, "tool_slot_gap", &tool_slot_gap_);
        extract_int_value(metrics_object, "tool_slot_count", &tool_slot_count_);
        extract_float_value(metrics_object, "survivor_gap", &survivor_gap_);
        extract_int_value(metrics_object, "survivor_count", &survivor_count_);
    }

    return true;
}

SDL_FRect GameHudLayout::portrait_panel_rect() const { return to_logical_rect(portrait_panel_); }
SDL_FRect GameHudLayout::top_panel_rect(int index) const { return to_logical_rect(top_panels_[index < 0 ? 0 : index % 4]); }
SDL_FRect GameHudLayout::time_panel_rect() const { return to_logical_rect(time_panel_); }
SDL_FRect GameHudLayout::objectives_panel_rect() const { return to_logical_rect(objectives_panel_); }
SDL_FRect GameHudLayout::warning_panel_rect() const { return to_logical_rect(warning_panel_); }

SDL_FRect GameHudLayout::mode_button_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(mode_button_start_);
    rect.x += index * (rect.w + normalized_x(mode_button_gap_));
    return rect;
}

SDL_FRect GameHudLayout::tool_slot_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(tool_slot_start_);
    rect.x += index * (rect.w + normalized_x(tool_slot_gap_));
    return rect;
}

SDL_FRect GameHudLayout::survivor_panel_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(survivor_start_);
    rect.x += index * (rect.w + normalized_x(survivor_gap_));
    return rect;
}

SDL_FRect GameHudLayout::to_logical_rect(const NormalizedRect& rect) const
{
    return SDL_FRect{normalized_x(rect.x), normalized_y(rect.y), normalized_x(rect.w), normalized_y(rect.h)};
}

float GameHudLayout::normalized_x(float value) const { return value * static_cast<float>(kUiDesignWidth); }
float GameHudLayout::normalized_y(float value) const { return value * static_cast<float>(kUiDesignHeight); }

} // namespace zg
