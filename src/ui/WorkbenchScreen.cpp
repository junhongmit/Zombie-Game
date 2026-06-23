#include "WorkbenchScreen.h"

#include "../Assets.h"
#include "../Constants.h"
#include "../Presentation.h"
#include "../Texture.h"
#include "../Weapon.h"
#include "Card.h"
#include "Button.h"
#include "BoxItem.h"
#include "Canvas.h"
#include "Panel.h"
#include "TextItem.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <memory>

namespace zg {

namespace {

TTF_Font* load_ui_font(int point_size)
{
    static const char* kCandidates[] = {
        "C:\\Windows\\Fonts\\msyh.ttc",
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\lucon.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
    };

    for (const char* path : kCandidates) {
        TTF_Font* font = TTF_OpenFont(path, point_size);
        if (font != nullptr) {
            return font;
        }
    }
    return nullptr;
}

float ui_scale_for_rect(const SDL_FRect& rect)
{
    return std::min(
        rect.w / static_cast<float>(kUiDesignWidth),
        rect.h / static_cast<float>(kUiDesignHeight));
}

void render_texture_fit(
    SDL_Renderer* renderer,
    const Texture& texture,
    const SDL_FRect& dst_rect)
{
    if (!texture.valid() || dst_rect.w <= 0.0f || dst_rect.h <= 0.0f) {
        return;
    }

    const float scale = std::min(
        dst_rect.w / std::max(1.0f, texture.width()),
        dst_rect.h / std::max(1.0f, texture.height()));
    const float draw_w = std::round(texture.width() * scale);
    const float draw_h = std::round(texture.height() * scale);
    const SDL_FRect dst{
        dst_rect.x + std::floor((dst_rect.w - draw_w) * 0.5f),
        dst_rect.y + std::floor((dst_rect.h - draw_h) * 0.5f),
        draw_w,
        draw_h
    };
    SDL_RenderTexture(renderer, texture.get(), nullptr, &dst);
}

SDL_FRect rect_within(const SDL_FRect& parent, const NormalizedRect& child)
{
    return SDL_FRect{
        parent.x + parent.w * child.x,
        parent.y + parent.h * child.y,
        parent.w * child.w,
        parent.h * child.h
    };
}

ControlVisualState weapon_row_state(
    int index,
    int active_index,
    const SDL_FRect& row_rect,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    int armed_index)
{
    if (index == active_index) {
        return ControlVisualState::Pressed;
    }

    const bool hovered = mouse_in_view &&
        mouse_x >= row_rect.x && mouse_x <= row_rect.x + row_rect.w &&
        mouse_y >= row_rect.y && mouse_y <= row_rect.y + row_rect.h;
    if (hovered && mouse_down && armed_index == index) {
        return ControlVisualState::Pressed;
    }
    if (hovered) {
        return ControlVisualState::Hover;
    }
    return ControlVisualState::Normal;
}

ui::Panel make_panel(float x, float y, float w, float h, const char* title)
{
    ui::Panel panel(x, y, w, h);
    panel.set_title(title);
    return panel;
}

SDL_FRect panel_content_logical_rect(
    const ControlStyle& style,
    const SDL_FRect& logical_rect,
    const SDL_FRect& presentation_rect)
{
    return ui_present_to_logical_rect(
        style.content_rect(ui_logical_to_present_rect(logical_rect, presentation_rect)),
        presentation_rect);
}

const ControlStyle& resolve_skin(const Assets& assets, const std::string& skin_name, const ControlStyle& fallback)
{
    if (const ControlStyle* skin = assets.find_ui_skin(skin_name)) {
        return *skin;
    }
    if (skin_name == "panel_square_bronze") {
        return assets.panel_skin;
    }
    if (skin_name == "card1_weapon_row") {
        return assets.weapon_card_style;
    }
    if (skin_name == "button_square_bronze") {
        return assets.title_button_skin;
    }
    return fallback;
}

struct AttachmentOption {
    const char* name;
    int owned;
    bool locked;
};

struct AttachmentSlotData {
    const char* key;
    const char* fallback;
    const char* equipped_name;
    const AttachmentOption* options;
    int option_count;
};

constexpr AttachmentOption kMuzzleOptions[] = {
    {"Suppressor", 1, false},
    {"Compensator", 1, false},
    {"Flash Hider", 1, false},
    {"Heavy Suppressor", 0, true},
    {"Experimental Can", 0, true},
};

constexpr AttachmentOption kBarrelOptions[] = {
    {"Short Barrel", 1, false},
    {"Long Barrel", 1, false},
    {"Match Barrel", 0, true},
};

constexpr AttachmentOption kSightOptions[] = {
    {"Iron Sight", 1, false},
    {"Red Dot", 1, false},
    {"Holo Sight", 0, true},
};

constexpr AttachmentOption kMagazineOptions[] = {
    {"Standard Mag", 1, false},
    {"Extended Mag", 1, false},
    {"Drum Mag", 0, true},
};

constexpr AttachmentOption kGripOptions[] = {
    {"Polymer Grip", 1, false},
    {"Rubber Grip", 1, false},
    {"Tactical Grip", 0, true},
};

constexpr AttachmentOption kStockOptions[] = {
    {"No Stock", 0, true},
};

constexpr AttachmentSlotData kAttachmentSlots[] = {
    {"attachment.muzzle", "Muzzle", "Suppressor", kMuzzleOptions, static_cast<int>(sizeof(kMuzzleOptions) / sizeof(kMuzzleOptions[0]))},
    {"attachment.barrel", "Barrel", "Long Barrel", kBarrelOptions, static_cast<int>(sizeof(kBarrelOptions) / sizeof(kBarrelOptions[0]))},
    {"attachment.sight", "Sight", "Red Dot", kSightOptions, static_cast<int>(sizeof(kSightOptions) / sizeof(kSightOptions[0]))},
    {"attachment.magazine", "Magazine", "Extended Mag", kMagazineOptions, static_cast<int>(sizeof(kMagazineOptions) / sizeof(kMagazineOptions[0]))},
    {"attachment.grip", "Grip", "Polymer Grip", kGripOptions, static_cast<int>(sizeof(kGripOptions) / sizeof(kGripOptions[0]))},
    {"attachment.stock", "Stock", "Locked", kStockOptions, static_cast<int>(sizeof(kStockOptions) / sizeof(kStockOptions[0]))},
};

} // namespace

WorkbenchScreen::WorkbenchScreen(SDL_Renderer* renderer)
    : renderer_(renderer),
      weapon_list_view_(0.0f, 0.0f, 1.0f, 1.0f, ui::ListViewOrientation::Vertical),
      attachment_strip_view_(0.0f, 0.0f, 1.0f, 1.0f, ui::ListViewOrientation::Horizontal),
      attachment_popup_view_(0.0f, 0.0f, 1.0f, 1.0f, ui::ListViewOrientation::Horizontal)
{
    layout_hot_reload_.set_path("assets/ui/layouts/workbench.json");
    layout_.load("assets/ui/layouts/workbench.json");
    layout_hot_reload_.mark_loaded();
    if (!strings_.load("assets/localization/zh-cn/workbench.loc")) {
        strings_.load("assets/localization/us-en/workbench.loc");
    }
    font_point_size_ = static_cast<int>(kHudFontPointSize);
    font_ = load_ui_font(font_point_size_);
    list_title_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 1.95f)));
    list_subtitle_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 1.55f)));
}

