#pragma once

#include "../Texture.h"

#include <SDL3/SDL.h>

#include <array>

namespace zg {

enum class ControlVisualState {
    Normal = 0,
    Hover,
    Pressed,
    Disabled
};

struct ControlInsets {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
};

struct ControlRect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    ControlRect() {}
    ControlRect(int x_value, int y_value, int w_value, int h_value)
        : x(x_value), y(y_value), w(w_value), h(h_value) {}
};

enum class ControlFillMode {
    Stretch,
    Tile,
    Fixed
};

enum class ControlAnchor {
    Center,
    Top,
    Bottom,
    Left,
    Right,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

struct ControlFillStyle {
    ControlFillMode mode = ControlFillMode::Stretch;
    ControlRect rect;
    SDL_Point tile{0, 0};
    bool valid = false;
};

struct ControlDecorStyle {
    ControlRect rect;
    ControlAnchor anchor = ControlAnchor::Center;
    SDL_Point offset{0, 0};
    bool valid = false;
};

struct ControlStateStyle {
    ControlRect frame;
    ControlInsets border;
    ControlFillStyle fill;
    ControlDecorStyle grip;
    bool valid = false;
};

struct ControlRegions {
    bool has_track = false;
    ControlRect track;
    bool has_fill_track = false;
    ControlRect fill_track;
    bool has_grip = false;
    ControlRect grip;
    bool has_channel = false;
    ControlRect channel;
    bool has_thumb_bounds = false;
    ControlRect thumb_bounds;
    bool has_icon = false;
    ControlRect icon;
    bool has_title = false;
    ControlRect title;
    bool has_subtitle = false;
    ControlRect subtitle;
    bool has_badge = false;
    ControlRect badge;
    bool has_meta = false;
    ControlRect meta;
};

class ControlStyle {
public:
    bool load(SDL_Renderer* renderer, const char* image_path, const char* metadata_path, const char* control_name);
    bool valid() const;

    void render(SDL_Renderer* renderer, const SDL_FRect& dst, ControlVisualState state, Uint8 alpha = 255) const;
    void render_grip(SDL_Renderer* renderer, const SDL_FRect& dst, ControlVisualState state, float scale, Uint8 alpha = 255) const;
    SDL_FRect content_rect(const SDL_FRect& dst) const;
    SDL_FRect map_region(const ControlRect& region, const SDL_FRect& dst, ControlVisualState state) const;
    SDL_FRect fit_region_to_rect(const ControlRect& region, const SDL_FRect& mapped_rect, ControlVisualState state) const;
    float style_scale() const { return style_scale_; }
    SDL_Point min_size() const { return min_size_; }
    bool has_max_size() const { return has_max_size_; }
    SDL_Point max_size() const { return max_size_; }
    const ControlRect* track_region() const { return regions_.has_track ? &regions_.track : nullptr; }
    const ControlRect* fill_track_region() const { return regions_.has_fill_track ? &regions_.fill_track : nullptr; }
    const ControlRect* grip_region() const { return regions_.has_grip ? &regions_.grip : nullptr; }
    const ControlRect* channel_region() const { return regions_.has_channel ? &regions_.channel : nullptr; }
    const ControlRect* thumb_bounds_region() const { return regions_.has_thumb_bounds ? &regions_.thumb_bounds : nullptr; }
    const ControlRect* icon_region() const { return regions_.has_icon ? &regions_.icon : nullptr; }
    const ControlRect* title_region() const { return regions_.has_title ? &regions_.title : nullptr; }
    const ControlRect* subtitle_region() const { return regions_.has_subtitle ? &regions_.subtitle : nullptr; }
    const ControlRect* badge_region() const { return regions_.has_badge ? &regions_.badge : nullptr; }
    const ControlRect* meta_region() const { return regions_.has_meta ? &regions_.meta : nullptr; }
    const ControlStateStyle& resolved_state(ControlVisualState state) const;
    bool has_track_cross_ratio() const { return has_track_cross_ratio_; }
    float track_cross_ratio() const { return track_cross_ratio_; }

private:
    const ControlStateStyle& state(ControlVisualState state) const;

    Texture texture_;
    std::array<ControlStateStyle, 4> states_{};
    ControlInsets content_padding_;
    ControlRegions regions_;
    ControlDecorStyle grip_decor_;
    bool has_track_cross_ratio_ = false;
    float track_cross_ratio_ = 1.0f;
    float style_scale_ = 1.0f;
    SDL_Point min_size_{0, 0};
    bool has_max_size_ = false;
    SDL_Point max_size_{0, 0};
};

} // namespace zg
