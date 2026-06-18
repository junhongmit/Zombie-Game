#include "ControlStyle.h"

#include "../AssetPaths.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace zg {

namespace {

float scaled_metric(int value, float style_scale)
{
    return std::round(static_cast<float>(value) * style_scale);
}

ControlInsets scaled_insets(const ControlInsets& insets, float style_scale)
{
    ControlInsets scaled;
    scaled.left = static_cast<int>(std::round(static_cast<float>(insets.left) * style_scale));
    scaled.right = static_cast<int>(std::round(static_cast<float>(insets.right) * style_scale));
    scaled.top = static_cast<int>(std::round(static_cast<float>(insets.top) * style_scale));
    scaled.bottom = static_cast<int>(std::round(static_cast<float>(insets.bottom) * style_scale));
    return scaled;
}

bool read_text_file(const char* path, std::string* out)
{
    const std::string resolved = resolve_asset_path(path);
    std::ifstream input(resolved.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        std::fprintf(stderr, "Failed to open %s\n", resolved.c_str());
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    *out = buffer.str();
    return true;
}

bool extract_object_value(const std::string& text, const char* key, std::string* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }

    size_t brace_pos = text.find('{', key_pos + token.size());
    if (brace_pos == std::string::npos) {
        return false;
    }

    int depth = 0;
    for (size_t i = brace_pos; i < text.size(); ++i) {
        if (text[i] == '{') {
            ++depth;
        } else if (text[i] == '}') {
            --depth;
            if (depth == 0) {
                *out = text.substr(brace_pos, i - brace_pos + 1);
                return true;
            }
        }
    }
    return false;
}

bool extract_string_value(const std::string& text, const char* key, std::string* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }

    size_t colon_pos = text.find(':', key_pos + token.size());
    if (colon_pos == std::string::npos) {
        return false;
    }

    size_t value_start = text.find('"', colon_pos + 1);
    if (value_start == std::string::npos) {
        return false;
    }
    const size_t value_end = text.find('"', value_start + 1);
    if (value_end == std::string::npos) {
        return false;
    }

    *out = text.substr(value_start + 1, value_end - value_start - 1);
    return true;
}

bool extract_int_value(const std::string& text, const char* key, int* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }

    size_t colon_pos = text.find(':', key_pos + token.size());
    if (colon_pos == std::string::npos) {
        return false;
    }

    size_t value_pos = colon_pos + 1;
    while (value_pos < text.size() && std::isspace(static_cast<unsigned char>(text[value_pos]))) {
        ++value_pos;
    }

    size_t end_pos = value_pos;
    if (end_pos < text.size() && (text[end_pos] == '-' || text[end_pos] == '+')) {
        ++end_pos;
    }
    while (end_pos < text.size() && std::isdigit(static_cast<unsigned char>(text[end_pos]))) {
        ++end_pos;
    }

    if (end_pos == value_pos) {
        return false;
    }

    *out = std::atoi(text.substr(value_pos, end_pos - value_pos).c_str());
    return true;
}

bool extract_float_value(const std::string& text, const char* key, float* out)
{
    const std::string token = std::string("\"") + key + "\"";
    const size_t key_pos = text.find(token);
    if (key_pos == std::string::npos) {
        return false;
    }

    size_t colon_pos = text.find(':', key_pos + token.size());
    if (colon_pos == std::string::npos) {
        return false;
    }

    size_t value_pos = colon_pos + 1;
    while (value_pos < text.size() && std::isspace(static_cast<unsigned char>(text[value_pos]))) {
        ++value_pos;
    }

    size_t end_pos = value_pos;
    if (end_pos < text.size() && (text[end_pos] == '-' || text[end_pos] == '+')) {
        ++end_pos;
    }
    bool saw_digit = false;
    while (end_pos < text.size()) {
        const unsigned char c = static_cast<unsigned char>(text[end_pos]);
        if (std::isdigit(c)) {
            saw_digit = true;
            ++end_pos;
            continue;
        }
        if (text[end_pos] == '.') {
            ++end_pos;
            continue;
        }
        break;
    }

    if (!saw_digit) {
        return false;
    }

    *out = static_cast<float>(std::atof(text.substr(value_pos, end_pos - value_pos).c_str()));
    return true;
}

bool parse_rect(const std::string& text, ControlRect* rect)
{
    return extract_int_value(text, "x", &rect->x) &&
        extract_int_value(text, "y", &rect->y) &&
        extract_int_value(text, "w", &rect->w) &&
        extract_int_value(text, "h", &rect->h);
}

