#include "WorkbenchScreen.h"

#include "../Assets.h"
#include "../Constants.h"
#include "../Presentation.h"
#include "../Texture.h"
#include "../Weapon.h"
#include "Card.h"
#include "Panel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

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
    layout_.load("assets/ui/layouts/workbench.json");
    strings_.load("assets/localization/us-en/workbench.loc");
    font_point_size_ = static_cast<int>(kHudFontPointSize);
    font_ = load_ui_font(font_point_size_);
}

WorkbenchScreen::~WorkbenchScreen()
{
    if (font_ != nullptr) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
}

void WorkbenchScreen::render(
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

    const SDL_Color panel_text{241, 226, 205, 255};
    const SDL_FRect stats_panel = layout_.stats_panel_rect();
    const SDL_FRect effects_panel = layout_.effects_panel_rect();
    const SDL_FRect upgrade_panel = layout_.upgrade_panel_rect();
    make_panel(layout_.weapon_list_panel_rect().x, layout_.weapon_list_panel_rect().y, layout_.weapon_list_panel_rect().w, layout_.weapon_list_panel_rect().h, "").render(renderer_, assets.panel_skin, font_, presentation_rect_, panel_text);
    make_panel(layout_.attachments_panel_rect().x, layout_.attachments_panel_rect().y, layout_.attachments_panel_rect().w, layout_.attachments_panel_rect().h, "").render(renderer_, assets.panel_skin, font_, presentation_rect_, panel_text);
    make_panel(stats_panel.x, stats_panel.y, stats_panel.w, stats_panel.h, tr("panel.weapon_stats", "Stats").c_str()).render(renderer_, assets.panel_skin, font_, presentation_rect_, panel_text);
    make_panel(effects_panel.x, effects_panel.y, effects_panel.w, effects_panel.h, tr("panel.current_effects", "Effects").c_str()).render(renderer_, assets.panel_skin, font_, presentation_rect_, panel_text);
    make_panel(upgrade_panel.x, upgrade_panel.y, upgrade_panel.w, upgrade_panel.h, tr("panel.upgrade_requirements", "Upgrade Cost").c_str()).render(renderer_, assets.panel_skin, font_, presentation_rect_, panel_text);
    render_text_centered(strings_.resolve_token(layout_.weapon_list_title_text()).c_str(), layout_.weapon_list_title_rect(), panel_text, -1.0f);
    render_text_centered(strings_.resolve_token(layout_.attachments_title_text()).c_str(), layout_.attachments_title_rect(), panel_text, -1.0f);

    render_weapon_list(assets, weapon_state, dt, wheel_y, mouse_x, mouse_y, mouse_in_view, mouse_down, mouse_pressed, mouse_released);
    render_preview_nameplate(assets, weapon_state.current_definition(), layout_.preview_nameplate_rect());
    render_weapon_preview(assets, weapon_state.current_definition(), layout_.preview_panel_rect());
    render_weapon_stats(assets, weapon_state.current_definition(), stats_panel);
    render_attachment_popup(assets, dt, wheel_x, mouse_x, mouse_y, mouse_in_view, mouse_down, mouse_pressed, mouse_released);
    render_attachment_strip(assets, dt, wheel_x, mouse_x, mouse_y, mouse_in_view, mouse_down, mouse_pressed, mouse_released);

    render_text(tr("effects.accuracy_up", "+ Accuracy 10").c_str(), effects_panel.x + 22.0f, effects_panel.y + 64.0f, SDL_Color{116, 188, 84, 255});
    render_text(tr("effects.reload_up", "+ Reload Speed 15%").c_str(), effects_panel.x + 22.0f, effects_panel.y + 94.0f, SDL_Color{116, 188, 84, 255});
    render_text(tr("upgrade.metal_parts", "Metal Parts 78 / 15").c_str(), upgrade_panel.x + 22.0f, upgrade_panel.y + 54.0f, SDL_Color{241, 226, 205, 255});
    render_text(tr("upgrade.polymer_parts", "Polymer Parts 23 / 10").c_str(), upgrade_panel.x + 22.0f, upgrade_panel.y + 88.0f, SDL_Color{241, 226, 205, 255});
    render_text(tr("upgrade.fabric", "Fabric 12 / 5").c_str(), upgrade_panel.x + 22.0f, upgrade_panel.y + 122.0f, SDL_Color{241, 226, 205, 255});
    render_text(tr("upgrade.tool_level", "Tool Bench Lv. 2").c_str(), upgrade_panel.x + 22.0f, upgrade_panel.y + 156.0f, SDL_Color{241, 226, 205, 255});
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
    font_point_size_ = desired_point_size;
    font_ = load_ui_font(font_point_size_);
}

