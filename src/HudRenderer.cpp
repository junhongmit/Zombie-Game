#include "HudRenderer.h"

#include "Assets.h"
#include "Camera.h"
#include "Constants.h"
#include "InventoryState.h"
#include "LocalizationTable.h"
#include "MathUtil.h"
#include "Player.h"
#include "Presentation.h"
#include "Texture.h"
#include "Weapon.h"
#include "ui/Button.h"
#include "ui/BoxItem.h"
#include "ui/Canvas.h"
#include "ui/ControlStyle.h"
#include "ui/GameHudLayout.h"
#include "ui/Panel.h"
#include "ui/ProgressBar.h"
#include "ui/TextItem.h"

#include <cmath>
#include <cstdio>
#include <memory>
#include <string>

namespace zg {

namespace {

struct UiNormalizedRect {
    float x;
    float y;
    float w;
    float h;
};

SDL_FRect to_ui_rect(const UiNormalizedRect& rect)
{
    return SDL_FRect{
        rect.x * static_cast<float>(kUiDesignWidth),
        rect.y * static_cast<float>(kUiDesignHeight),
        rect.w * static_cast<float>(kUiDesignWidth),
        rect.h * static_cast<float>(kUiDesignHeight)
    };
}

void draw_pie(SDL_Renderer* renderer, float center_x, float center_y, float radius, float ratio)
{
    const float start_angle = 90.0f * 3.1415926535f / 180.0f;
    const float clamped_ratio = clamp_float(ratio, 0.0f, 1.0f);
    const float end_angle = start_angle - clamped_ratio * 2.0f * 3.1415926535f;
    for (float angle = start_angle; angle >= end_angle; angle -= 0.04f) {
        const float px = center_x + std::cos(angle) * radius;
        const float py = center_y - std::sin(angle) * radius;
        SDL_RenderLine(renderer, center_x, center_y, px, py);
    }
}

SDL_Color with_alpha(SDL_Color color, Uint8 alpha)
{
    color.a = alpha;
    return color;
}

void render_texture_fit(
    SDL_Renderer* renderer,
    const Texture& texture,
    const SDL_FRect& dst_rect,
    bool preserve_aspect = true)
{
    if (!texture.valid()) {
        return;
    }

    SDL_FRect dst = dst_rect;
    if (preserve_aspect && texture.width() > 0.0f && texture.height() > 0.0f) {
        const float scale = std::min(dst_rect.w / texture.width(), dst_rect.h / texture.height());
        const float w = std::round(texture.width() * scale);
        const float h = std::round(texture.height() * scale);
        dst.x += std::floor((dst_rect.w - w) * 0.5f);
        dst.y += std::floor((dst_rect.h - h) * 0.5f);
        dst.w = w;
        dst.h = h;
    }

    SDL_RenderTexture(renderer, texture.get(), nullptr, &dst);
}

SDL_FRect inset_rect(const SDL_FRect& rect, float horizontal, float vertical)
{
    return SDL_FRect{
        rect.x + horizontal,
        rect.y + vertical,
        std::max(0.0f, rect.w - horizontal * 2.0f),
        std::max(0.0f, rect.h - vertical * 2.0f)
    };
}

TTF_Font* load_hud_font(int point_size)
{
    static const char* kCandidates[] = {
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\lucon.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
        "C:\\Windows\\Fonts\\msyh.ttc",
    };

    for (const char* path : kCandidates) {
        TTF_Font* font = TTF_OpenFont(path, point_size);
        if (font != nullptr) {
            return font;
        }
    }
    return nullptr;
}

SDL_FRect title_banner_rect()
{
    return to_ui_rect(UiNormalizedRect{0.675f, 0.265f, 0.215f, 0.073f});
}

struct TitleButtonSpec {
    const char* label;
    TitleMenuAction action;
    bool enabled;
};

TitleButtonSpec title_button_spec(int index)
{
    static const TitleButtonSpec kButtons[] = {
        {"Start Game", TitleMenuAction::Start, true},
        {"Loadout", TitleMenuAction::Loadout, true},
        {"Black Market", TitleMenuAction::Market, true},
        {"Exit", TitleMenuAction::Exit, true},
    };
    return kButtons[index];
}

ui::Button title_button(int index)
{
    const TitleButtonSpec spec = title_button_spec(index);
    const float x = 0.685f * static_cast<float>(kUiDesignWidth);
    const float y = (0.365f + index * 0.088f) * static_cast<float>(kUiDesignHeight);
    const float w = 0.172f * static_cast<float>(kUiDesignWidth);
    const float h = 0.060f * static_cast<float>(kUiDesignHeight);
    return ui::Button(spec.label, x, y, w, h, spec.enabled);
}

ui::Button hud_mode_button(const char* label, float x, float y, float w, float h, bool active)
{
    (void)active;
    ui::Button button(label, x, y, w, h, true);
    button.set_enabled(true);
    return button;
}

} // namespace

HudRenderer::HudRenderer(SDL_Renderer* renderer)
    : renderer_(renderer)
{
    gameplay_layout_ = new GameHudLayout();
    gameplay_layout_hot_reload_.set_path("assets/ui/layouts/game_hud.json");
    gameplay_layout_->load("assets/ui/layouts/game_hud.json");
    gameplay_layout_hot_reload_.mark_loaded();
    gameplay_strings_ = new LocalizationTable();
    if (!gameplay_strings_->load("assets/localization/zh-cn/game_hud.loc")) {
        gameplay_strings_->load("assets/localization/us-en/game_hud.loc");
    }
    hud_font_point_size_ = static_cast<int>(kHudFontPointSize);
    hud_font_ = load_hud_font(hud_font_point_size_);
}

HudRenderer::~HudRenderer()
{
    if (hud_font_ != nullptr) {
        TTF_CloseFont(hud_font_);
        hud_font_ = nullptr;
    }
    delete gameplay_layout_;
    gameplay_layout_ = nullptr;
    delete gameplay_strings_;
    gameplay_strings_ = nullptr;
}

void HudRenderer::render_weapon_status(
    const Texture& bullet_icon,
    const Player& player,
    const WeaponDefinition* weapon_definition,
    const WeaponState& weapon,
    const Camera& camera,
    const SDL_FRect& presentation_rect)
{
    presentation_rect_ = presentation_rect;
    ensure_hud_font(presentation_scale(presentation_rect_));
    const float center_x = std::round(player.x - camera.render_x() + 10.0f);
    const float center_y = std::round(player.y - 10.0f + camera.render_y());
    const float ratio = weapon.indicator_ratio();

    if (weapon.reloading && weapon.reload_flash_on) {
        SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    }
    if (ratio > 0.0f) {
        draw_pie(renderer_, to_screen_x(center_x), to_screen_y(center_y), kHudIndicatorRadius * presentation_scale(presentation_rect_), ratio);
    }

    const SDL_FRect icon_dst = to_screen_rect(SDL_FRect{
        std::round(player.x - camera.render_x() + 24.0f),
        std::round(player.y - 20.0f + camera.render_y()),
        7.0f,
        11.0f
    });
    SDL_RenderTexture(renderer_, bullet_icon.get(), nullptr, &icon_dst);

    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%d/%d", weapon.ammo_in_mag, weapon.ammo_reserve);
    const float text_x = std::round(player.x - camera.render_x() + 34.0f);
    const float text_y = std::round(player.y - 19.0f + camera.render_y());
    if (weapon_definition != nullptr) {
        render_text(weapon_definition->name.c_str(), text_x, text_y - 12.0f);
    }
    render_text(buffer, text_x, text_y - 1.0f);
}

void HudRenderer::render_top_bar(const WeaponState& weapon, int wave, int alive_count, const SDL_FRect& presentation_rect)
{
    if (kEnableUiLayoutHotReload && gameplay_layout_hot_reload_.poll_changed()) {
        gameplay_layout_->load("assets/ui/layouts/game_hud.json");
    }
    presentation_rect_ = presentation_rect;
    ensure_hud_font(ui_presentation_scale(presentation_rect_));

    char wave_text[32];
    std::snprintf(wave_text, sizeof(wave_text), "Wave %d", wave);
    char alive_text[32];
    std::snprintf(alive_text, sizeof(alive_text), "Alive %d", alive_count);

    const SDL_FRect logical_panel = to_ui_rect(UiNormalizedRect{0.012f, 0.014f, 0.235f, 0.055f});
    ui::Canvas top_bar(logical_panel.x, logical_panel.y, logical_panel.w, logical_panel.h);

    std::unique_ptr<ui::BoxItem> background(new ui::BoxItem());
    background->set_rect(0.0f, 0.0f, logical_panel.w, logical_panel.h);
    background->set_fill_color(SDL_Color{8, 12, 18, 180});
    background->set_border_color(SDL_Color{78, 84, 96, 220});
    background->set_fill_enabled(true);
    background->set_border_enabled(true);
    top_bar.add_child(std::move(background));

    std::unique_ptr<ui::TextItem> wave_label(new ui::TextItem());
    wave_label->set_rect(18.0f, 8.0f, 120.0f, 20.0f);
    wave_label->set_font(hud_font_);
    wave_label->set_text(wave_text);
    wave_label->set_color(SDL_Color{255, 255, 255, 255});
    wave_label->set_fit_to_bounds(false);
    top_bar.add_child(std::move(wave_label));

    std::unique_ptr<ui::TextItem> alive_label(new ui::TextItem());
    alive_label->set_rect(18.0f, 34.0f, 120.0f, 20.0f);
    alive_label->set_font(hud_font_);
    alive_label->set_text(alive_text);
    alive_label->set_color(SDL_Color{255, 255, 255, 255});
    alive_label->set_fit_to_bounds(false);
    top_bar.add_child(std::move(alive_label));

    ui::RenderContext context;
    context.renderer = renderer_;
    context.presentation_rect = presentation_rect_;
    context.alpha = 255;
    top_bar.render(context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});