bool parse_insets(const std::string& text, ControlInsets* insets)
{
    return extract_int_value(text, "left", &insets->left) &&
        extract_int_value(text, "right", &insets->right) &&
        extract_int_value(text, "top", &insets->top) &&
        extract_int_value(text, "bottom", &insets->bottom);
}

bool parse_fill(const std::string& state_obj, ControlFillStyle* fill)
{
    std::string fill_obj;
    if (!extract_object_value(state_obj, "fill", &fill_obj)) {
        return true;
    }

    std::string mode;
    if (extract_string_value(fill_obj, "mode", &mode)) {
        if (mode == "tile") {
            fill->mode = ControlFillMode::Tile;
        } else if (mode == "fixed") {
            fill->mode = ControlFillMode::Fixed;
        } else {
            fill->mode = ControlFillMode::Stretch;
        }
    }

    std::string rect_obj;
    if (!extract_object_value(fill_obj, "rect", &rect_obj) || !parse_rect(rect_obj, &fill->rect)) {
        return false;
    }

    std::string tile_obj;
    if (extract_object_value(fill_obj, "tile", &tile_obj)) {
        extract_int_value(tile_obj, "w", &fill->tile.x);
        extract_int_value(tile_obj, "h", &fill->tile.y);
    }

    fill->valid = true;
    return true;
}

ControlAnchor parse_anchor(const std::string& value);

bool parse_decor(const std::string& object_text, const char* key, ControlDecorStyle* decor)
{
    std::string decor_obj;
    if (!extract_object_value(object_text, key, &decor_obj)) {
        return false;
    }

    std::string rect_obj;
    if (!extract_object_value(decor_obj, "rect", &rect_obj) || !parse_rect(rect_obj, &decor->rect)) {
        return false;
    }

    std::string anchor_value;
    if (extract_string_value(decor_obj, "anchor", &anchor_value)) {
        decor->anchor = parse_anchor(anchor_value);
    }

    std::string offset_obj;
    if (extract_object_value(decor_obj, "offset", &offset_obj)) {
        extract_int_value(offset_obj, "x", &decor->offset.x);
        extract_int_value(offset_obj, "y", &decor->offset.y);
    }

    decor->valid = true;
    return true;
}

bool parse_state_style(const std::string& states_obj, const char* state_name, ControlStateStyle* out)
{
    std::string state_obj;
    if (!extract_object_value(states_obj, state_name, &state_obj)) {
        return false;
    }

    std::string frame_obj;
    if (!extract_object_value(state_obj, "frame", &frame_obj) || !parse_rect(frame_obj, &out->frame)) {
        return false;
    }

    std::string border_obj;
    std::string slice_obj;
    if (!extract_object_value(state_obj, "border", &border_obj) ||
        !extract_object_value(border_obj, "slice", &slice_obj) ||
        !parse_insets(slice_obj, &out->border)) {
        return false;
    }

    if (!parse_fill(state_obj, &out->fill)) {
        return false;
    }

    parse_decor(state_obj, "grip", &out->grip);

    out->valid = true;
    return true;
}

bool parse_named_region(const std::string& regions_obj, const char* key, bool* has_region, ControlRect* rect)
{
    std::string rect_obj;
    if (!extract_object_value(regions_obj, key, &rect_obj)) {
        *has_region = false;
        return true;
    }
    *has_region = parse_rect(rect_obj, rect);
    return *has_region;
}

ControlAnchor parse_anchor(const std::string& value)
{
    if (value == "top") return ControlAnchor::Top;
    if (value == "bottom") return ControlAnchor::Bottom;
    if (value == "left") return ControlAnchor::Left;
    if (value == "right") return ControlAnchor::Right;
    if (value == "top_left") return ControlAnchor::TopLeft;
    if (value == "top_right") return ControlAnchor::TopRight;
    if (value == "bottom_left") return ControlAnchor::BottomLeft;
    if (value == "bottom_right") return ControlAnchor::BottomRight;
    return ControlAnchor::Center;
}

