#include "Item.h"

#include <algorithm>

namespace zg {
namespace ui {

Item& Item::set_rect(float x, float y, float w, float h)
{
    logical_rect_ = SDL_FRect{x, y, w, h};
    return *this;
}

Item& Item::set_visible(bool visible)
{
    visible_ = visible;
    return *this;
}

Item& Item::set_enabled(bool enabled)
{
    enabled_ = enabled;
    return *this;
}

Item& Item::set_layout_mode(LayoutMode mode)
{
    layout_mode_ = mode;
    return *this;
}

Item& Item::set_slot_name(const char* slot_name)
{
    slot_name_ = slot_name != nullptr ? slot_name : "";
    return *this;
}

Item& Item::set_property(const char* key, const char* value)
{
    if (key == nullptr || key[0] == '\0') {
        return *this;
    }
    properties_[key] = value != nullptr ? value : "";
    return *this;
}

const std::string* Item::property(const char* key) const
{
    if (key == nullptr) {
        return nullptr;
    }
    const std::unordered_map<std::string, std::string>::const_iterator it = properties_.find(key);
    return it != properties_.end() ? &it->second : nullptr;
}

SDL_FRect Item::resolve_rect(const SDL_FRect& parent_rect) const
{
    switch (layout_mode_) {
    case LayoutMode::Fill:
        return SDL_FRect{
            parent_rect.x + logical_rect_.x,
            parent_rect.y + logical_rect_.y,
            std::max(0.0f, parent_rect.w - logical_rect_.x - logical_rect_.w),
            std::max(0.0f, parent_rect.h - logical_rect_.y - logical_rect_.h)
        };
    case LayoutMode::Center:
        return SDL_FRect{
            parent_rect.x + (parent_rect.w - logical_rect_.w) * 0.5f + logical_rect_.x,
            parent_rect.y + (parent_rect.h - logical_rect_.h) * 0.5f + logical_rect_.y,
            logical_rect_.w,
            logical_rect_.h
        };
    case LayoutMode::Left:
        return SDL_FRect{
            parent_rect.x + logical_rect_.x,
            parent_rect.y + (parent_rect.h - logical_rect_.h) * 0.5f + logical_rect_.y,
            logical_rect_.w,
            logical_rect_.h
        };
    case LayoutMode::Right:
        return SDL_FRect{
            parent_rect.x + parent_rect.w - logical_rect_.w - logical_rect_.x,
            parent_rect.y + (parent_rect.h - logical_rect_.h) * 0.5f + logical_rect_.y,
            logical_rect_.w,
            logical_rect_.h
        };
    case LayoutMode::Top:
        return SDL_FRect{
            parent_rect.x + (parent_rect.w - logical_rect_.w) * 0.5f + logical_rect_.x,
            parent_rect.y + logical_rect_.y,
            logical_rect_.w,
            logical_rect_.h
        };
    case LayoutMode::Bottom:
        return SDL_FRect{
            parent_rect.x + (parent_rect.w - logical_rect_.w) * 0.5f + logical_rect_.x,
            parent_rect.y + parent_rect.h - logical_rect_.h - logical_rect_.y,
            logical_rect_.w,
            logical_rect_.h
        };
    case LayoutMode::Manual:
    default:
        return SDL_FRect{
            parent_rect.x + logical_rect_.x,
            parent_rect.y + logical_rect_.y,
            logical_rect_.w,
            logical_rect_.h
        };
    }
}

} // namespace ui
} // namespace zg
