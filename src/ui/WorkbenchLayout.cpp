#include "WorkbenchLayout.h"

#include "../Constants.h"
#include "LayoutJson.h"

namespace zg {

namespace {

void parse_weapon_row_template(const std::string& object_text, WeaponRowTemplateLayout* layout)
{
    if (layout == nullptr) {
        return;
    }
    parse_named_rect(object_text, "icon", &layout->icon);
    parse_named_rect(object_text, "title", &layout->title);
    parse_named_rect(object_text, "subtitle", &layout->subtitle);
    extract_float_value(object_text, "title_scale", &layout->title_scale);
    extract_float_value(object_text, "subtitle_scale", &layout->subtitle_scale);
    extract_float_value(object_text, "title_y_offset", &layout->title_y_offset);
    extract_float_value(object_text, "subtitle_y_offset", &layout->subtitle_y_offset);
}

} // namespace

bool WorkbenchLayout::load(const char* asset_path)
{
    std::string text;
    if (!read_layout_text_file(asset_path, &text)) {
        return false;
    }

    std::string rects_object;
    if (extract_object_value(text, "rects", &rects_object)) {
        parse_named_rect(rects_object, "resource_strip", &resource_strip_);
        parse_named_rect(rects_object, "close_button", &close_button_);
        parse_container_node(rects_object, "weapon_list_panel", &weapon_list_);
        parse_panel_node(rects_object, "preview_panel", &preview_panel_);
        parse_named_rect(rects_object, "preview_nameplate", &preview_nameplate_);
        parse_panel_node(rects_object, "stats_panel", &stats_panel_);
        parse_panel_node(rects_object, "effects_panel", &effects_panel_);
        parse_panel_node(rects_object, "upgrade_panel", &upgrade_panel_);
        parse_container_node(rects_object, "attachments_panel", &attachments_);
        parse_panel_node(rects_object, "parts_panel", &parts_panel_);
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

    std::string templates_object;
    if (extract_object_value(text, "weapon_row_templates", &templates_object)) {
        std::string template_object;
        if (extract_object_value(templates_object, "default", &template_object)) {
            parse_weapon_row_template(template_object, &weapon_row_template_default_);
        }
        if (extract_object_value(templates_object, "pistol", &template_object)) {
            parse_weapon_row_template(template_object, &weapon_row_template_pistol_);
        }
        if (extract_object_value(templates_object, "rifle", &template_object)) {
            parse_weapon_row_template(template_object, &weapon_row_template_rifle_);
        }
        if (extract_object_value(templates_object, "smg", &template_object)) {
            parse_weapon_row_template(template_object, &weapon_row_template_smg_);
        }
        if (extract_object_value(templates_object, "sniper", &template_object)) {
            parse_weapon_row_template(template_object, &weapon_row_template_sniper_);
        }
        if (extract_object_value(templates_object, "shotgun", &template_object)) {
            parse_weapon_row_template(template_object, &weapon_row_template_shotgun_);
        }
    }

    return true;
}

SDL_FRect WorkbenchLayout::resource_strip_rect() const
{
    return to_logical_rect(resource_strip_);
}

SDL_FRect WorkbenchLayout::close_button_rect() const
{
    return to_logical_rect(close_button_);
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
    return to_logical_rect(preview_panel_.rect);
}

SDL_FRect WorkbenchLayout::preview_nameplate_rect() const
{
    return to_logical_rect(preview_nameplate_);
}

SDL_FRect WorkbenchLayout::stats_panel_rect() const
{
    return to_logical_rect(stats_panel_.rect);
}

SDL_FRect WorkbenchLayout::effects_panel_rect() const
{
    return to_logical_rect(effects_panel_.rect);
}

SDL_FRect WorkbenchLayout::upgrade_panel_rect() const
{
    return to_logical_rect(upgrade_panel_.rect);
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
    return to_logical_rect(parts_panel_.rect);
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

const WeaponRowTemplateLayout& WorkbenchLayout::weapon_row_template(const std::string& name) const
{
    if (name == "pistol") {
        return weapon_row_template_pistol_;
    }
    if (name == "rifle") {
        return weapon_row_template_rifle_;
    }
    if (name == "smg") {
        return weapon_row_template_smg_;
    }
    if (name == "sniper") {
        return weapon_row_template_sniper_;
    }
    if (name == "shotgun") {
        return weapon_row_template_shotgun_;
    }
    return weapon_row_template_default_;
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