void parse_regions(const std::string& control_obj, ControlRegions* regions)
{
    std::string regions_obj;
    if (!extract_object_value(control_obj, "regions", &regions_obj)) {
        return;
    }

    parse_named_region(regions_obj, "track", &regions->has_track, &regions->track);
    parse_named_region(regions_obj, "fill_track", &regions->has_fill_track, &regions->fill_track);
    parse_named_region(regions_obj, "grip", &regions->has_grip, &regions->grip);
    parse_named_region(regions_obj, "channel", &regions->has_channel, &regions->channel);
    parse_named_region(regions_obj, "thumb_bounds", &regions->has_thumb_bounds, &regions->thumb_bounds);
    parse_named_region(regions_obj, "icon", &regions->has_icon, &regions->icon);
    parse_named_region(regions_obj, "title", &regions->has_title, &regions->title);
    parse_named_region(regions_obj, "subtitle", &regions->has_subtitle, &regions->subtitle);
    parse_named_region(regions_obj, "badge", &regions->has_badge, &regions->badge);
    parse_named_region(regions_obj, "meta", &regions->has_meta, &regions->meta);
}

void parse_grip_decor(const std::string& control_obj, ControlDecorStyle* grip)
{
    parse_decor(control_obj, "grip", grip);
}

void parse_track_cross_ratio(const std::string& control_obj, bool* has_value, float* value)
{
    float parsed = 0.0f;
    if (extract_float_value(control_obj, "track_cross_ratio", &parsed)) {
        *has_value = true;
        *value = std::max(0.1f, std::min(1.0f, parsed));
    }
}

void inherit_missing_states(std::array<ControlStateStyle, 4>* states)
{
    ControlStateStyle* all_states = states->data();
    const ControlStateStyle& normal = all_states[static_cast<size_t>(ControlVisualState::Normal)];
    for (size_t i = 0; i < states->size(); ++i) {
        if (!all_states[i].valid) {
            all_states[i] = normal;
        }
    }
}

void render_textured_patch(SDL_Renderer* renderer, SDL_Texture* texture, const ControlRect& src, const SDL_FRect& dst)
{
    if (src.w <= 0 || src.h <= 0 || dst.w <= 0.0f || dst.h <= 0.0f) {
        return;
    }

    const SDL_FRect src_rect{
        static_cast<float>(src.x),
        static_cast<float>(src.y),
        static_cast<float>(src.w),
        static_cast<float>(src.h)
    };
    SDL_RenderTexture(renderer, texture, &src_rect, &dst);
}

void render_fill(SDL_Renderer* renderer, SDL_Texture* texture, const ControlFillStyle& fill, const SDL_FRect& dst)
{
    if (!fill.valid || dst.w <= 0.0f || dst.h <= 0.0f) {
        return;
    }

    if (fill.mode == ControlFillMode::Stretch) {
        render_textured_patch(renderer, texture, fill.rect, dst);
        return;
    }

    if (fill.mode == ControlFillMode::Fixed) {
        SDL_FRect fixed_dst{
            dst.x + std::floor((dst.w - static_cast<float>(fill.rect.w)) * 0.5f),
            dst.y + std::floor((dst.h - static_cast<float>(fill.rect.h)) * 0.5f),
            static_cast<float>(fill.rect.w),
            static_cast<float>(fill.rect.h)
        };
        render_textured_patch(renderer, texture, fill.rect, fixed_dst);
        return;
    }

    const int tile_w = fill.tile.x > 0 ? fill.tile.x : fill.rect.w;
    const int tile_h = fill.tile.y > 0 ? fill.tile.y : fill.rect.h;
    if (tile_w <= 0 || tile_h <= 0) {
        return;
    }

    for (float y = 0.0f; y < dst.h; y += static_cast<float>(tile_h)) {
        for (float x = 0.0f; x < dst.w; x += static_cast<float>(tile_w)) {
            const float draw_w = std::min(static_cast<float>(tile_w), dst.w - x);
            const float draw_h = std::min(static_cast<float>(tile_h), dst.h - y);
            ControlRect src = fill.rect;
            src.w = static_cast<int>(draw_w);
            src.h = static_cast<int>(draw_h);
            const SDL_FRect patch_dst{dst.x + x, dst.y + y, draw_w, draw_h};
            render_textured_patch(renderer, texture, src, patch_dst);
        }
    }
}

void render_fixed_region_centered(
    SDL_Renderer* renderer,
    SDL_Texture* texture,
    const ControlRect& source_frame,
    const ControlRect& local_region,
    const SDL_FRect& dst,
    float scale)
{
    if (scale <= 0.0f || local_region.w <= 0 || local_region.h <= 0 || dst.w <= 0.0f || dst.h <= 0.0f) {
        return;
    }

    const ControlRect src{
        source_frame.x + local_region.x,
        source_frame.y + local_region.y,
        local_region.w,
        local_region.h
    };

    const float draw_w = local_region.w * scale;
    const float draw_h = local_region.h * scale;
    const SDL_FRect grip_dst{
        dst.x + std::floor((dst.w - draw_w) * 0.5f),
        dst.y + std::floor((dst.h - draw_h) * 0.5f),
        draw_w,
        draw_h
    };
    render_textured_patch(renderer, texture, src, grip_dst);
}

