#include "MarketScreen.h"

#include "../Assets.h"
#include "../Constants.h"
#include "../InventoryState.h"
#include "../Presentation.h"
#include "../Texture.h"
#include "../Weapon.h"
#include "Button.h"
#include "Canvas.h"
#include "Panel.h"
#include "ProgressBar.h"
#include "TextItem.h"

#include <algorithm>
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

void render_texture_fit(SDL_Renderer* renderer, const Texture& texture, const SDL_FRect& dst_rect, bool preserve_aspect = true)
{
    if (!texture.valid() || dst_rect.w <= 0.0f || dst_rect.h <= 0.0f) {
        return;
    }

    SDL_FRect dst = dst_rect;
    if (preserve_aspect) {
        const float scale = std::min(dst_rect.w / std::max(1.0f, texture.width()), dst_rect.h / std::max(1.0f, texture.height()));
        const float w = std::round(texture.width() * scale);
        const float h = std::round(texture.height() * scale);
        dst.x += std::floor((dst_rect.w - w) * 0.5f);
        dst.y += std::floor((dst_rect.h - h) * 0.5f);
        dst.w = w;
        dst.h = h;
    }

    SDL_SetTextureScaleMode(texture.get(), SDL_SCALEMODE_LINEAR);
    SDL_RenderTexture(renderer, texture.get(), nullptr, &dst);
}

const char* category_key(int index)
{
    static const char* kKeys[] = {
        "category.all",
        "category.pistol",
        "category.shotgun",
        "category.rifle",
        "category.sniper",
        "category.other",
    };
    return kKeys[index < 0 ? 0 : index % 6];
}

const ControlStyle& resolve_skin(const Assets& assets, const std::string& skin_name, const ControlStyle& fallback)
{
    if (const ControlStyle* skin = assets.find_ui_skin(skin_name)) {
        return *skin;
    }
    if (skin_name == "panel_square_bronze") {
        return assets.panel_skin;
    }
    if (skin_name == "button_square_bronze") {
        return assets.title_button_skin;
    }
    if (skin_name == "card1_weapon_row") {
        return assets.weapon_card_style;
    }
    return fallback;
}

} // namespace

MarketScreen::MarketScreen(SDL_Renderer* renderer)
    : renderer_(renderer),
      category_tabs_(0.0f, 0.0f, 1.0f, 1.0f, ui::TabBarOrientation::Vertical)
{
    layout_hot_reload_.set_path("assets/ui/layouts/market.json");
    layout_.load("assets/ui/layouts/market.json");
    layout_hot_reload_.mark_loaded();
    if (!strings_.load("assets/localization/zh-cn/market.loc")) {
        strings_.load("assets/localization/us-en/market.loc");
    }
    font_point_size_ = static_cast<int>(kHudFontPointSize);
    font_ = load_ui_font(font_point_size_);
    title_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 1.6f)));
    body_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 1.2f)));
    small_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 0.95f)));
}

MarketScreen::~MarketScreen()
{
    if (small_font_ != nullptr) {
        TTF_CloseFont(small_font_);
    }
    if (body_font_ != nullptr) {
        TTF_CloseFont(body_font_);
    }
    if (title_font_ != nullptr) {
        TTF_CloseFont(title_font_);
    }
    if (font_ != nullptr) {
        TTF_CloseFont(font_);
    }
}

