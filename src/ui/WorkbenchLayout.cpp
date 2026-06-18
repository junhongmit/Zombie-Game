#include "WorkbenchLayout.h"

#include "../Constants.h"
#include "LayoutJson.h"

namespace zg {

bool WorkbenchLayout::load(const char* asset_path)
{
    std::string text;
    if (!read_layout_text_file(asset_path, &text)) {
        return false;
    }

    std::string rects_object;
    if (extract_object_value(text, "rects", &rects_object)) {
        parse_named_rect(rects_object, "resource_strip", &resource_strip_);
        parse_container_node(rects_object, "weapon_list_panel", &weapon_list_);
        parse_named_rect(rects_object, "preview_panel", &preview_panel_);
        parse_named_rect(rects_object, "preview_nameplate", &preview_nameplate_);
        parse_named_rect(rects_object, "stats_panel", &stats_panel_);
        parse_named_rect(rects_object, "effects_panel", &effects_panel_);
        parse_named_rect(rects_object, "upgrade_panel", &upgrade_panel_);
        parse_container_node(rects_object, "attachments_panel", &attachments_);
        parse_named_rect(rects_object, "parts_panel", &parts_panel_);
    }

    std::string metrics_object;
    if (extract_object_value(text, "metrics", &metrics_object)) {
        extract_float_value(metrics_object, "weapon_list_row_height", &weapon_list_row_height_);
        parse_named_rect(metrics_object, "attachment_slot", &attachment_slot_);
        parse_named_rect(metrics_object, "attachment_label", &attachment_label_);
        extract_float_value(metrics_object, "weapon_list_row_pitch", &weapon_list_row_pitch_);
        extract_float_value(metrics_object, "attachment_slot_gap", &attachment_slot_gap_);
        extract_int_value(metrics_object, "attachment_slot_count", &attachment_slot_count_);
    }

    return true;
}

SDL_FRect WorkbenchLayout::resource_strip_rect() const
{
    return to_logical_rect(resource_strip_);
}

SDL_FRect WorkbenchLayout::weapon_list_panel_rect() const
{
    return to_logical_rect(weapon_list_.rect);
}

SDL_FRect WorkbenchLayout::weapon_list_title_rect() const
{
    return child_rect(weapon_list_, weapon_list_.title.rect);
}

SDL_FRect WorkbenchLayout::weapon_list_view_rect() const
{
    return child_rect(weapon_list_, weapon_list_.list_view);
}

SDL_FRect WorkbenchLayout::preview_panel_rect() const
{
    return to_logical_rect(preview_panel_);
}

SDL_FRect WorkbenchLayout::preview_nameplate_rect() const
{
    return to_logical_rect(preview_nameplate_);
}

SDL_FRect WorkbenchLayout::stats_panel_rect() const
{
    return to_logical_rect(stats_panel_);
}

SDL_FRect WorkbenchLayout::effects_panel_rect() const
{
    return to_logical_rect(effects_panel_);
}

SDL_FRect WorkbenchLayout::upgrade_panel_rect() const
{
    return to_logical_rect(upgrade_panel_);
}

SDL_FRect WorkbenchLayout::attachments_panel_rect() const
{
    return to_logical_rect(attachments_.rect);
}

SDL_FRect WorkbenchLayout::attachments_title_rect() const
{
    return child_rect(attachments_, attachments_.title.rect);
}

SDL_FRect WorkbenchLayout::attachments_view_rect() const
{
    return child_rect(attachments_, attachments_.list_view);
}

SDL_FRect WorkbenchLayout::parts_panel_rect() const
{
    return to_logical_rect(parts_panel_);
}

SDL_FRect WorkbenchLayout::weapon_row_rect(const SDL_FRect& viewport_rect, int index, float scroll_offset) const
{
    return SDL_FRect{
        viewport_rect.x,
        viewport_rect.y + weapon_row_pitch() * index - scroll_offset,
        viewport_rect.w,
        weapon_row_height()
    };
}

float WorkbenchLayout::weapon_row_height() const
{
    return normalized_y(weapon_list_row_height_);
}

float WorkbenchLayout::weapon_row_pitch() const
{
    return normalized_y(weapon_list_row_pitch_);
}

float WorkbenchLayout::weapon_list_content_height(int slot_count) const
{
    if (slot_count <= 0) {
        return 0.0f;
    }
    return (slot_count - 1) * weapon_row_pitch() + weapon_row_height();
}

SDL_FRect WorkbenchLayout::attachment_slot_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(attachment_slot_);
    rect.x += index * (rect.w + normalized_x(attachment_slot_gap_));
    return rect;
}

SDL_FRect WorkbenchLayout::attachment_label_rect(int index) const
{
    SDL_FRect rect = to_logical_rect(attachment_label_);
    rect.x += index * (rect.w + normalized_x(attachment_slot_gap_));
    return rect;
}

SDL_FRect WorkbenchLayout::to_logical_rect(const NormalizedRect& rect) const
{
    return SDL_FRect{
        normalized_x(rect.x),
        normalized_y(rect.y),
        normalized_x(rect.w),
        normalized_y(rect.h)
    };
}

SDL_FRect WorkbenchLayout::child_rect(const LayoutContainerNode& container, const NormalizedRect& child) const
{
    const SDL_FRect parent = to_logical_rect(container.rect);
    return SDL_FRect{
        parent.x + parent.w * child.x,
        parent.y + parent.h * child.y,
        parent.w * child.w,
        parent.h * child.h
    };
}

float WorkbenchLayout::normalized_x(float value) const
{
    return value * static_cast<float>(kUiDesignWidth);
}

float WorkbenchLayout::normalized_y(float value) const
{
    return value * static_cast<float>(kUiDesignHeight);
}

} // namespace zg