SDL_FRect anchored_rect(const SDL_FRect& container, float w, float h, ControlAnchor anchor, float offset_x, float offset_y)
{
    float x = container.x;
    float y = container.y;

    switch (anchor) {
    case ControlAnchor::Center:
        x += std::floor((container.w - w) * 0.5f);
        y += std::floor((container.h - h) * 0.5f);
        break;
    case ControlAnchor::Top:
        x += std::floor((container.w - w) * 0.5f);
        break;
    case ControlAnchor::Bottom:
        x += std::floor((container.w - w) * 0.5f);
        y += container.h - h;
        break;
    case ControlAnchor::Left:
        y += std::floor((container.h - h) * 0.5f);
        break;
    case ControlAnchor::Right:
        x += container.w - w;
        y += std::floor((container.h - h) * 0.5f);
        break;
    case ControlAnchor::TopLeft:
        break;
    case ControlAnchor::TopRight:
        x += container.w - w;
        break;
    case ControlAnchor::BottomLeft:
        y += container.h - h;
        break;
    case ControlAnchor::BottomRight:
        x += container.w - w;
        y += container.h - h;
        break;
    }

    x += offset_x;
    y += offset_y;
    return SDL_FRect{x, y, w, h};
}

void render_fixed_decor(
    SDL_Renderer* renderer,
    SDL_Texture* texture,
    const ControlDecorStyle& decor,
    const SDL_FRect& dst,
    float scale)
{
    if (!decor.valid || decor.rect.w <= 0 || decor.rect.h <= 0 || dst.w <= 0.0f || dst.h <= 0.0f || scale <= 0.0f) {
        return;
    }

    const float draw_w = static_cast<float>(decor.rect.w) * scale;
    const float draw_h = static_cast<float>(decor.rect.h) * scale;
    const SDL_FRect decor_dst = anchored_rect(
        dst,
        draw_w,
        draw_h,
        decor.anchor,
        static_cast<float>(decor.offset.x) * scale,
        static_cast<float>(decor.offset.y) * scale);
    render_textured_patch(renderer, texture, decor.rect, decor_dst);
}

void render_nine_slice(SDL_Renderer* renderer, SDL_Texture* texture, const ControlStateStyle& state, const SDL_FRect& dst, float style_scale, bool render_center)
{
    const ControlInsets scaled = scaled_insets(state.border, style_scale);
    const float left = static_cast<float>(scaled.left);
    const float right = static_cast<float>(scaled.right);
    const float top = static_cast<float>(scaled.top);
    const float bottom = static_cast<float>(scaled.bottom);
    const float center_w = std::max(0.0f, dst.w - left - right);
    const float center_h = std::max(0.0f, dst.h - top - bottom);
    const int src_center_w = std::max(0, state.frame.w - state.border.left - state.border.right);
    const int src_center_h = std::max(0, state.frame.h - state.border.top - state.border.bottom);

    const ControlRect src_top_left{state.frame.x, state.frame.y, state.border.left, state.border.top};
    const ControlRect src_top{state.frame.x + state.border.left, state.frame.y, src_center_w, state.border.top};
    const ControlRect src_top_right{state.frame.x + state.frame.w - state.border.right, state.frame.y, state.border.right, state.border.top};
    const ControlRect src_left{state.frame.x, state.frame.y + state.border.top, state.border.left, src_center_h};
    const ControlRect src_center{state.frame.x + state.border.left, state.frame.y + state.border.top, src_center_w, src_center_h};
    const ControlRect src_right{state.frame.x + state.frame.w - state.border.right, state.frame.y + state.border.top, state.border.right, src_center_h};
    const ControlRect src_bottom_left{state.frame.x, state.frame.y + state.frame.h - state.border.bottom, state.border.left, state.border.bottom};
    const ControlRect src_bottom{state.frame.x + state.border.left, state.frame.y + state.frame.h - state.border.bottom, src_center_w, state.border.bottom};
    const ControlRect src_bottom_right{state.frame.x + state.frame.w - state.border.right, state.frame.y + state.frame.h - state.border.bottom, state.border.right, state.border.bottom};

    const SDL_FRect dst_top_left{dst.x, dst.y, left, top};
    const SDL_FRect dst_top{dst.x + left, dst.y, center_w, top};
    const SDL_FRect dst_top_right{dst.x + dst.w - right, dst.y, right, top};
    const SDL_FRect dst_left{dst.x, dst.y + top, left, center_h};
    const SDL_FRect dst_center{dst.x + left, dst.y + top, center_w, center_h};
    const SDL_FRect dst_right{dst.x + dst.w - right, dst.y + top, right, center_h};
    const SDL_FRect dst_bottom_left{dst.x, dst.y + dst.h - bottom, left, bottom};
    const SDL_FRect dst_bottom{dst.x + left, dst.y + dst.h - bottom, center_w, bottom};
    const SDL_FRect dst_bottom_right{dst.x + dst.w - right, dst.y + dst.h - bottom, right, bottom};

    render_textured_patch(renderer, texture, src_top_left, dst_top_left);
    render_textured_patch(renderer, texture, src_top, dst_top);
    render_textured_patch(renderer, texture, src_top_right, dst_top_right);
    render_textured_patch(renderer, texture, src_left, dst_left);
    if (render_center) {
        render_textured_patch(renderer, texture, src_center, dst_center);
    }
    render_textured_patch(renderer, texture, src_right, dst_right);
    render_textured_patch(renderer, texture, src_bottom_left, dst_bottom_left);
    render_textured_patch(renderer, texture, src_bottom, dst_bottom);
    render_textured_patch(renderer, texture, src_bottom_right, dst_bottom_right);
}

} // namespace

