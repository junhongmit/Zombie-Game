#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <unordered_map>

namespace zg {
namespace ui {

struct RenderContext {
    SDL_Renderer* renderer = nullptr;
    SDL_FRect presentation_rect{};
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    bool mouse_in_view = false;
    bool mouse_down = false;
    bool mouse_pressed = false;
    bool mouse_released = false;
    Uint8 alpha = 255;
};

enum class LayoutMode {
    Manual = 0,
    Fill,
    Center,
    Left,
    Right,
    Top,
    Bottom
};

class Container;

class Item {
public:
    Item() = default;
    virtual ~Item() {}
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;
    Item(Item&&) = default;
    Item& operator=(Item&&) = default;

    Item& set_rect(float x, float y, float w, float h);
    Item& set_visible(bool visible);
    Item& set_enabled(bool enabled);
    Item& set_layout_mode(LayoutMode mode);
    Item& set_slot_name(const char* slot_name);
    Item& set_property(const char* key, const char* value);

    const SDL_FRect& logical_rect() const { return logical_rect_; }
    bool visible() const { return visible_; }
    bool enabled() const { return enabled_; }
    LayoutMode layout_mode() const { return layout_mode_; }
    const std::string& slot_name() const { return slot_name_; }
    const std::unordered_map<std::string, std::string>& properties() const { return properties_; }
    const std::string* property(const char* key) const;

    SDL_FRect resolve_rect(const SDL_FRect& parent_rect) const;
    virtual void render(const RenderContext& context, const SDL_FRect& parent_rect) const = 0;

protected:
    friend class Container;
    void set_parent(const Container* parent) { parent_ = parent; }
    const Container* parent() const { return parent_; }

private:
    SDL_FRect logical_rect_{};
    bool visible_ = true;
    bool enabled_ = true;
    LayoutMode layout_mode_ = LayoutMode::Manual;
    std::string slot_name_;
    const Container* parent_ = nullptr;
    std::unordered_map<std::string, std::string> properties_;
};

} // namespace ui
} // namespace zg