void WorkbenchScreen::render_text(const char* text, float x, float y, SDL_Color color) const
{
    if (font_ == nullptr || text == nullptr || text[0] == '\0') {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font_, text, 0, color);
    if (surface == nullptr) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (texture == nullptr) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
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

        char ammo[32];
        std::snprintf(ammo, sizeof(ammo), "%d/%d", slot.ammo_in_mag, slot.ammo_reserve);
        const Texture* card_icon = slot.definition->preview_texture.valid()
            ? &slot.definition->preview_texture
            : &slot.definition->texture;
        ui::Card card(row_rect.x, row_rect.y, row_rect.w, row_rect.h);
        card
            .set_selected(i == weapon_state.active_slot_index())
            .set_title(slot.definition->name.c_str())
            .set_subtitle(slot.definition->full_auto
                ? tr("weapon.fire_mode.auto", "Auto").c_str()
                : tr("weapon.fire_mode.semi", "Semi").c_str())
            .set_meta(ammo)
            .set_icon(card_icon);
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
            armed_weapon_index_ == i,
            SDL_Color{241, 226, 205, 255},
            SDL_Color{201, 182, 150, 255},
            SDL_Color{168, 150, 124, 255});
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

    for (int i = 0; i < 5; ++i) {
        const float row_y = logical_rect.y + 56.0f + i * 74.0f;
        render_text(rows[i].label, logical_rect.x + 28.0f, row_y, SDL_Color{241, 226, 205, 255});
        render_text(rows[i].value, logical_rect.x + logical_rect.w - 82.0f, row_y, SDL_Color{241, 226, 205, 255});
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
    const SDL_FRect rect = to_screen_rect(logical_rect);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 8, 10, 12, 168);
    SDL_RenderFillRect(renderer_, &rect);
    SDL_SetRenderDrawColor(renderer_, 88, 72, 42, 220);
    SDL_RenderRect(renderer_, &rect);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);

    const float x = logical_rect.x;
    const float y = logical_rect.y;
    render_text(tr("top.workbench", "Workbench").c_str(), x + 16.0f, y + 18.0f, SDL_Color{241, 226, 205, 255});
    render_text(tr("top.level", "Lv. 2").c_str(), x + 136.0f, y + 18.0f, SDL_Color{211, 188, 142, 255});
    render_text(tr("top.metal", "Metal 350").c_str(), x + 268.0f, y + 18.0f, SDL_Color{241, 226, 205, 255});
    render_text(tr("top.wood", "Wood 126").c_str(), x + 470.0f, y + 18.0f, SDL_Color{241, 226, 205, 255});
    render_text(tr("top.parts", "Parts 78").c_str(), x + 676.0f, y + 18.0f, SDL_Color{241, 226, 205, 255});
    render_text(tr("top.fabric", "Fabric 23").c_str(), x + 874.0f, y + 18.0f, SDL_Color{241, 226, 205, 255});
    render_text(tr("top.gems", "Gems 0").c_str(), x + 1074.0f, y + 18.0f, SDL_Color{241, 226, 205, 255});
    render_text(tr("top.back", "Esc Back").c_str(), x + logical_rect.w - 230.0f, y + 18.0f, SDL_Color{211, 188, 142, 255});
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
                mouse_y >= card_rect.y && mouse_y <= card_rect.y + card_rect.h;
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
            mouse_y >= card_rect.y && mouse_y <= card_rect.y + card_rect.h;
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