bool ControlStyle::load(SDL_Renderer* renderer, const char* image_path, const char* metadata_path, const char* control_name)
{
    texture_.reset();
    states_ = {};
    content_padding_ = {};
    regions_ = {};
    grip_decor_ = {};
    has_track_cross_ratio_ = false;
    track_cross_ratio_ = 1.0f;
    style_scale_ = 1.0f;
    has_max_size_ = false;
    max_size_ = SDL_Point{0, 0};
    min_size_ = SDL_Point{0, 0};

    if (!texture_.load(renderer, image_path, true)) {
        return false;
    }

    std::string text;
    if (!read_text_file(metadata_path, &text)) {
        return false;
    }

    std::string controls_obj;
    std::string control_obj;
    std::string states_obj;
    if (!extract_object_value(text, "controls", &controls_obj) ||
        !extract_object_value(controls_obj, control_name, &control_obj) ||
        !extract_object_value(control_obj, "states", &states_obj)) {
        return false;
    }

    if (!parse_state_style(states_obj, "normal", &states_[static_cast<size_t>(ControlVisualState::Normal)])) {
        return false;
    }
    parse_state_style(states_obj, "hover", &states_[static_cast<size_t>(ControlVisualState::Hover)]);
    parse_state_style(states_obj, "pressed", &states_[static_cast<size_t>(ControlVisualState::Pressed)]);
    parse_state_style(states_obj, "disabled", &states_[static_cast<size_t>(ControlVisualState::Disabled)]);
    inherit_missing_states(&states_);
    parse_regions(control_obj, &regions_);
    parse_grip_decor(control_obj, &grip_decor_);
    parse_track_cross_ratio(control_obj, &has_track_cross_ratio_, &track_cross_ratio_);
    extract_float_value(control_obj, "style_scale", &style_scale_);

    std::string padding_obj;
    if (extract_object_value(control_obj, "content_padding", &padding_obj)) {
        parse_insets(padding_obj, &content_padding_);
    }

    std::string min_size_obj;
    if (extract_object_value(control_obj, "min_size", &min_size_obj)) {
        extract_int_value(min_size_obj, "w", &min_size_.x);
        extract_int_value(min_size_obj, "h", &min_size_.y);
    }

    std::string max_size_obj;
    if (extract_object_value(control_obj, "max_size", &max_size_obj)) {
        extract_int_value(max_size_obj, "w", &max_size_.x);
        extract_int_value(max_size_obj, "h", &max_size_.y);
        has_max_size_ = max_size_.x > 0 || max_size_.y > 0;
    }

    return true;
}

bool ControlStyle::valid() const
{
    return texture_.valid() && state(ControlVisualState::Normal).valid;
}