WorkbenchScreen::~WorkbenchScreen()
{
    if (list_title_font_ != nullptr) {
        TTF_CloseFont(list_title_font_);
        list_title_font_ = nullptr;
    }
    if (list_subtitle_font_ != nullptr) {
        TTF_CloseFont(list_subtitle_font_);
        list_subtitle_font_ = nullptr;
    }
    if (font_ != nullptr) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
}

bool WorkbenchScreen::render(
    const Assets& assets,
    WeaponState& weapon_state,
    const SDL_FRect& presentation_rect,
    float dt,
    float wheel_x,
    float wheel_y,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool mouse_pressed,
    bool mouse_released)
{
    bool close_requested = false;
    if (kEnableUiLayoutHotReload && layout_hot_reload_.poll_changed()) {
        layout_.load("assets/ui/layouts/workbench.json");
    }
    presentation_rect_ = presentation_rect;
    ensure_font(ui_scale_for_rect(presentation_rect_));

    const SDL_FRect full_screen_rect{
        0.0f,
        0.0f,
        static_cast<float>(kInternalRenderWidth),
        static_cast<float>(kInternalRenderHeight)
    };
    SDL_RenderTexture(renderer_, assets.bench.get(), nullptr, &full_screen_rect);

    render_resource_strip();
    const SDL_FRect close_rect = layout_.close_button_rect();
    ui::Button close_button("X", close_rect.x, close_rect.y, close_rect.w, close_rect.h, true);
    close_requested = close_button.process_pointer(
        mouse_x,
        mouse_y,
        mouse_in_view,
        mouse_pressed,
        mouse_released,
        close_armed_);
    close_button.render(
        renderer_,
        assets.title_button_skin,
        font_,
        presentation_rect_,
        mouse_x,
        mouse_y,
        mouse_in_view,
        mouse_down,
        close_armed_,
        SDL_Color{228, 205, 170, 255},
        SDL_Color{156, 142, 122, 255},
        236);

    const SDL_Color panel_text{241, 226, 205, 255};
    const SDL_FRect stats_panel = layout_.stats_panel_rect();
    const SDL_FRect effects_panel = layout_.effects_panel_rect();
    const SDL_FRect upgrade_panel = layout_.upgrade_panel_rect();
    make_panel(layout_.weapon_list_panel_rect().x, layout_.weapon_list_panel_rect().y, layout_.weapon_list_panel_rect().w, layout_.weapon_list_panel_rect().h, "").render(renderer_, resolve_skin(assets, layout_.weapon_list_panel_skin(), assets.panel_skin), font_, presentation_rect_, panel_text);
    make_panel(layout_.attachments_panel_rect().x, layout_.attachments_panel_rect().y, layout_.attachments_panel_rect().w, layout_.attachments_panel_rect().h, "").render(renderer_, resolve_skin(assets, layout_.attachments_panel_skin(), assets.panel_skin), font_, presentation_rect_, panel_text);
    make_panel(stats_panel.x, stats_panel.y, stats_panel.w, stats_panel.h, tr("panel.weapon_stats", "Stats").c_str()).render(renderer_, resolve_skin(assets, layout_.stats_panel_skin(), assets.panel_skin), font_, presentation_rect_, panel_text);
    make_panel(effects_panel.x, effects_panel.y, effects_panel.w, effects_panel.h, tr("panel.current_effects", "Effects").c_str()).render(renderer_, resolve_skin(assets, layout_.effects_panel_skin(), assets.panel_skin), font_, presentation_rect_, panel_text);
    make_panel(upgrade_panel.x, upgrade_panel.y, upgrade_panel.w, upgrade_panel.h, tr("panel.upgrade_requirements", "Upgrade Cost").c_str()).render(renderer_, resolve_skin(assets, layout_.upgrade_panel_skin(), assets.panel_skin), font_, presentation_rect_, panel_text);
    render_text_centered(strings_.resolve_token(layout_.weapon_list_title_text()).c_str(), layout_.weapon_list_title_rect(), panel_text, -1.0f);
    render_text_centered(strings_.resolve_token(layout_.attachments_title_text()).c_str(), layout_.attachments_title_rect(), panel_text, -1.0f);

    render_weapon_list(assets, weapon_state, dt, wheel_y, mouse_x, mouse_y, mouse_in_view, mouse_down, mouse_pressed, mouse_released);
    render_preview_nameplate(assets, weapon_state.current_definition(), layout_.preview_nameplate_rect());
    render_weapon_preview(assets, weapon_state.current_definition(), layout_.preview_panel_rect());
    render_weapon_stats(assets, weapon_state.current_definition(), stats_panel);
    render_attachment_popup(assets, dt, wheel_x, mouse_x, mouse_y, mouse_in_view, mouse_down, mouse_pressed, mouse_released);
    render_attachment_strip(assets, dt, wheel_x, mouse_x, mouse_y, mouse_in_view, mouse_down, mouse_pressed, mouse_released);

    ui::Canvas effects_canvas(effects_panel.x, effects_panel.y, effects_panel.w, effects_panel.h);
    const char* effect_lines[] = {
        tr("effects.accuracy_up", "+ Accuracy 10").c_str(),
        tr("effects.reload_up", "+ Reload Speed 15%").c_str(),
    };
    for (int i = 0; i < 2; ++i) {
        std::unique_ptr<ui::TextItem> line(new ui::TextItem());
        line->set_rect(22.0f, 64.0f + i * 30.0f, effects_panel.w - 44.0f, 22.0f);
        line->set_font(font_);
        line->set_text(effect_lines[i]);
        line->set_color(SDL_Color{116, 188, 84, 255});
        line->set_fit_to_bounds(false);
        effects_canvas.add_child(std::move(line));
    }

    ui::Canvas upgrade_canvas(upgrade_panel.x, upgrade_panel.y, upgrade_panel.w, upgrade_panel.h);
    const char* upgrade_lines[] = {
        tr("upgrade.metal_parts", "Metal Parts 78 / 15").c_str(),
        tr("upgrade.polymer_parts", "Polymer Parts 23 / 10").c_str(),
        tr("upgrade.fabric", "Fabric 12 / 5").c_str(),
        tr("upgrade.tool_level", "Tool Bench Lv. 2").c_str(),
    };
    for (int i = 0; i < 4; ++i) {
        std::unique_ptr<ui::TextItem> line(new ui::TextItem());
        line->set_rect(22.0f, 54.0f + i * 34.0f, upgrade_panel.w - 44.0f, 24.0f);
        line->set_font(font_);
        line->set_text(upgrade_lines[i]);
        line->set_color(SDL_Color{241, 226, 205, 255});
        line->set_fit_to_bounds(false);
        upgrade_canvas.add_child(std::move(line));
    }

    ui::RenderContext panel_context;
    panel_context.renderer = renderer_;
    panel_context.presentation_rect = presentation_rect_;
    panel_context.alpha = 255;
    effects_canvas.render(panel_context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});
    upgrade_canvas.render(panel_context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});
    return close_requested;
}

