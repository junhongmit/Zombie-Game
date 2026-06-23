#pragma once

#include "Item.h"

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <vector>

namespace zg {
namespace ui {

class Container : public Item {
public:
    Container() = default;
    virtual ~Container() {}
    Container(const Container&) = delete;
    Container& operator=(const Container&) = delete;
    Container(Container&&) = default;
    Container& operator=(Container&&) = default;

    Container& add_child(std::unique_ptr<Item> child);
    Container& add_child_to_slot(const char* slot_name, std::unique_ptr<Item> child);
    void clear_children();

    const std::vector<std::unique_ptr<Item> >& children() const { return children_; }
    virtual SDL_FRect content_rect(const SDL_FRect& resolved_rect) const { return resolved_rect; }
    virtual SDL_FRect slot_rect(const std::string& slot_name, const SDL_FRect& resolved_rect) const;

protected:
    void render_children(const RenderContext& context, const SDL_FRect& resolved_rect) const;

private:
    std::vector<std::unique_ptr<Item> > children_;
};

inline Container& Container::add_child(std::unique_ptr<Item> child)
{
    if (child) {
        child->set_parent(this);
        children_.push_back(std::move(child));
    }
    return *this;
}

inline Container& Container::add_child_to_slot(const char* slot_name, std::unique_ptr<Item> child)
{
    if (child) {
        child->set_slot_name(slot_name);
        add_child(std::move(child));
    }
    return *this;
}

inline void Container::clear_children()
{
    children_.clear();
}

inline void Container::render_children(const RenderContext& context, const SDL_FRect& resolved_rect) const
{
    const SDL_FRect child_parent_rect = content_rect(resolved_rect);
    for (size_t i = 0; i < children_.size(); ++i) {
        if (children_[i] && children_[i]->visible()) {
            children_[i]->render(context, slot_rect(children_[i]->slot_name(), child_parent_rect));
        }
    }
}

inline SDL_FRect Container::slot_rect(const std::string& slot_name, const SDL_FRect& resolved_rect) const
{
    (void)slot_name;
    return resolved_rect;
}

} // namespace ui
} // namespace zg