bool MarketScreen::render(
    const Assets& assets,
    const InventoryState& inventory,
    const WeaponCatalog& weapon_catalog,
    const SDL_FRect& presentation_rect,
    float dt,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool mouse_pressed,
    bool mouse_released)
{
    (void)dt;
    if (kEnableUiLayoutHotReload && layout_hot_reload_.poll_changed()) {
        layout_.load("assets/ui/layouts/market.json");
    }
    presentation_rect_ = presentation_rect;
    ensure_font(ui_scale_for_rect(presentation_rect_));
    rebuild_catalog(weapon_catalog);

    const SDL_FRect full_screen_rect{0.0f, 0.0f, static_cast<float>(kInternalRenderWidth), static_cast<float>(kInternalRenderHeight)};
    SDL_SetTextureScaleMode(assets.market.get(), SDL_SCALEMODE_LINEAR);
    SDL_RenderTexture(renderer_, assets.market.get(), nullptr, &full_screen_rect);
    render_resource_strip(assets, inventory);
    render_left_panels(assets, inventory);

    const SDL_FRect notebook_rect = to_screen_rect(layout_.notebook_rect());
    render_texture_fit(renderer_, assets.notebook, notebook_rect, true);
    render_notebook(assets, weapon_catalog, mouse_x, mouse_y, mouse_in_view, mouse_down, mouse_pressed, mouse_released);
    render_category_buttons(assets, mouse_x, mouse_y, mouse_in_view, mouse_down, mouse_pressed, mouse_released);

    const SDL_FRect close_rect = layout_.close_button_rect();
    ui::Button close_button(tr("button.close_link", "Close Link").c_str(), close_rect.x, close_rect.y, close_rect.w, close_rect.h, true);
    const bool close_requested = close_button.process_pointer(mouse_x, mouse_y, mouse_in_view, mouse_pressed, mouse_released, close_armed_);
    close_button.render(
        renderer_,
        assets.title_button_skin,
        body_font_ != nullptr ? body_font_ : font_,
        presentation_rect_,
        mouse_x,
        mouse_y,
        mouse_in_view,
        mouse_down,
        close_armed_,
        SDL_Color{234, 222, 205, 255},
        SDL_Color{142, 136, 128, 255},
        240);
    return close_requested;
}

void MarketScreen::ensure_font(float ui_scale)
{
    const int desired = std::max(static_cast<int>(kHudFontPointSize), static_cast<int>(std::round(kHudFontPointSize * ui_scale)));
    if (font_ != nullptr && desired == font_point_size_) {
        return;
    }

    if (small_font_ != nullptr) {
        TTF_CloseFont(small_font_);
        small_font_ = nullptr;
    }
    if (body_font_ != nullptr) {
        TTF_CloseFont(body_font_);
        body_font_ = nullptr;
    }
    if (title_font_ != nullptr) {
        TTF_CloseFont(title_font_);
        title_font_ = nullptr;
    }
    if (font_ != nullptr) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    font_point_size_ = desired;
    font_ = load_ui_font(font_point_size_);
    title_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 1.6f)));
    body_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 1.2f)));
    small_font_ = load_ui_font(static_cast<int>(std::round(font_point_size_ * 0.95f)));
}

void MarketScreen::rebuild_catalog(const WeaponCatalog& weapon_catalog)
{
    filtered_entries_.clear();
    for (int i = 0; i < weapon_catalog.count(); ++i) {
        const WeaponDefinition* definition = weapon_catalog.definition(i);
        if (definition == nullptr || !matches_category(*definition, active_category_)) {
            continue;
        }
        CatalogEntry entry;
        entry.definition = definition;
        entry.price = std::max(120, definition->damage * 12 + definition->magazine_size * 4 + definition->speed_rpm / 3);
        filtered_entries_.push_back(entry);
    }
    const int items_per_page = std::max(1, layout_.catalog_item_count());
    const int max_page = std::max(0, static_cast<int>((filtered_entries_.size() + items_per_page - 1) / items_per_page) - 1);
    current_page_ = std::max(0, std::min(current_page_, max_page));
}

bool MarketScreen::matches_category(const WeaponDefinition& definition, MarketCategory category) const
{
    if (category == MarketCategory::All) {
        return true;
    }
    if (category == MarketCategory::Pistol) {
        return definition.ui_card_template == "pistol";
    }
    if (category == MarketCategory::Shotgun) {
        return definition.ui_card_template == "shotgun";
    }
    if (category == MarketCategory::Rifle) {
        return definition.ui_card_template == "rifle";
    }
    if (category == MarketCategory::Sniper) {
        return definition.ui_card_template == "sniper";
    }
    return definition.ui_card_template == "smg" || definition.type == WeaponType::Grenade;
}