void WorkbenchScreen::ensure_font(float ui_scale)
{
    const int desired_point_size = std::max(
        static_cast<int>(kHudFontPointSize),
        static_cast<int>(std::round(kHudFontPointSize * ui_scale)));
    if (font_ != nullptr && desired_point_size == font_point_size_) {
        return;
    }

    if (font_ != nullptr) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if (list_title_font_ != nullptr) {
        TTF_CloseFont(list_title_font_);
        list_title_font_ = nullptr;
    }
    if (list_subtitle_font_ != nullptr) {
        TTF_CloseFont(list_subtitle_font_);
        list_subtitle_font_ = nullptr;
    }
    font_point_size_ = desired_point_size;
    font_ = load_ui_font(font_point_size_);
    list_title_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 1.95f)));
    list_subtitle_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 1.55f)));
}

void WorkbenchScreen::render_text(const char* text, float x, float y, SDL_Color color) const
{
    render_text_with_font(font_, text, x, y, color);
}

void WorkbenchScreen::render_text_with_font(TTF_Font* font, const char* text, float x, float y, SDL_Color color) const
{
    if (font == nullptr || text == nullptr || text[0] == '\0') {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, color);
    if (surface == nullptr) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (texture == nullptr) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    const SDL_FRect dst{
        to_screen_x(x),
        to_screen_y(y),
        static_cast<float>(surface->w),
        static_cast<float>(surface->h)
    };
    SDL_RenderTexture(renderer_, texture, nullptr, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void render_text_texture_fit(
    SDL_Renderer* renderer,
    TTF_Font* font,
    const char* text,
    const SDL_FRect& region,
    SDL_Color color,
    float height_scale = 1.0f,
    float y_offset = 0.0f)
{
    if (renderer == nullptr || font == nullptr || text == nullptr || text[0] == '\0' || region.w <= 0.0f || region.h <= 0.0f) {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, color);
    if (surface == nullptr) {
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    const float effective_height = std::max(1.0f, region.h * height_scale);
    const float scale = std::min(region.w / std::max(1.0f, static_cast<float>(surface->w)), effective_height / std::max(1.0f, static_cast<float>(surface->h)));
    const float draw_w = std::round(surface->w * scale);
    const float draw_h = std::round(surface->h * scale);
    const SDL_FRect dst{
        region.x,
        region.y + y_offset,
        draw_w,
        draw_h
    };
    SDL_RenderTexture(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void WorkbenchScreen::render_text_centered(const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset) const
{
    if (font_ == nullptr || text == nullptr || text[0] == '\0') {
        return;
    }

    int text_width = 0;
    int text_height = 0;
    if (!TTF_GetStringSize(font_, text, 0, &text_width, &text_height)) {
        return;
    }

    const float x_scale = presentation_rect_.w / static_cast<float>(kUiDesignWidth);
    const float y_scale = presentation_rect_.h / static_cast<float>(kUiDesignHeight);
    const float logical_text_width = static_cast<float>(text_width) / x_scale;
    const float logical_text_height = static_cast<float>(text_height) / y_scale;
    const float x = rect.x + std::floor((rect.w - logical_text_width) * 0.5f);
    const float y = rect.y + std::floor((rect.h - logical_text_height) * 0.5f) + y_offset;
    render_text(text, x, y, color);
}

void WorkbenchScreen::render_weapon_preview(const Assets&, const WeaponDefinition* definition, const SDL_FRect& logical_rect) const
{
    if (definition == nullptr) {
        return;
    }

    const Texture* preview = definition->workbench_lit_texture.valid()
        ? &definition->workbench_lit_texture
        : (definition->preview_texture.valid() ? &definition->preview_texture : &definition->texture);
    if (!preview->valid()) {
        return;
    }

    const SDL_FRect screen_rect = to_screen_rect(logical_rect);
    const bool use_half_width = preview == &definition->texture;
    const float frame_width = use_half_width ? preview->width() * 0.5f : preview->width();
    const float frame_height = preview->height();
    const SDL_FRect src{0.0f, 0.0f, frame_width, frame_height};

    const float padding = 82.0f * (presentation_rect_.w / static_cast<float>(kUiDesignWidth));
    const float max_width = std::max(10.0f, screen_rect.w - padding * 2.0f);
    const float max_height = std::max(10.0f, screen_rect.h - padding * 2.1f);
    const float scale = std::min(max_width / frame_width, max_height / frame_height) * 0.84f;
    const float draw_width = std::round(frame_width * scale);
    const float draw_height = std::round(frame_height * scale);
    const float draw_x = screen_rect.x + std::floor((screen_rect.w - draw_width) * 0.5f);
    const float draw_y = screen_rect.y + std::floor((screen_rect.h - draw_height) * 0.5f) +
        38.0f * (presentation_rect_.h / static_cast<float>(kUiDesignHeight));

    if (definition->workbench_shadow_texture.valid()) {
        const SDL_FRect shadow_dst{
            draw_x - definition->workbench_shadow_placement.anchor_x * scale,
            draw_y - definition->workbench_shadow_placement.anchor_y * scale,
            definition->workbench_shadow_texture.width() * scale,
            definition->workbench_shadow_texture.height() * scale
        };
        SDL_SetTextureScaleMode(definition->workbench_shadow_texture.get(), SDL_SCALEMODE_LINEAR);
        SDL_RenderTexture(renderer_, definition->workbench_shadow_texture.get(), nullptr, &shadow_dst);
    }

    const SDL_FRect dst{draw_x, draw_y, draw_width, draw_height};
    SDL_RenderTexture(renderer_, preview->get(), &src, &dst);
}

void WorkbenchScreen::render_preview_nameplate(const Assets& assets, const WeaponDefinition* definition, const SDL_FRect& logical_rect) const
{
    if (definition == nullptr) {
        return;
    }

    const SDL_FRect screen_rect = to_screen_rect(logical_rect);
    assets.title_button_skin.render(renderer_, screen_rect, ControlVisualState::Normal, 236);
    render_text_centered(definition->name.c_str(), logical_rect, SDL_Color{241, 226, 205, 255}, -1.0f);
}

void WorkbenchScreen::render_weapon_list(
    Assets const& assets,
    WeaponState& weapon_state,
    float dt,
    float wheel_y,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool mouse_pressed,
    bool mouse_released)
{
    const WeaponSlot* slots = weapon_state.slots();
    weapon_list_view_
        .set_rect(
            layout_.weapon_list_view_rect().x,
            layout_.weapon_list_view_rect().y,
            layout_.weapon_list_view_rect().w,
            layout_.weapon_list_view_rect().h)
        .set_title("")
        .set_enabled(true)
        .set_content_height(layout_.weapon_list_content_height(weapon_state.slot_count()))
        .set_content_padding(10.0f, 10.0f)
        .set_scrollbar_gap(8.0f)
        .set_scrollbar_width(14.0f)
        .set_draw_panel(false)
        .set_draw_title(false)
        .set_always_show_scrollbar(true);
    if (wheel_y != 0.0f && weapon_list_view_.contains(mouse_x, mouse_y, mouse_in_view)) {
        weapon_list_view_.nudge_scroll(-wheel_y * 0.12f);
    }
    weapon_list_view_.update_and_render(
        renderer_,
        assets.panel_skin,
        assets.scrollbar_vertical_track_style,
        assets.scrollbar_vertical_fill_style,
        assets.scrollbar_vertical_thumb_style,
        font_,
        presentation_rect_,
        SDL_Color{241, 226, 205, 255},
        dt,
        mouse_x,
        mouse_y,
        mouse_in_view,
        mouse_down,
        mouse_pressed,
        mouse_released);

    const SDL_FRect viewport = weapon_list_view_.viewport_rect();
    const float scroll_offset = weapon_list_view_.scroll_offset();

    if (mouse_pressed) {
        armed_weapon_index_ = -1;
        for (int i = 0; i < weapon_state.slot_count(); ++i) {
            const SDL_FRect row_rect = layout_.weapon_row_rect(viewport, i, scroll_offset);
            if (mouse_in_view &&
                mouse_x >= row_rect.x &&
                mouse_x <= row_rect.x + row_rect.w &&
                mouse_y >= row_rect.y &&
                mouse_y <= row_rect.y + row_rect.h &&
                mouse_y >= viewport.y &&
                mouse_y <= viewport.y + viewport.h) {
                armed_weapon_index_ = i;
                break;
            }
        }
    }

    if (mouse_released && armed_weapon_index_ >= 0) {
        const SDL_FRect armed_rect = layout_.weapon_row_rect(viewport, armed_weapon_index_, scroll_offset);
        const bool released_inside = mouse_in_view &&
            mouse_x >= armed_rect.x && mouse_x <= armed_rect.x + armed_rect.w &&
            mouse_y >= armed_rect.y && mouse_y <= armed_rect.y + armed_rect.h &&
            mouse_y >= viewport.y && mouse_y <= viewport.y + viewport.h;
        if (released_inside) {
            weapon_state.switch_to_slot(armed_weapon_index_);
        }
        armed_weapon_index_ = -1;
    }

    const SDL_FRect present_viewport = to_screen_rect(viewport);
    const SDL_Rect clip_rect{
        static_cast<int>(std::round(present_viewport.x)),
        static_cast<int>(std::round(present_viewport.y)),
        static_cast<int>(std::round(present_viewport.w)),
        static_cast<int>(std::round(present_viewport.h))
    };
    SDL_SetRenderClipRect(renderer_, &clip_rect);

    for (int i = 0; i < weapon_state.slot_count(); ++i) {
        const WeaponSlot& slot = slots[i];
        if (slot.definition == nullptr) {
            continue;
        }

        const SDL_FRect row_rect = layout_.weapon_row_rect(viewport, i, scroll_offset);
        if (row_rect.y + row_rect.h < viewport.y || row_rect.y > viewport.y + viewport.h) {
            continue;
        }

        const Texture* card_icon = slot.definition->icon_texture.valid()
            ? &slot.definition->icon_texture
            : (slot.definition->preview_texture.valid()
                ? &slot.definition->preview_texture
                : &slot.definition->texture);
        const ControlVisualState state = weapon_row_state(
            i,
            weapon_state.active_slot_index(),
            row_rect,
            mouse_x,
            mouse_y,
            mouse_in_view,
            mouse_down,
            armed_weapon_index_);
        const SDL_FRect row_screen = to_screen_rect(row_rect);
        assets.weapon_card_style.render(renderer_, row_screen, state, 255);

        const SDL_FRect content = assets.weapon_card_style.content_rect(row_screen);
        const WeaponRowTemplateLayout& card_template = layout_.weapon_row_template(slot.definition->ui_card_template);
        const SDL_FRect icon_rect = rect_within(content, card_template.icon);
        const SDL_FRect title_rect = rect_within(content, card_template.title);
        const SDL_FRect subtitle_rect = rect_within(content, card_template.subtitle);

        render_texture_fit(renderer_, *card_icon, icon_rect);
        render_text_texture_fit(
            renderer_,
            list_title_font_ != nullptr ? list_title_font_ : font_,
            slot.definition->name.c_str(),
            title_rect,
            SDL_Color{241, 226, 205, 255},
            card_template.title_scale,
            content.h * card_template.title_y_offset);
        render_text_texture_fit(
            renderer_,
            list_subtitle_font_ != nullptr ? list_subtitle_font_ : font_,
            weapon_category_label(*slot.definition).c_str(),
            subtitle_rect,
            SDL_Color{186, 160, 122, 255},
            card_template.subtitle_scale,
            content.h * card_template.subtitle_y_offset);
    }

    SDL_SetRenderClipRect(renderer_, nullptr);
}

void WorkbenchScreen::render_weapon_stats(const Assets& assets, const WeaponDefinition* definition, const SDL_FRect& logical_rect) const
{
    if (definition == nullptr) {
        return;
    }

    struct StatRow {
        const char* label;
        float ratio;
        char value[32];
    };

    StatRow rows[] = {
        {tr("stat.damage", "Damage").c_str(), std::min(1.0f, definition->damage / 100.0f), ""},
        {tr("stat.fire_rate", "Fire Rate").c_str(), std::min(1.0f, definition->speed_rpm / 1000.0f), ""},
        {tr("stat.magazine", "Magazine").c_str(), std::min(1.0f, definition->magazine_size / 60.0f), ""},
        {tr("stat.recoil", "Recoil").c_str(), std::min(1.0f, definition->shake_magnitude / 8.0f), ""},
        {tr("stat.loudness", "Loudness").c_str(), std::min(1.0f, definition->loudness), ""},
    };
    std::snprintf(rows[0].value, sizeof(rows[0].value), "%d", definition->damage);
    std::snprintf(rows[1].value, sizeof(rows[1].value), "%d", definition->speed_rpm);
    std::snprintf(rows[2].value, sizeof(rows[2].value), "%d", definition->magazine_size);
    std::snprintf(rows[3].value, sizeof(rows[3].value), "%.1f", definition->shake_magnitude);
    std::snprintf(rows[4].value, sizeof(rows[4].value), "%.2f", definition->loudness);

    ui::Canvas stats_canvas(logical_rect.x, logical_rect.y, logical_rect.w, logical_rect.h);
    for (int i = 0; i < 5; ++i) {
        const float row_y = 56.0f + i * 74.0f;

        std::unique_ptr<ui::TextItem> label(new ui::TextItem());
        label->set_rect(28.0f, row_y, logical_rect.w - 120.0f, 20.0f);
        label->set_font(font_);
        label->set_text(rows[i].label);
        label->set_color(SDL_Color{241, 226, 205, 255});
        label->set_fit_to_bounds(false);
        stats_canvas.add_child(std::move(label));

        std::unique_ptr<ui::TextItem> value(new ui::TextItem());
        value->set_rect(logical_rect.w - 96.0f, row_y, 68.0f, 20.0f);
        value->set_font(font_);
        value->set_text(rows[i].value);
        value->set_color(SDL_Color{241, 226, 205, 255});
        value->set_fit_to_bounds(false);
        value->set_horizontal_align(ui::HorizontalAlign::Right);
        stats_canvas.add_child(std::move(value));
    }

    ui::RenderContext stats_context;
    stats_context.renderer = renderer_;
    stats_context.presentation_rect = presentation_rect_;
    stats_context.alpha = 255;
    stats_canvas.render(stats_context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});

    for (int i = 0; i < 5; ++i) {
        const float row_y = logical_rect.y + 56.0f + i * 74.0f;
        ui::ProgressBar meter(
            logical_rect.x + 26.0f,
            row_y + 28.0f,
            logical_rect.w - 52.0f,
            22.0f,
            ui::ProgressBarOrientation::Horizontal);
        meter
            .set_enabled(true)
            .set_fill_origin(ui::ProgressBarFillOrigin::Start)
            .set_progress(rows[i].ratio);
        meter.render(
            renderer_,
            assets.progressbar_horizontal_track_style,
            assets.progressbar_horizontal_fill_style,
            presentation_rect_);
    }
}

void WorkbenchScreen::render_resource_strip() const
{
    const SDL_FRect logical_rect = layout_.resource_strip_rect();
    ui::Canvas strip(logical_rect.x, logical_rect.y, logical_rect.w, logical_rect.h);
    std::unique_ptr<ui::BoxItem> background(new ui::BoxItem());
    background->set_rect(0.0f, 0.0f, logical_rect.w, logical_rect.h);
    background->set_fill_color(SDL_Color{8, 10, 12, 168});
    background->set_border_color(SDL_Color{88, 72, 42, 220});
    background->set_fill_enabled(true);
    background->set_border_enabled(true);
    background->set_border_width(1.0f);
    strip.add_child(std::move(background));

    struct StripLabel {
        float x;
        float w;
        const std::string* text;
        SDL_Color color;
    };
    const std::string workbench = tr("top.workbench", "Workbench");
    const std::string level = tr("top.level", "Lv. 2");
    const std::string metal = tr("top.metal", "Metal 350");
    const std::string wood = tr("top.wood", "Wood 126");
    const std::string parts = tr("top.parts", "Parts 78");
    const std::string fabric = tr("top.fabric", "Fabric 23");
    const std::string gems = tr("top.gems", "Gems 0");
    const std::string back = tr("top.back", "Esc Back");
    const StripLabel labels[] = {
        {16.0f, 110.0f, &workbench, SDL_Color{241, 226, 205, 255}},
        {136.0f, 110.0f, &level, SDL_Color{211, 188, 142, 255}},
        {268.0f, 170.0f, &metal, SDL_Color{241, 226, 205, 255}},
        {470.0f, 170.0f, &wood, SDL_Color{241, 226, 205, 255}},
        {676.0f, 160.0f, &parts, SDL_Color{241, 226, 205, 255}},
        {874.0f, 150.0f, &fabric, SDL_Color{241, 226, 205, 255}},
        {1074.0f, 120.0f, &gems, SDL_Color{241, 226, 205, 255}},
        {logical_rect.w - 230.0f, 180.0f, &back, SDL_Color{211, 188, 142, 255}},
    };
    for (size_t i = 0; i < sizeof(labels) / sizeof(labels[0]); ++i) {
        std::unique_ptr<ui::TextItem> text(new ui::TextItem());
        text->set_rect(labels[i].x, 8.0f, labels[i].w, logical_rect.h - 16.0f);
        text->set_font(font_);
        text->set_text(labels[i].text->c_str());
        text->set_color(labels[i].color);
        text->set_fit_to_bounds(false);
        text->set_vertical_align(ui::VerticalAlign::Middle);
        strip.add_child(std::move(text));
    }

    ui::RenderContext context;
    context.renderer = renderer_;
    context.presentation_rect = presentation_rect_;
    context.alpha = 255;
    strip.render(context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});
}

void WorkbenchScreen::render_attachment_strip(
    const Assets& assets,
    float dt,
    float wheel_x,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool mouse_pressed,
    bool mouse_released)
{
    const float card_w = 156.0f;
    const float gap = 12.0f;
    attachment_strip_view_
        .set_rect(layout_.attachments_view_rect().x, layout_.attachments_view_rect().y, layout_.attachments_view_rect().w, layout_.attachments_view_rect().h)
        .set_orientation(ui::ListViewOrientation::Horizontal)
        .set_title("")
        .set_enabled(true)
        .set_content_padding(12.0f, 10.0f)
        .set_scrollbar_gap(8.0f)
        .set_scrollbar_width(12.0f)
        .set_content_extent(layout_.attachment_slot_count() * card_w + std::max(0, layout_.attachment_slot_count() - 1) * gap)
        .set_draw_panel(false)
        .set_draw_title(false)
        .set_always_show_scrollbar(true);
    if (wheel_x != 0.0f && attachment_strip_view_.contains(mouse_x, mouse_y, mouse_in_view)) {
        attachment_strip_view_.nudge_scroll(wheel_x * 0.12f);
    }
    attachment_strip_view_.update_and_render(
        renderer_,
        assets.panel_skin,
        assets.scrollbar_horizontal_track_style,
        assets.scrollbar_horizontal_fill_style,
        assets.scrollbar_horizontal_thumb_style,
        font_,
        presentation_rect_,
        SDL_Color{241, 226, 205, 255},
        dt,
        mouse_x,
        mouse_y,
        mouse_in_view,
        mouse_down,
        mouse_pressed,
        mouse_released);
    const SDL_FRect viewport = attachment_strip_view_.viewport_rect();
    const float card_h = viewport.h;
    const float scroll_offset = attachment_strip_view_.scroll_offset();

    attachment_anchor_rect_ = SDL_FRect{};
    if (mouse_pressed) {
        armed_attachment_index_ = -1;
        for (int i = 0; i < layout_.attachment_slot_count(); ++i) {
            const SDL_FRect slot_rect{
                viewport.x + i * (card_w + gap) - scroll_offset,
                viewport.y,
                card_w,
                card_h
            };
            const bool hovered = mouse_in_view &&
                mouse_x >= slot_rect.x && mouse_x <= slot_rect.x + slot_rect.w &&
                mouse_y >= slot_rect.y && mouse_y <= slot_rect.y + slot_rect.h &&
                mouse_x >= viewport.x && mouse_x <= viewport.x + viewport.w &&
                mouse_y >= viewport.y && mouse_y <= viewport.y + viewport.h;
            if (hovered) {
                armed_attachment_index_ = i;
                break;
            }
        }
    }

    if (mouse_released && armed_attachment_index_ >= 0) {
        const SDL_FRect armed_rect{
            viewport.x + armed_attachment_index_ * (card_w + gap) - scroll_offset,
            viewport.y,
            card_w,
            card_h
        };
        const bool released_inside = mouse_in_view &&
            mouse_x >= armed_rect.x && mouse_x <= armed_rect.x + armed_rect.w &&
            mouse_y >= armed_rect.y && mouse_y <= armed_rect.y + armed_rect.h &&
            mouse_x >= viewport.x && mouse_x <= viewport.x + viewport.w &&
            mouse_y >= viewport.y && mouse_y <= viewport.y + viewport.h;
        if (released_inside) {
            active_attachment_index_ = (active_attachment_index_ == armed_attachment_index_) ? -1 : armed_attachment_index_;
        }
        armed_attachment_index_ = -1;
    }

    const SDL_FRect present_viewport = to_screen_rect(viewport);
    const SDL_Rect clip_rect{
        static_cast<int>(std::round(present_viewport.x)),
        static_cast<int>(std::round(present_viewport.y)),
        static_cast<int>(std::round(present_viewport.w)),
        static_cast<int>(std::round(present_viewport.h))
    };
    SDL_SetRenderClipRect(renderer_, &clip_rect);

    for (int i = 0; i < layout_.attachment_slot_count(); ++i) {
        const SDL_FRect slot_rect{
            viewport.x + i * (card_w + gap) - scroll_offset,
            viewport.y,
            card_w,
            card_h
        };
        if (slot_rect.x + slot_rect.w < viewport.x || slot_rect.x > viewport.x + viewport.w) {
            continue;
        }

        if (active_attachment_index_ == i) {
            attachment_anchor_rect_ = slot_rect;
        }

        const int selected_option = selected_attachment_option_[i];
        const char* equipped_name = kAttachmentSlots[i].equipped_name;
        if (selected_option >= 0 && selected_option < kAttachmentSlots[i].option_count) {
            equipped_name = kAttachmentSlots[i].options[selected_option].name;
        }

        ui::Card card(slot_rect.x, slot_rect.y, slot_rect.w, slot_rect.h);
        card
            .set_selected(active_attachment_index_ == i)
            .set_title(tr(kAttachmentSlots[i].key, kAttachmentSlots[i].fallback).c_str())
            .set_subtitle(equipped_name)
            .set_meta(selected_attachment_option_[i] >= 0 ? tr("attachment.meta.equipped", "Equipped").c_str() : tr("attachment.meta.locked", "Locked").c_str());
        card.render(
            renderer_,
            assets.weapon_card_style,
            font_,
            font_,
            presentation_rect_,
            mouse_x,
            mouse_y,
            mouse_in_view,
            mouse_down,
            armed_attachment_index_ == i,
            SDL_Color{241, 226, 205, 255},
            SDL_Color{201, 182, 150, 255},
            SDL_Color{168, 150, 124, 255});
    }

    SDL_SetRenderClipRect(renderer_, nullptr);
}

void WorkbenchScreen::render_attachment_popup(
    const Assets& assets,
    float dt,
    float wheel_x,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool mouse_pressed,
    bool mouse_released)
{
    if (active_attachment_index_ < 0 || active_attachment_index_ >= layout_.attachment_slot_count()) {
        armed_popup_option_index_ = -1;
        return;
    }

    const SDL_FRect attachments_panel = layout_.attachments_panel_rect();
    const SDL_FRect anchor_rect = attachment_anchor_rect_.w > 0.0f ? attachment_anchor_rect_ : layout_.attachment_slot_rect(active_attachment_index_);
    const SDL_FRect popup_rect{
        attachments_panel.x + 12.0f,
        attachments_panel.y - 156.0f,
        attachments_panel.w - 24.0f,
        136.0f
    };
    const AttachmentSlotData& slot = kAttachmentSlots[active_attachment_index_];

    const float arrow_center_x = anchor_rect.x + anchor_rect.w * 0.5f;
    const SDL_FPoint arrow_points[] = {
        {to_screen_x(arrow_center_x - 10.0f), to_screen_y(popup_rect.y + popup_rect.h - 1.0f)},
        {to_screen_x(arrow_center_x + 10.0f), to_screen_y(popup_rect.y + popup_rect.h - 1.0f)},
        {to_screen_x(arrow_center_x), to_screen_y(anchor_rect.y - 4.0f)}
    };
    SDL_RenderLines(renderer_, arrow_points, 3);
    SDL_RenderLine(renderer_, arrow_points[2].x, arrow_points[2].y, arrow_points[0].x, arrow_points[0].y);
    SDL_RenderLine(renderer_, arrow_points[2].x, arrow_points[2].y, arrow_points[1].x, arrow_points[1].y);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);

    char title_buffer[128];
    std::snprintf(
        title_buffer,
        sizeof(title_buffer),
        "%s - %s",
        tr("popup.available_parts", "Available Parts").c_str(),
        tr(slot.key, slot.fallback).c_str());

    const float card_w = 132.0f;
    const float gap = 12.0f;
    attachment_popup_view_
        .set_rect(popup_rect.x, popup_rect.y, popup_rect.w, popup_rect.h)
        .set_orientation(ui::ListViewOrientation::Horizontal)
        .set_title(title_buffer)
        .set_enabled(true)
        .set_content_padding(18.0f, 28.0f)
        .set_scrollbar_gap(8.0f)
        .set_scrollbar_width(12.0f)
        .set_content_extent(slot.option_count * card_w + std::max(0, slot.option_count - 1) * gap)
        .set_always_show_scrollbar(slot.option_count > 0);
    if (wheel_x != 0.0f && attachment_popup_view_.contains(mouse_x, mouse_y, mouse_in_view)) {
        attachment_popup_view_.nudge_scroll(wheel_x * 0.12f);
    }
    attachment_popup_view_.update_and_render(
        renderer_,
        assets.panel_skin,
        assets.scrollbar_horizontal_track_style,
        assets.scrollbar_horizontal_fill_style,
        assets.scrollbar_horizontal_thumb_style,
        font_,
        presentation_rect_,
        SDL_Color{232, 212, 172, 255},
        dt,
        mouse_x,
        mouse_y,
        mouse_in_view,
        mouse_down,
        mouse_pressed,
        mouse_released,
        244);
    const SDL_FRect viewport = attachment_popup_view_.viewport_rect();
    const float card_h = viewport.h;
    const float scroll_offset = attachment_popup_view_.scroll_offset();

    if (mouse_pressed) {
        armed_popup_option_index_ = -1;
        for (int i = 0; i < slot.option_count; ++i) {
            const SDL_FRect card_rect{
                viewport.x + i * (card_w + gap) - scroll_offset,
                viewport.y,
                card_w,
                card_h
            };
            const bool hovered = mouse_in_view &&
                mouse_x >= card_rect.x && mouse_x <= card_rect.x + card_rect.w &&
                mouse_y >= card_rect.y && mouse_y <= card_rect.y + card_rect.h &&
                mouse_x >= viewport.x && mouse_x <= viewport.x + viewport.w &&
                mouse_y >= viewport.y && mouse_y <= viewport.y + viewport.h;
            if (hovered) {
                armed_popup_option_index_ = i;
                break;
            }
        }
    }
    if (mouse_released && armed_popup_option_index_ >= 0) {
        const SDL_FRect card_rect{
            viewport.x + armed_popup_option_index_ * (card_w + gap) - scroll_offset,
            viewport.y,
            card_w,
            card_h
        };
        const bool hovered = mouse_in_view &&
            mouse_x >= card_rect.x && mouse_x <= card_rect.x + card_rect.w &&
            mouse_y >= card_rect.y && mouse_y <= card_rect.y + card_rect.h &&
            mouse_x >= viewport.x && mouse_x <= viewport.x + viewport.w &&
            mouse_y >= viewport.y && mouse_y <= viewport.y + viewport.h;
        if (hovered && !slot.options[armed_popup_option_index_].locked) {
            selected_attachment_option_[active_attachment_index_] = armed_popup_option_index_;
        }
        armed_popup_option_index_ = -1;
    }

    const SDL_FRect present_viewport = to_screen_rect(viewport);
    const SDL_Rect clip_rect{
        static_cast<int>(std::round(present_viewport.x)),
        static_cast<int>(std::round(present_viewport.y)),
        static_cast<int>(std::round(present_viewport.w)),
        static_cast<int>(std::round(present_viewport.h))
    };
    SDL_SetRenderClipRect(renderer_, &clip_rect);
    for (int i = 0; i < slot.option_count; ++i) {
        const AttachmentOption& option = slot.options[i];
        const SDL_FRect card_rect{
            viewport.x + i * (card_w + gap) - scroll_offset,
            viewport.y,
            card_w,
            card_h
        };
        ui::Card card(card_rect.x, card_rect.y, card_rect.w, card_rect.h, !option.locked);
        char meta[32];
        std::snprintf(meta, sizeof(meta), "Owned: %d", option.owned);
        card
            .set_selected(selected_attachment_option_[active_attachment_index_] == i)
            .set_title(option.name)
            .set_subtitle(option.locked ? "Locked" : "Available")
            .set_meta(meta);
        card.render(
            renderer_,
            assets.weapon_card_style,
            font_,
            font_,
            presentation_rect_,
            mouse_x,
            mouse_y,
            mouse_in_view,
            mouse_down,
            armed_popup_option_index_ == i,
            option.locked ? SDL_Color{178, 88, 88, 255} : SDL_Color{241, 226, 205, 255},
            option.locked ? SDL_Color{144, 124, 124, 255} : SDL_Color{201, 182, 150, 255},
            SDL_Color{168, 150, 124, 255});
    }
    SDL_SetRenderClipRect(renderer_, nullptr);

    if (mouse_pressed) {
        const bool over_popup = mouse_in_view &&
            mouse_x >= popup_rect.x && mouse_x <= popup_rect.x + popup_rect.w &&
            mouse_y >= popup_rect.y && mouse_y <= popup_rect.y + popup_rect.h;
        const bool over_anchor = mouse_in_view &&
            mouse_x >= anchor_rect.x && mouse_x <= anchor_rect.x + anchor_rect.w &&
            mouse_y >= anchor_rect.y && mouse_y <= anchor_rect.y + anchor_rect.h;
        if (!over_popup && !over_anchor) {
            active_attachment_index_ = -1;
            armed_popup_option_index_ = -1;
        }
    }
}

const std::string& WorkbenchScreen::tr(const char* key, const char* fallback) const
{
    return strings_.get(key, fallback);
}

const std::string& WorkbenchScreen::weapon_category_label(const WeaponDefinition& definition) const
{
    if (definition.ui_card_template == "pistol") {
        return tr("weapon.category.pistol", "Pistol");
    }
    if (definition.ui_card_template == "rifle") {
        return tr("weapon.category.rifle", "Rifle");
    }
    if (definition.ui_card_template == "shotgun") {
        return tr("weapon.category.shotgun", "Shotgun");
    }
    if (definition.ui_card_template == "sniper") {
        return tr("weapon.category.sniper", "Sniper");
    }
    if (definition.ui_card_template == "smg") {
        return tr("weapon.category.smg", "SMG");
    }
    if (definition.type == WeaponType::Grenade) {
        return tr("weapon.category.launcher", "Launcher");
    }
    return tr("weapon.category.unknown", "Weapon");
}

float WorkbenchScreen::to_screen_x(float logical_x) const
{
    return presentation_rect_.x + logical_x * (presentation_rect_.w / static_cast<float>(kUiDesignWidth));
}

float WorkbenchScreen::to_screen_y(float logical_y) const
{
    return presentation_rect_.y + logical_y * (presentation_rect_.h / static_cast<float>(kUiDesignHeight));
}

SDL_FRect WorkbenchScreen::to_screen_rect(const SDL_FRect& logical_rect) const
{
    const float x_scale = presentation_rect_.w / static_cast<float>(kUiDesignWidth);
    const float y_scale = presentation_rect_.h / static_cast<float>(kUiDesignHeight);
    return SDL_FRect{
        presentation_rect_.x + logical_rect.x * x_scale,
        presentation_rect_.y + logical_rect.y * y_scale,
        logical_rect.w * x_scale,
        logical_rect.h * y_scale
    };
}

} // namespace zg