void ControlStyle::render(SDL_Renderer* renderer, const SDL_FRect& dst, ControlVisualState visual_state, Uint8 alpha) const
{
    if (!valid()) {
        return;
    }

    const ControlStateStyle& active = resolved_state(visual_state);
    const ControlInsets scaled = scaled_insets(active.border, style_scale_);

    SDL_SetTextureAlphaMod(texture_.get(), alpha);
    const SDL_FRect fill_dst{
        dst.x + static_cast<float>(scaled.left),
        dst.y + static_cast<float>(scaled.top),
        std::max(0.0f, dst.w - static_cast<float>(scaled.left + scaled.right)),
        std::max(0.0f, dst.h - static_cast<float>(scaled.top + scaled.bottom))
    };
    render_fill(renderer, texture_.get(), active.fill, fill_dst);
    render_nine_slice(renderer, texture_.get(), active, dst, style_scale_, !active.fill.valid);
    SDL_SetTextureAlphaMod(texture_.get(), 255);
}

void ControlStyle::render_grip(SDL_Renderer* renderer, const SDL_FRect& dst, ControlVisualState visual_state, float scale, Uint8 alpha) const
{
    if (!valid()) {
        return;
    }

    SDL_SetTextureAlphaMod(texture_.get(), alpha);
    const ControlStateStyle& active = resolved_state(visual_state);
    const float final_scale = scale * style_scale_;
    if (active.grip.valid) {
        render_fixed_decor(renderer, texture_.get(), active.grip, dst, final_scale);
    } else if (grip_decor_.valid) {
        render_fixed_decor(renderer, texture_.get(), grip_decor_, dst, final_scale);
    } else if (regions_.has_grip) {
        render_fixed_region_centered(renderer, texture_.get(), active.frame, regions_.grip, dst, final_scale);
    }
    SDL_SetTextureAlphaMod(texture_.get(), 255);
}

SDL_FRect ControlStyle::content_rect(const SDL_FRect& dst) const
{
    const ControlInsets scaled = scaled_insets(content_padding_, style_scale_);
    return SDL_FRect{
        dst.x + static_cast<float>(scaled.left),
        dst.y + static_cast<float>(scaled.top),
        std::max(0.0f, dst.w - static_cast<float>(scaled.left + scaled.right)),
        std::max(0.0f, dst.h - static_cast<float>(scaled.top + scaled.bottom))
    };
}

SDL_FRect ControlStyle::map_region(const ControlRect& region, const SDL_FRect& dst, ControlVisualState visual_state) const
{
    const ControlStateStyle& active = resolved_state(visual_state);
    const float scaled_frame_w = std::max(1.0f, static_cast<float>(active.frame.w) * style_scale_);
    const float scaled_frame_h = std::max(1.0f, static_cast<float>(active.frame.h) * style_scale_);
    const float x_scale = dst.w / scaled_frame_w;
    const float y_scale = dst.h / scaled_frame_h;
    return SDL_FRect{
        dst.x + scaled_metric(region.x, style_scale_) * x_scale,
        dst.y + scaled_metric(region.y, style_scale_) * y_scale,
        scaled_metric(region.w, style_scale_) * x_scale,
        scaled_metric(region.h, style_scale_) * y_scale
    };
}

SDL_FRect ControlStyle::fit_region_to_rect(const ControlRect& region, const SDL_FRect& mapped_rect, ControlVisualState visual_state) const
{
    const ControlStateStyle& active = resolved_state(visual_state);
    const float scaled_frame_w = std::max(1.0f, static_cast<float>(active.frame.w) * style_scale_);
    const float scaled_frame_h = std::max(1.0f, static_cast<float>(active.frame.h) * style_scale_);
    const float scaled_region_w = std::max(1.0f, scaled_metric(region.w, style_scale_));
    const float scaled_region_h = std::max(1.0f, scaled_metric(region.h, style_scale_));
    const float scaled_region_x = scaled_metric(region.x, style_scale_);
    const float scaled_region_y = scaled_metric(region.y, style_scale_);

    const float x_scale = mapped_rect.w / scaled_region_w;
    const float y_scale = mapped_rect.h / scaled_region_h;
    const float dst_w = scaled_frame_w * x_scale;
    const float dst_h = scaled_frame_h * y_scale;
    return SDL_FRect{
        mapped_rect.x - scaled_region_x * x_scale,
        mapped_rect.y - scaled_region_y * y_scale,
        dst_w,
        dst_h
    };
}

const ControlStateStyle& ControlStyle::state(ControlVisualState visual_state) const
{
    return states_[static_cast<size_t>(visual_state)];
}

const ControlStateStyle& ControlStyle::resolved_state(ControlVisualState visual_state) const
{
    const ControlStateStyle& selected = state(visual_state);
    return selected.valid ? selected : state(ControlVisualState::Normal);
}

} // namespace zg