void MarketScreen::render_text(TTF_Font* font, const char* text, float x, float y, SDL_Color color) const
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
    const SDL_FRect dst{to_screen_x(x), to_screen_y(y), static_cast<float>(surface->w), static_cast<float>(surface->h)};
    SDL_RenderTexture(renderer_, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void MarketScreen::render_text_centered(TTF_Font* font, const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset) const
{
    if (font == nullptr || text == nullptr || text[0] == '\0') {
        return;
    }
    int text_width = 0;
    int text_height = 0;
    if (!TTF_GetStringSize(font, text, 0, &text_width, &text_height)) {
        return;
    }
    const float x_scale = presentation_rect_.w / static_cast<float>(kUiDesignWidth);
    const float y_scale = presentation_rect_.h / static_cast<float>(kUiDesignHeight);
    const float logical_text_width = static_cast<float>(text_width) / x_scale;
    const float logical_text_height = static_cast<float>(text_height) / y_scale;
    render_text(font, text, rect.x + std::floor((rect.w - logical_text_width) * 0.5f), rect.y + std::floor((rect.h - logical_text_height) * 0.5f) + y_offset, color);
}

void MarketScreen::render_resource_strip(const Assets& assets, const InventoryState& inventory) const
{
    const SDL_Color text_primary{234, 222, 205, 255};
    const SDL_FRect strip = layout_.resource_strip_rect();
    const SDL_FRect entries[] = {
        SDL_FRect{strip.x + 0.0f, strip.y, strip.w * 0.25f, strip.h},
        SDL_FRect{strip.x + strip.w * 0.25f, strip.y, strip.w * 0.25f, strip.h},
        SDL_FRect{strip.x + strip.w * 0.50f, strip.y, strip.w * 0.25f, strip.h},
        SDL_FRect{strip.x + strip.w * 0.75f, strip.y, strip.w * 0.25f, strip.h},
    };
    const char* values[] = {
        tr("resource.wood", "Wood").c_str(),
        tr("resource.metal", "Metal").c_str(),
        tr("resource.med", "Med").c_str(),
        tr("resource.gems", "Gems").c_str()
    };
    int numbers[] = {
        inventory.resources().wood,
        inventory.resources().metal,
        inventory.resources().medicine,
        inventory.resources().gems
    };
    for (int i = 0; i < 4; ++i) {
        ui::Panel panel(entries[i].x, entries[i].y, entries[i].w - 10.0f, entries[i].h);
        panel.render(renderer_, resolve_skin(assets, layout_.resource_strip_skin(), assets.panel_skin), font_, presentation_rect_, text_primary, 236);
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%s %d", values[i], numbers[i]);
        render_text_centered(body_font_ != nullptr ? body_font_ : font_, buffer, SDL_FRect{entries[i].x, entries[i].y, entries[i].w - 10.0f, entries[i].h}, text_primary);
    }
}

void MarketScreen::render_left_panels(const Assets& assets, const InventoryState& inventory) const
{
    const SDL_Color text_primary{234, 222, 205, 255};
    const SDL_Color text_secondary{176, 157, 132, 255};
    const SDL_Color accent_green{102, 168, 86, 255};
    const SDL_FRect merchant = layout_.merchant_panel_rect();
    ui::Panel merchant_panel(merchant.x, merchant.y, merchant.w, merchant.h);
    merchant_panel.set_title(tr("panel.link_status", "Black Market Radio").c_str()).render(renderer_, resolve_skin(assets, layout_.merchant_panel_skin(), assets.panel_skin), font_, presentation_rect_, text_primary, 236);
    render_text(body_font_ != nullptr ? body_font_ : font_, tr("merchant.alias", "Arms Broker").c_str(), merchant.x + 18.0f, merchant.y + 52.0f, text_primary);
    render_text(small_font_ != nullptr ? small_font_ : font_, tr("merchant.signal", "Signal: Good").c_str(), merchant.x + 18.0f, merchant.y + 88.0f, accent_green);
    render_text(small_font_ != nullptr ? small_font_ : font_, tr("merchant.trust", "Trust Level: Lv.2").c_str(), merchant.x + 18.0f, merchant.y + 116.0f, text_secondary);
    ui::ProgressBar trust_bar(merchant.x + 18.0f, merchant.y + 148.0f, merchant.w - 36.0f, 16.0f, ui::ProgressBarOrientation::Horizontal);
    trust_bar.set_progress(0.53f);
    trust_bar.render(renderer_, assets.progressbar_horizontal_track_style, assets.progressbar_horizontal_fill_style, presentation_rect_, 235);

    const SDL_FRect cart = layout_.cart_panel_rect();
    ui::Panel cart_panel(cart.x, cart.y, cart.w, cart.h);
    cart_panel.set_title(tr("panel.cart", "Purchased Ledger").c_str()).render(renderer_, resolve_skin(assets, layout_.cart_panel_skin(), assets.panel_skin), font_, presentation_rect_, text_primary, 236);
    render_text_centered(body_font_ != nullptr ? body_font_ : font_, tr("cart.empty", "Empty").c_str(), SDL_FRect{cart.x, cart.y + 48.0f, cart.w, 42.0f}, text_secondary);
    char total_text[32];
    std::snprintf(total_text, sizeof(total_text), "%s %d", tr("cart.total", "Total").c_str(), 0);
    render_text(body_font_ != nullptr ? body_font_ : font_, total_text, cart.x + 18.0f, cart.y + cart.h - 56.0f, text_primary);

    const SDL_FRect timer = layout_.timer_panel_rect();
    ui::Panel timer_panel(timer.x, timer.y, timer.w, timer.h);
    timer_panel.set_title(tr("panel.next_contact", "Next Contact").c_str()).render(renderer_, resolve_skin(assets, layout_.timer_panel_skin(), assets.panel_skin), font_, presentation_rect_, text_primary, 236);
    render_text_centered(title_font_ != nullptr ? title_font_ : font_, "02:30:00", SDL_FRect{timer.x, timer.y + 44.0f, timer.w, 36.0f}, text_primary);

    const SDL_FRect note = layout_.note_panel_rect();
    ui::Panel note_panel(note.x, note.y, note.w, note.h);
    note_panel.set_title(tr("panel.rules", "Trade Rules").c_str()).render(renderer_, resolve_skin(assets, layout_.note_panel_skin(), assets.panel_skin), font_, presentation_rect_, text_primary, 228);
    render_text(small_font_ != nullptr ? small_font_ : font_, tr("rules.line1", "1. Pay first, receive later").c_str(), note.x + 16.0f, note.y + 42.0f, text_secondary);
    render_text(small_font_ != nullptr ? small_font_ : font_, tr("rules.line2", "2. Delivery by relay drop").c_str(), note.x + 16.0f, note.y + 66.0f, text_secondary);
    render_text(small_font_ != nullptr ? small_font_ : font_, tr("rules.line3", "3. No refunds, no questions").c_str(), note.x + 16.0f, note.y + 90.0f, text_secondary);
    (void)inventory;
}

void MarketScreen::render_notebook(const Assets& assets, const WeaponCatalog&, float mouse_x, float mouse_y, bool mouse_in_view, bool mouse_down, bool mouse_pressed, bool mouse_released)
{
    const SDL_Color ink_dark{42, 31, 21, 255};
    const SDL_Color ink_light{108, 83, 61, 255};
    const SDL_FRect notebook = layout_.notebook_rect();
    render_text_centered(title_font_ != nullptr ? title_font_ : font_, tr("catalog.title", "Black Market Catalog").c_str(), SDL_FRect{notebook.x + 120.0f, notebook.y + 26.0f, notebook.w - 240.0f, 30.0f}, ink_dark);
    render_text_centered(body_font_ != nullptr ? body_font_ : font_, tr("catalog.subtitle", "Confidential").c_str(), SDL_FRect{notebook.x + 120.0f, notebook.y + 58.0f, notebook.w - 240.0f, 22.0f}, SDL_Color{135, 58, 52, 255});

    const int items_per_page = std::max(1, layout_.catalog_item_count());
    const int start_index = current_page_ * items_per_page;
    const int end_index = std::min(static_cast<int>(filtered_entries_.size()), start_index + items_per_page);
    for (int i = start_index; i < end_index; ++i) {
        const CatalogEntry& entry = filtered_entries_[static_cast<size_t>(i)];
        const int local_index = i - start_index;
        const SDL_FRect item_rect = layout_.catalog_item_rect(local_index);
        const Texture* icon = entry.definition->icon_texture.valid()
            ? &entry.definition->icon_texture
            : (entry.definition->preview_texture.valid() ? &entry.definition->preview_texture : &entry.definition->texture);
        render_texture_fit(renderer_, *icon, SDL_FRect{to_screen_x(item_rect.x + 12.0f), to_screen_y(item_rect.y + 20.0f), presentation_rect_.w * 0.07f, presentation_rect_.h * 0.11f}, true);
        render_text(body_font_ != nullptr ? body_font_ : font_, entry.definition->name.c_str(), item_rect.x + 92.0f, item_rect.y + 10.0f, ink_dark);
        render_text(small_font_ != nullptr ? small_font_ : font_, weapon_category_label(*entry.definition).c_str(), item_rect.x + 92.0f, item_rect.y + 38.0f, ink_light);
        char price_text[32];
        std::snprintf(price_text, sizeof(price_text), "%d", entry.price);
        render_text(body_font_ != nullptr ? body_font_ : font_, price_text, item_rect.x + item_rect.w - 40.0f, item_rect.y + 10.0f, SDL_Color{132, 44, 39, 255});
        for (int row = 0; row < 5; ++row) {
            const float row_y = item_rect.y + 64.0f + row * 20.0f;
            render_text(small_font_ != nullptr ? small_font_ : font_, tr(row == 0 ? "stat.damage" : row == 1 ? "stat.fire_rate" : row == 2 ? "stat.accuracy" : row == 3 ? "stat.stability" : "stat.magazine", row == 0 ? "Damage" : row == 1 ? "Fire Rate" : row == 2 ? "Accuracy" : row == 3 ? "Stability" : "Magazine").c_str(), item_rect.x + 92.0f, row_y, ink_light);
            ui::ProgressBar bar(item_rect.x + 170.0f, row_y + 3.0f, 90.0f, 10.0f, ui::ProgressBarOrientation::Horizontal);
            bar.set_progress(stat_ratio_for_row(*entry.definition, row));
            bar.render(renderer_, assets.progressbar_horizontal_track_style, assets.progressbar_horizontal_fill_style, presentation_rect_, 180);
            char value_text[32];
            if (row == 1) {
                std::snprintf(value_text, sizeof(value_text), "%.1f", stat_value_for_row(*entry.definition, row));
            } else {
                std::snprintf(value_text, sizeof(value_text), "%.0f", stat_value_for_row(*entry.definition, row));
            }
            render_text(small_font_ != nullptr ? small_font_ : font_, value_text, item_rect.x + item_rect.w - 26.0f, row_y, ink_dark);
        }
    }

    const int max_page = std::max(0, static_cast<int>((filtered_entries_.size() + items_per_page - 1) / items_per_page) - 1);
    const SDL_FRect prev_rect = layout_.page_button_rect(0);
    const SDL_FRect next_rect = layout_.page_button_rect(1);
    ui::Button prev_button(tr("button.prev", "Prev").c_str(), prev_rect.x, prev_rect.y, prev_rect.w, prev_rect.h, current_page_ > 0);
    ui::Button next_button(tr("button.next", "Next").c_str(), next_rect.x, next_rect.y, next_rect.w, next_rect.h, current_page_ < max_page);
    if (prev_button.process_pointer(mouse_x, mouse_y, mouse_in_view, mouse_pressed, mouse_released, prev_page_armed_) && current_page_ > 0) {
        --current_page_;
    }
    if (next_button.process_pointer(mouse_x, mouse_y, mouse_in_view, mouse_pressed, mouse_released, next_page_armed_) && current_page_ < max_page) {
        ++current_page_;
    }
    prev_button.render(renderer_, assets.title_button_skin, small_font_ != nullptr ? small_font_ : font_, presentation_rect_, mouse_x, mouse_y, mouse_in_view, mouse_down, prev_page_armed_, SDL_Color{234, 222, 205, 255}, SDL_Color{142, 136, 128, 255}, 224);
    next_button.render(renderer_, assets.title_button_skin, small_font_ != nullptr ? small_font_ : font_, presentation_rect_, mouse_x, mouse_y, mouse_in_view, mouse_down, next_page_armed_, SDL_Color{234, 222, 205, 255}, SDL_Color{142, 136, 128, 255}, 224);

    char page_text[16];
    std::snprintf(page_text, sizeof(page_text), "%d / %d", current_page_ + 1, max_page + 1);
    render_text_centered(body_font_ != nullptr ? body_font_ : font_, page_text, SDL_FRect{prev_rect.x + 90.0f, prev_rect.y + 4.0f, 60.0f, prev_rect.h}, ink_dark);
}

void MarketScreen::render_category_buttons(const Assets& assets, float mouse_x, float mouse_y, bool mouse_in_view, bool mouse_down, bool mouse_pressed, bool mouse_released)
{
    std::vector<std::string> labels;
    labels.reserve(static_cast<size_t>(layout_.category_button_count()));
    for (int i = 0; i < layout_.category_button_count(); ++i) {
        labels.push_back(tr(category_key(i), category_key(i)));
    }

    const SDL_FRect first = layout_.category_button_rect(0);
    const SDL_FRect last = layout_.category_button_rect(layout_.category_button_count() - 1);
    category_tabs_
        .set_rect(first.x, first.y, first.w, (last.y + last.h) - first.y)
        .set_orientation(ui::TabBarOrientation::Vertical)
        .set_tab_gap(last.y > first.y ? (last.y - first.y - first.h) : 0.0f)
        .set_labels(labels)
        .set_selected_index(static_cast<int>(active_category_));

    const int activated = category_tabs_.update_and_render(
        renderer_,
        assets.title_button_skin,
        small_font_ != nullptr ? small_font_ : font_,
        presentation_rect_,
        SDL_Color{234, 222, 205, 255},
        SDL_Color{142, 136, 128, 255},
        mouse_x,
        mouse_y,
        mouse_in_view,
        mouse_down,
        mouse_pressed,
        mouse_released,
        234);
    if (activated >= 0) {
        active_category_ = static_cast<MarketCategory>(activated);
        current_page_ = 0;
    }
}

const std::string& MarketScreen::tr(const char* key, const char* fallback) const
{
    return strings_.get(key, fallback);
}

std::string MarketScreen::weapon_category_label(const WeaponDefinition& definition) const
{
    if (definition.ui_card_template == "pistol") {
        return tr("category.pistol", "Pistol");
    }
    if (definition.ui_card_template == "shotgun") {
        return tr("category.shotgun", "Shotgun");
    }
    if (definition.ui_card_template == "rifle") {
        return tr("category.rifle", "Rifle");
    }
    if (definition.ui_card_template == "sniper") {
        return tr("category.sniper", "Sniper");
    }
    if (definition.ui_card_template == "smg") {
        return tr("category.smg", "SMG");
    }
    if (definition.type == WeaponType::Grenade) {
        return tr("category.other", "Other");
    }
    return tr("category.other", "Other");
}

float MarketScreen::stat_value_for_row(const WeaponDefinition& definition, int row) const
{
    switch (row) {
    case 0: return static_cast<float>(definition.damage);
    case 1: return static_cast<float>(definition.speed_rpm) / 100.0f;
    case 2: return std::max(35.0f, 100.0f - definition.up * 0.8f);
    case 3: return std::max(20.0f, 100.0f - definition.up * 1.1f);
    default: return static_cast<float>(definition.magazine_size);
    }
}

float MarketScreen::stat_ratio_for_row(const WeaponDefinition& definition, int row) const
{
    switch (row) {
    case 0: return std::min(1.0f, definition.damage / 100.0f);
    case 1: return std::min(1.0f, definition.speed_rpm / 1000.0f);
    case 2: return std::min(1.0f, std::max(35.0f, 100.0f - definition.up * 0.8f) / 100.0f);
    case 3: return std::min(1.0f, std::max(20.0f, 100.0f - definition.up * 1.1f) / 100.0f);
    default: return std::min(1.0f, definition.magazine_size / 100.0f);
    }
}

float MarketScreen::to_screen_x(float logical_x) const
{
    return presentation_rect_.x + logical_x * ui_presentation_scale_x(presentation_rect_);
}

float MarketScreen::to_screen_y(float logical_y) const
{
    return presentation_rect_.y + logical_y * ui_presentation_scale_y(presentation_rect_);
}

SDL_FRect MarketScreen::to_screen_rect(const SDL_FRect& logical_rect) const
{
    return ui_logical_to_present_rect(logical_rect, presentation_rect_);
}

} // namespace zg