    render_slot_bar(weapon, 160.0f, 23.0f);
}

void HudRenderer::render_title_screen(
    const ControlStyle& button_skin,
    float ui_alpha,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    TitleMenuAction pressed_action,
    const SDL_FRect& presentation_rect)
{
    presentation_rect_ = presentation_rect;
    ensure_hud_font(ui_presentation_scale(presentation_rect_));
    const Uint8 alpha = static_cast<Uint8>(std::round(clamp_float(ui_alpha, 0.0f, 1.0f) * 255.0f));
    if (alpha == 0) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    if (button_skin.valid()) {
        const SDL_FRect banner_logical = title_banner_rect();
        const SDL_FRect banner = ui_to_screen_rect(banner_logical);
        button_skin.render(renderer_, banner, ControlVisualState::Normal, alpha);
        ui::Canvas banner_canvas(banner_logical.x, banner_logical.y, banner_logical.w, banner_logical.h);
        std::unique_ptr<ui::TextItem> title(new ui::TextItem());
        title->set_rect(0.0f, 12.0f, banner_logical.w, 26.0f);
        title->set_font(hud_font_);
        title->set_text("Zombie Game");
        title->set_color(SDL_Color{249, 235, 208, alpha});
        title->set_fit_to_bounds(false);
        title->set_horizontal_align(ui::HorizontalAlign::Center);
        title->set_vertical_align(ui::VerticalAlign::Middle);
        banner_canvas.add_child(std::move(title));
        std::unique_ptr<ui::TextItem> subtitle(new ui::TextItem());
        subtitle->set_rect(0.0f, 38.0f, banner_logical.w, 20.0f);
        subtitle->set_font(hud_font_);
        subtitle->set_text("Night Watch");
        subtitle->set_color(SDL_Color{118, 81, 48, alpha});
        subtitle->set_fit_to_bounds(false);
        subtitle->set_horizontal_align(ui::HorizontalAlign::Center);
        subtitle->set_vertical_align(ui::VerticalAlign::Middle);
        banner_canvas.add_child(std::move(subtitle));
        ui::RenderContext banner_context;
        banner_context.renderer = renderer_;
        banner_context.presentation_rect = presentation_rect_;
        banner_context.alpha = alpha;
        banner_canvas.render(banner_context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});

        for (int i = 0; i < 4; ++i) {
            const TitleButtonSpec spec = title_button_spec(i);
            const ui::Button button = title_button(i);
            button.render(
                renderer_,
                button_skin,
                hud_font_,
                presentation_rect_,
                mouse_x,
                mouse_y,
                mouse_in_view,
                mouse_down,
                pressed_action == spec.action,
                SDL_Color{241, 226, 205, alpha},
                SDL_Color{142, 136, 128, alpha},
                alpha);
        }
    } else {
        const SDL_FRect panel_logical = to_ui_rect(UiNormalizedRect{0.66f, 0.25f, 0.24f, 0.18f});
        ui::Canvas fallback_panel(panel_logical.x, panel_logical.y, panel_logical.w, panel_logical.h);
        std::unique_ptr<ui::BoxItem> background(new ui::BoxItem());
        background->set_rect(0.0f, 0.0f, panel_logical.w, panel_logical.h);
        background->set_fill_color(SDL_Color{8, 12, 18, static_cast<Uint8>(alpha * 0.78f)});
        background->set_border_color(SDL_Color{102, 112, 126, alpha});
        background->set_fill_enabled(true);
        background->set_border_enabled(true);
        fallback_panel.add_child(std::move(background));
        const struct LabelSpec {
            float x;
            float y;
            float w;
            float h;
            const char* text;
        } labels[] = {
            {24.0f, 18.0f, panel_logical.w - 48.0f, 28.0f, "Zombie Game"},
            {24.0f, 54.0f, panel_logical.w - 48.0f, 24.0f, "Survive the night"},
            {24.0f, 144.0f, panel_logical.w - 48.0f, 22.0f, "Click or Press Enter"},
        };
        for (const LabelSpec& spec : labels) {
            std::unique_ptr<ui::TextItem> item(new ui::TextItem());
            item->set_rect(spec.x, spec.y, spec.w, spec.h);
            item->set_font(hud_font_);
            item->set_text(spec.text);
            item->set_color(SDL_Color{255, 255, 255, alpha});
            item->set_fit_to_bounds(false);
            item->set_horizontal_align(ui::HorizontalAlign::Left);
            item->set_vertical_align(ui::VerticalAlign::Middle);
            fallback_panel.add_child(std::move(item));
        }
        ui::RenderContext fallback_context;
        fallback_context.renderer = renderer_;
        fallback_context.presentation_rect = presentation_rect_;
        fallback_context.alpha = alpha;
        fallback_panel.render(fallback_context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void HudRenderer::render_gameplay_hud(
    const Assets& assets,
    const Player& player,
    const InventoryState& inventory,
    const WeaponState& weapon,
    int wave,
    int alive_count,
    const SDL_FRect& presentation_rect)
{
    if (kEnableUiLayoutHotReload && gameplay_layout_hot_reload_.poll_changed()) {
        gameplay_layout_->load("assets/ui/layouts/game_hud.json");
    }
    presentation_rect_ = presentation_rect;
    ensure_hud_font(ui_presentation_scale(presentation_rect_));
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    const SDL_Color text_primary{234, 222, 205, 255};
    const SDL_Color text_secondary{174, 160, 140, 255};
    const SDL_Color accent_gold{214, 171, 82, 255};
    const SDL_Color accent_red{181, 68, 64, 255};
    const SDL_Color accent_blue{69, 120, 192, 255};
    const SDL_Color accent_yellow{186, 141, 49, 255};
    const SDL_Color accent_green{102, 168, 86, 255};

    const GameHudLayout& layout = *gameplay_layout_;
    const LocalizationTable& strings = *gameplay_strings_;
    const std::string& top_radio = strings.get("top.radio", "Radio");
    const std::string& top_radio_status = strings.get("top.radio_status", "Connected");
    const std::string& top_coins = strings.get("top.coins", "Coins");
    const std::string& top_gems = strings.get("top.gems", "Gems");
    const std::string& top_medkits = strings.get("top.medkits", "Medkits");
    const std::string& top_day = strings.get("top.day", "Day 5");
    const std::string& top_time = strings.get("top.time", "22:30  |  Night");
    const std::string& panel_objectives = strings.get("panel.objectives", "Current Objectives");
    const std::string& panel_warning = strings.get("panel.warning", "Warning");
    const std::string& objective_main = strings.get("objective.main", "Main Quest");
    const std::string& objective_radio = strings.get("objective.repair_radio", "- Repair the radio station");
    const std::string& objective_side = strings.get("objective.side", "Side Tasks");
    const std::string& objective_wood = strings.get("objective.gather_wood", "- Gather wooden boards");
    const std::string& objective_parts = strings.get("objective.gather_parts", "- Gather metal parts");
    const std::string& warning_tide = strings.get("warning.tide", "Zombie tide ETA");
    const std::string& survivor_you = strings.get("survivor.you", "You");
    const std::string& survivor_amy = strings.get("survivor.amy", "Amy");
    const std::string& survivor_tom = strings.get("survivor.tom", "Tom");
    const std::string& mode_combat = strings.get("mode.combat", "Combat");
    const std::string& mode_build = strings.get("mode.build", "Build");
    const std::string& mode_blueprint = strings.get("mode.blueprint", "Blueprint");
    const std::string& mode_support = strings.get("mode.support", "Support");

    const SDL_FRect root_rect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)};
    const auto make_label = [&](float x, float y, float w, float h, const char* text, SDL_Color color, ui::HorizontalAlign h_align = ui::HorizontalAlign::Left) -> std::unique_ptr<ui::TextItem> {
        std::unique_ptr<ui::TextItem> item(new ui::TextItem());
        item->set_rect(x, y, w, h);
        item->set_font(hud_font_);
        item->set_text(text);
        item->set_color(color);
        item->set_fit_to_bounds(false);
        item->set_horizontal_align(h_align);
        item->set_vertical_align(ui::VerticalAlign::Middle);
        return item;
    };
    const auto render_canvas = [&](const ui::Canvas& canvas) {
        ui::RenderContext context;
        context.renderer = renderer_;
        context.presentation_rect = presentation_rect_;
        context.alpha = 255;
        canvas.render(context, root_rect);
    };

    ui::Panel portrait_panel(layout.portrait_panel_rect().x, layout.portrait_panel_rect().y, layout.portrait_panel_rect().w, layout.portrait_panel_rect().h);
    portrait_panel.render(renderer_, assets.panel_skin, hud_font_, presentation_rect_, text_primary, 245);

    const SDL_FRect portrait_screen = portrait_panel.content_rect(assets.panel_skin, presentation_rect_);
    const SDL_FRect portrait_image = SDL_FRect{
        portrait_screen.x + 12.0f,
        portrait_screen.y + 8.0f,
        82.0f,
        portrait_screen.h - 16.0f
    };
    render_texture_fit(renderer_, assets.hero, portrait_image, true);
    ui::Canvas portrait_canvas(layout.portrait_panel_rect().x, layout.portrait_panel_rect().y, layout.portrait_panel_rect().w, layout.portrait_panel_rect().h);
    portrait_canvas.add_child(make_label(18.0f, 88.0f, 90.0f, 20.0f, "Lv. 2", text_primary));
    render_canvas(portrait_canvas);

    struct StatusBarSpec {
        const char* label;
        float value;
        float max_value;
        SDL_Color color;
        float y;
    };
    const StatusBarSpec bars[] = {
        {"HP", player.hp, 100.0f, accent_red, 34.0f},
        {"STM", 85.0f, 100.0f, accent_blue, 64.0f},
        {"MOR", 65.0f, 100.0f, accent_yellow, 94.0f},
    };
    ui::Canvas portrait_status_canvas(layout.portrait_panel_rect().x, layout.portrait_panel_rect().y, layout.portrait_panel_rect().w, layout.portrait_panel_rect().h);
    for (const StatusBarSpec& bar : bars) {
        portrait_status_canvas.add_child(make_label(112.0f, bar.y - 2.0f, 42.0f, 18.0f, bar.label, text_secondary));
        ui::ProgressBar meter(160.0f, bar.y, 170.0f, 16.0f, ui::ProgressBarOrientation::Horizontal);
        meter.set_progress(bar.max_value > 0.0f ? bar.value / bar.max_value : 0.0f);
        meter.render(
            renderer_,
            assets.progressbar_horizontal_track_style,
            assets.progressbar_horizontal_fill_style,
            presentation_rect_,
            240);
        char value_text[32];
        std::snprintf(value_text, sizeof(value_text), "%.0f/%.0f", bar.value, bar.max_value);
        portrait_status_canvas.add_child(
            make_label(
                160.0f,
                bar.y - 2.0f,
                170.0f,
                18.0f,
                value_text,
                with_alpha(bar.color, 255),
                ui::HorizontalAlign::Right));
    }
    render_canvas(portrait_status_canvas);

    const struct TopPanelSpec {
        const char* title;
        const char* value;
        SDL_FRect rect;
    } top_panels[] = {
        {top_radio.c_str(), top_radio_status.c_str(), layout.top_panel_rect(0)},
        {top_coins.c_str(), "", layout.top_panel_rect(1)},
        {top_gems.c_str(), "", layout.top_panel_rect(2)},
        {top_medkits.c_str(), "", layout.top_panel_rect(3)},
    };
    for (int i = 0; i < 4; ++i) {
        const TopPanelSpec& panel_spec = top_panels[i];
        ui::Panel panel(panel_spec.rect.x, panel_spec.rect.y, panel_spec.rect.w, panel_spec.rect.h);
        panel.render(renderer_, assets.panel_skin, hud_font_, presentation_rect_, text_primary, 238);
        ui::Canvas text_canvas(panel_spec.rect.x, panel_spec.rect.y, panel_spec.rect.w, panel_spec.rect.h);
        text_canvas.add_child(make_label(22.0f, 10.0f, panel_spec.rect.w - 44.0f, 18.0f, panel_spec.title, text_secondary));
        if (i == 1) {
            char value[32];
            std::snprintf(value, sizeof(value), "%d", inventory.resources().coins);
            text_canvas.add_child(make_label(22.0f, 28.0f, panel_spec.rect.w - 44.0f, 20.0f, value, text_primary));
        } else if (i == 2) {
            char value[32];
            std::snprintf(value, sizeof(value), "%d", inventory.resources().gems);
            text_canvas.add_child(make_label(22.0f, 28.0f, panel_spec.rect.w - 44.0f, 20.0f, value, text_primary));
        } else if (i == 3) {
            char value[32];
            std::snprintf(value, sizeof(value), "%d/%d", inventory.resources().medicine, inventory.resources().medicine);
            text_canvas.add_child(make_label(22.0f, 28.0f, panel_spec.rect.w - 44.0f, 20.0f, value, text_primary));
        } else {
            text_canvas.add_child(make_label(22.0f, 28.0f, panel_spec.rect.w - 44.0f, 20.0f, panel_spec.value, accent_green));
        }
        render_canvas(text_canvas);
    }

    const SDL_FRect time_rect = layout.time_panel_rect();
    ui::Panel time_panel(time_rect.x, time_rect.y, time_rect.w, time_rect.h);
    time_panel.render(renderer_, assets.panel_skin, hud_font_, presentation_rect_, text_primary, 238);
    ui::Canvas time_canvas(time_rect.x, time_rect.y, time_rect.w, time_rect.h);
    time_canvas.add_child(make_label(22.0f, 10.0f, time_rect.w - 44.0f, 18.0f, top_day.c_str(), text_secondary));
    time_canvas.add_child(make_label(22.0f, 28.0f, time_rect.w - 44.0f, 20.0f, top_time.c_str(), text_primary));
    render_canvas(time_canvas);

    const SDL_FRect objectives_rect = layout.objectives_panel_rect();
    ui::Panel objectives_panel(objectives_rect.x, objectives_rect.y, objectives_rect.w, objectives_rect.h);
    objectives_panel.set_title(panel_objectives.c_str()).render(renderer_, assets.panel_skin, hud_font_, presentation_rect_, text_primary, 240);
    ui::Canvas objectives_canvas(objectives_rect.x, objectives_rect.y, objectives_rect.w, objectives_rect.h);
    objectives_canvas.add_child(make_label(38.0f, 48.0f, objectives_rect.w - 76.0f, 22.0f, objective_main.c_str(), accent_gold));
    objectives_canvas.add_child(make_label(42.0f, 86.0f, objectives_rect.w - 120.0f, 22.0f, objective_radio.c_str(), text_primary));
    objectives_canvas.add_child(make_label(objectives_rect.w - 78.0f, 86.0f, 56.0f, 22.0f, "0 / 1", text_secondary, ui::HorizontalAlign::Right));
    objectives_canvas.add_child(make_label(38.0f, 136.0f, objectives_rect.w - 76.0f, 22.0f, objective_side.c_str(), accent_gold));
    objectives_canvas.add_child(make_label(42.0f, 174.0f, objectives_rect.w - 120.0f, 22.0f, objective_wood.c_str(), text_primary));
    char wood_text[16];
    char metal_text[16];
    std::snprintf(wood_text, sizeof(wood_text), "%d / 10", std::min(inventory.resources().wood, 10));
    std::snprintf(metal_text, sizeof(metal_text), "%d / 5", std::min(inventory.resources().parts, 5));
    objectives_canvas.add_child(make_label(objectives_rect.w - 78.0f, 174.0f, 56.0f, 22.0f, wood_text, text_secondary, ui::HorizontalAlign::Right));
    objectives_canvas.add_child(make_label(42.0f, 208.0f, objectives_rect.w - 120.0f, 22.0f, objective_parts.c_str(), text_primary));
    objectives_canvas.add_child(make_label(objectives_rect.w - 78.0f, 208.0f, 56.0f, 22.0f, metal_text, text_secondary, ui::HorizontalAlign::Right));
    render_canvas(objectives_canvas);

    const SDL_FRect warning_rect = layout.warning_panel_rect();
    ui::Panel warning_panel(warning_rect.x, warning_rect.y, warning_rect.w, warning_rect.h);
    warning_panel.set_title(panel_warning.c_str()).render(renderer_, assets.panel_skin, hud_font_, presentation_rect_, text_primary, 240);
    ui::Canvas warning_canvas(warning_rect.x, warning_rect.y, warning_rect.w, warning_rect.h);
    warning_canvas.add_child(make_label(44.0f, 52.0f, warning_rect.w - 88.0f, 22.0f, warning_tide.c_str(), text_primary));
    char timer_text[32];
    std::snprintf(timer_text, sizeof(timer_text), "00:%02d", 50 + (wave % 10));
    warning_canvas.add_child(make_label(84.0f, 98.0f, warning_rect.w - 168.0f, 30.0f, timer_text, accent_red, ui::HorizontalAlign::Center));
    render_canvas(warning_canvas);

    const char* modes[] = {mode_combat.c_str(), mode_build.c_str(), mode_blueprint.c_str(), mode_support.c_str()};
    for (int i = 0; i < 4; ++i) {
        const SDL_FRect rect = layout.mode_button_rect(i);
        const bool active = inventory.current_mode_index() == i;
        ui::Button button = hud_mode_button(modes[i], rect.x, rect.y, rect.w, rect.h, active);
        button.render(
            renderer_,
            assets.title_button_skin,
            hud_font_,
            presentation_rect_,
            -1000.0f,
            -1000.0f,
            false,
            false,
            active,
            text_primary,
            text_secondary,
            240);
    }

    const InventorySlot* tool_slots = inventory.visible_tool_slots();
    for (int i = 0; i < 9; ++i) {
        const SDL_FRect logical_rect = layout.tool_slot_rect(i);
        const SDL_FRect screen_rect = ui_to_screen_rect(logical_rect);
        const InventorySlot& slot = tool_slots[i];
        const InventoryItemDefinition* item = inventory.item_definition(slot.item_index);
        const bool highlighted = inventory.current_mode() == ToolMode::Combat && i == 0;
        const ControlVisualState state = slot.locked
            ? ControlVisualState::Disabled
            : (highlighted ? ControlVisualState::Pressed : ControlVisualState::Normal);
        assets.weapon_card_style.render(renderer_, screen_rect, state, 240);
        if (item != nullptr && item->icon.valid()) {
            render_texture_fit(renderer_, item->icon, inset_rect(screen_rect, 18.0f, 18.0f), true);
        } else if (inventory.current_mode() == ToolMode::Combat && i < 2 && i < weapon.slot_count()) {
            const WeaponSlot* weapon_slots = weapon.slots();
            if (weapon_slots[i].definition != nullptr) {
                const Texture* icon = weapon_slots[i].definition->icon_texture.valid()
                    ? &weapon_slots[i].definition->icon_texture
                    : (weapon_slots[i].definition->preview_texture.valid()
                        ? &weapon_slots[i].definition->preview_texture
                        : nullptr);
                if (icon != nullptr) {
                    render_texture_fit(renderer_, *icon, inset_rect(screen_rect, 18.0f, 18.0f), true);
                }
            }
        }
        const char* title = slot.locked ? "Locked" : (item != nullptr ? item->name.c_str() : "");
        char meta[32];
        meta[0] = '\0';
        if (!slot.locked && item != nullptr) {
            if (inventory.current_mode() == ToolMode::Combat && i < 2 && i < weapon.slot_count()) {
                const WeaponSlot* weapon_slots = weapon.slots();
                std::snprintf(meta, sizeof(meta), "%d/%d", weapon_slots[i].ammo_in_mag, weapon_slots[i].ammo_reserve);
            } else if (slot.quantity > 0) {
                std::snprintf(meta, sizeof(meta), "x%d", slot.quantity);
            }
        }
        ui::Canvas slot_canvas(logical_rect.x, logical_rect.y, logical_rect.w, logical_rect.h);
        slot_canvas.add_child(
            make_label(
                10.0f,
                logical_rect.h - 44.0f,
                logical_rect.w - 20.0f,
                18.0f,
                title,
                slot.locked ? text_secondary : text_primary,
                ui::HorizontalAlign::Center));
        slot_canvas.add_child(
            make_label(
                10.0f,
                logical_rect.h - 22.0f,
                logical_rect.w - 20.0f,
                16.0f,
                meta,
                text_secondary,
                ui::HorizontalAlign::Center));
        render_canvas(slot_canvas);
    }

    const struct SurvivorSpec {
        const char* name;
        float hp_ratio;
        SDL_FRect rect;
    } survivors[] = {
        {survivor_you.c_str(), 0.74f, layout.survivor_panel_rect(0)},
        {survivor_amy.c_str(), 0.62f, layout.survivor_panel_rect(1)},
        {survivor_tom.c_str(), 0.41f, layout.survivor_panel_rect(2)},
    };
    for (const SurvivorSpec& survivor : survivors) {
        ui::Panel panel(survivor.rect.x, survivor.rect.y, survivor.rect.w, survivor.rect.h);
        panel.render(renderer_, assets.panel_skin, hud_font_, presentation_rect_, text_primary, 236);
        const SDL_FRect portrait_rect{
            survivor.rect.x + 18.0f,
            survivor.rect.y + 18.0f,
            survivor.rect.w - 36.0f,
            90.0f
        };
        render_texture_fit(renderer_, assets.hero, ui_to_screen_rect(portrait_rect), true);
        ui::Canvas survivor_canvas(survivor.rect.x, survivor.rect.y, survivor.rect.w, survivor.rect.h);
        survivor_canvas.add_child(make_label(0.0f, 82.0f, survivor.rect.w, 22.0f, survivor.name, text_primary, ui::HorizontalAlign::Center));
        render_canvas(survivor_canvas);
        ui::ProgressBar hp_bar(
            survivor.rect.x + 18.0f,
            survivor.rect.y + survivor.rect.h - 28.0f,
            survivor.rect.w - 36.0f,
            12.0f,
            ui::ProgressBarOrientation::Horizontal);
        hp_bar.set_progress(survivor.hp_ratio);
        hp_bar.render(
            renderer_,
            assets.progressbar_horizontal_track_style,
            assets.progressbar_horizontal_fill_style,
            presentation_rect_,
            235);
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

TitleMenuAction HudRenderer::hit_test_title_menu(float mouse_x, float mouse_y, bool mouse_in_view) const
{
    if (!mouse_in_view) {
        return TitleMenuAction::None;
    }

    for (int i = 0; i < 4; ++i) {
        const TitleButtonSpec spec = title_button_spec(i);
        const ui::Button button = title_button(i);
        if (button.contains(mouse_x, mouse_y, mouse_in_view)) {
            return spec.action;
        }
    }
    return TitleMenuAction::None;
}

void HudRenderer::render_text(const char* text, float x, float y)
{
    render_text_colored(text, x, y, SDL_Color{255, 255, 255, 255});
}

void HudRenderer::ensure_hud_font(float ui_scale)
{
    const int desired_point_size = std::max(
        static_cast<int>(kHudFontPointSize),
        static_cast<int>(std::round(kHudFontPointSize * ui_scale)));
    if (hud_font_ != nullptr && desired_point_size == hud_font_point_size_) {
        return;
    }

    if (hud_font_ != nullptr) {
        TTF_CloseFont(hud_font_);
        hud_font_ = nullptr;
    }
    hud_font_point_size_ = desired_point_size;
    hud_font_ = load_hud_font(hud_font_point_size_);
}

void HudRenderer::render_text_colored(const char* text, float x, float y, SDL_Color color)
{
    if (hud_font_ == nullptr || text == nullptr || text[0] == '\0') {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(hud_font_, text, 0, color);
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

void HudRenderer::render_slot_bar(const WeaponState& weapon, float x, float y)
{
    const float slot_width = 44.0f;
    const float slot_height = 28.0f;
    const float gap = 10.0f;
    const SDL_FRect root_rect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)};

    for (int i = 0; i < weapon.slot_count(); ++i) {
        const float slot_x = x + i * (slot_width + gap);
        const SDL_FRect rect = ui_to_screen_rect(SDL_FRect{slot_x, y, slot_width, slot_height});
        const bool active = i == weapon.active_slot_index();
        if (active) {
            SDL_SetRenderDrawColor(renderer_, 236, 216, 88, 255);
            SDL_RenderFillRect(renderer_, &rect);
            SDL_SetRenderDrawColor(renderer_, 32, 24, 8, 255);
        } else {
            SDL_SetRenderDrawColor(renderer_, 20, 26, 34, 220);
            SDL_RenderFillRect(renderer_, &rect);
            SDL_SetRenderDrawColor(renderer_, 136, 142, 150, 255);
            SDL_RenderRect(renderer_, &rect);
        }

        const WeaponSlot* slots = weapon.slots();
        const WeaponDefinition* definition = slots[i].definition;
        if (definition != nullptr) {
            const Texture* icon = definition->icon_texture.valid()
                ? &definition->icon_texture
                : (definition->preview_texture.valid() ? &definition->preview_texture : nullptr);
            if (icon != nullptr) {
                render_texture_fit(renderer_, *icon, SDL_FRect{rect.x + 4.0f, rect.y + 4.0f, rect.w - 8.0f, rect.h - 8.0f}, true);
            }
        }

        char label[4];
        std::snprintf(label, sizeof(label), "%d", i + 1);
        ui::Canvas slot_label(slot_x, y - 4.0f, slot_width, 14.0f);
        std::unique_ptr<ui::TextItem> label_item(new ui::TextItem());
        label_item->set_rect(0.0f, 0.0f, slot_width, 14.0f);
        label_item->set_font(hud_font_);
        label_item->set_text(label);
        label_item->set_color(SDL_Color{220, 224, 232, 255});
        label_item->set_fit_to_bounds(false);
        label_item->set_horizontal_align(ui::HorizontalAlign::Left);
        label_item->set_vertical_align(ui::VerticalAlign::Top);
        slot_label.add_child(std::move(label_item));
        ui::RenderContext context;
        context.renderer = renderer_;
        context.presentation_rect = presentation_rect_;
        context.alpha = 255;
        slot_label.render(context, root_rect);
    }
}

float HudRenderer::to_screen_x(float logical_x) const
{
    return presentation_rect_.x + logical_x * presentation_scale(presentation_rect_);
}

float HudRenderer::to_screen_y(float logical_y) const
{
    return presentation_rect_.y + logical_y * presentation_scale(presentation_rect_);
}

SDL_FRect HudRenderer::to_screen_rect(const SDL_FRect& logical_rect) const
{
    return logical_to_present_rect(logical_rect, presentation_rect_);
}

float HudRenderer::ui_to_screen_x(float logical_x) const
{
    return presentation_rect_.x + logical_x * ui_presentation_scale_x(presentation_rect_);
}

float HudRenderer::ui_to_screen_y(float logical_y) const
{
    return presentation_rect_.y + logical_y * ui_presentation_scale_y(presentation_rect_);
}

SDL_FRect HudRenderer::ui_to_screen_rect(const SDL_FRect& logical_rect) const
{
    return ui_logical_to_present_rect(logical_rect, presentation_rect_);
}

} // namespace zg
