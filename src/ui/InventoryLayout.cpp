#include "InventoryLayout.h"

#include "../Constants.h"
#include "LayoutJson.h"

#include <cstdio>

namespace zg {

bool InventoryLayout::load(const char* asset_path)
{
    std::string text;
    if (!read_layout_text_file(asset_path, &text)) {
        return false;
    }

    std::string rects_object;
    if (extract_object_value(text, "rects", &rects_object)) {
        parse_named_rect(rects_object, "equipment_panel", &equipment_panel_);
        parse_named_rect(rects_object, "bag_panel", &bag_panel_);
        parse_named_rect(rects_object, "resources_panel", &resources_panel_);
        parse_named_rect(rects_object, "weight_panel", &weight_panel_);
        parse_named_rect(rects_object, "backpack_panel", &backpack_panel_);
        parse_named_rect(rects_object, "action_bar", &action_bar_);
        parse_named_rect(rects_object, "equipment_entry_start", &equipment_entry_start_);
        parse_named_rect(rects_object, "paperdoll_rect", &paperdoll_rect_);
        parse_named_rect(rects_object, "paperdoll_slot_start", &paperdoll_slot_start_);
        parse_named_rect(rects_object, "bag_slot_start", &bag_slot_start_);
        parse_named_rect(rects_object, "backpack_option_start", &backpack_option_start_);
        parse_named_rect(rects_object, "action_hint_start", &action_hint_start_);
    }

    std::string metrics_object;
    if (extract_object_value(text, "metrics", &metrics_object)) {
        extract_float_value(metrics_object, "equipment_entry_gap", &equipment_entry_gap_);
        extract_int_value(metrics_object, "equipment_entry_count", &equipment_entry_count_);
        extract_float_value(metrics_object, "paperdoll_slot_gap", &paperdoll_slot_gap_);
        extract_int_value(metrics_object, "paperdoll_slot_count", &paperdoll_slot_count_);
        extract_float_value(metrics_object, "bag_slot_gap_x", &bag_slot_gap_x_);
        extract_float_value(metrics_object, "bag_slot_gap_y", &bag_slot_gap_y_);
        extract_int_value(metrics_object, "bag_slot_columns", &bag_slot_columns_);
        extract_int_value(metrics_object, "bag_slot_rows", &bag_slot_rows_);
        extract_float_value(metrics_object, "backpack_option_gap", &backpack_option_gap_);
        extract_int_value(metrics_object, "backpack_option_count", &backpack_option_count_);
        extract_float_value(metrics_object, "action_hint_gap", &action_hint_gap_);
        extract_int_value(metrics_object, "action_hint_count", &action_hint_count_);
    }

    return true;
}

SDL_FRect InventoryLayout::equipment_panel_rect() const { return to_logical_rect(equipment_panel_); }
SDL_FRect InventoryLayout::bag_panel_rect() const { return to_logical_rect(bag_panel_); }
SDL_FRect InventoryLayout::resources_panel_rect() const { return to_logical_rect(resources_panel_); }
SDL_FRect InventoryLayout::weight_panel_rect() const { return to_logical_rect(weight_panel_); }
SDL_FRect InventoryLayout::backpack_panel_rect() const { return to_logical_rect(backpack_panel_); }
SDL_FRect InventoryLayout::action_bar_rect() const { return to_logical_rect(action_bar_); }
SDL_FRect InventoryLayout::paperdoll_rect() const { return to_logical_rect(paperdoll_rect_); }

SDL_FRect InventoryLayout::equipment_entry_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(equipment_entry_start_);
    rect.y += index * (rect.h + normalized_y(equipment_entry_gap_));
    return rect;
}

SDL_FRect InventoryLayout::paperdoll_slot_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(paperdoll_slot_start_);
    rect.y += index * (rect.h + normalized_y(paperdoll_slot_gap_));
    return rect;
}

SDL_FRect InventoryLayout::bag_slot_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(bag_slot_start_);
    const int column = index % bag_slot_columns_;
    const int row = index / bag_slot_columns_;
    rect.x += column * (rect.w + normalized_x(bag_slot_gap_x_));
    rect.y += row * (rect.h + normalized_y(bag_slot_gap_y_));
    return rect;
}

SDL_FRect InventoryLayout::backpack_option_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(backpack_option_start_);
    rect.x += index * (rect.w + normalized_x(backpack_option_gap_));
    return rect;
}

SDL_FRect InventoryLayout::action_hint_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(action_hint_start_);
    rect.x += index * (rect.w + normalized_x(action_hint_gap_));
    return rect;
}

SDL_FRect InventoryLayout::to_logical_rect(const NormalizedRect& rect) const
{
    return SDL_FRect{normalized_x(rect.x), normalized_y(rect.y), normalized_x(rect.w), normalized_y(rect.h)};
}

float InventoryLayout::normalized_x(float value) const { return value * static_cast<float>(kUiDesignWidth); }
float InventoryLayout::normalized_y(float value) const { return value * static_cast<float>(kUiDesignHeight); }

} // namespace zg
