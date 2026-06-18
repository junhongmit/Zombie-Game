#include "InventoryScreen.h"

#include "../Assets.h"
#include "../Constants.h"
#include "../InventoryState.h"
#include "../Presentation.h"
#include "../Texture.h"
#include "../Weapon.h"
#include "Card.h"
#include "Panel.h"
#include "ProgressBar.h"

#include <algorithm>
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

void fill_darken_overlay(SDL_Renderer* renderer, const SDL_FRect& presentation_rect)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 118);
    SDL_RenderFillRect(renderer, &presentation_rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

} // namespace

InventoryScreen::InventoryScreen(SDL_Renderer* renderer)
    : renderer_(renderer)
{
    layout_.load("assets/ui/layouts/inventory.json");
    strings_.load("assets/localization/us-en/inventory.loc");
    font_point_size_ = static_cast<int>(kHudFontPointSize);
    font_ = load_ui_font(font_point_size_);
}

InventoryScreen::~InventoryScreen()
{
    if (font_ != nullptr) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
}

void InventoryScreen::render(
    const Assets& assets,
    const InventoryState& inventory,
    const WeaponState& weapon_state,
    const SDL_FRect& presentation_rect)
{
    presentation_rect_ = presentation_rect;
    ensure_font(ui_scale_for_rect(presentation_rect_));
    fill_darken_overlay(renderer_, presentation_rect_);

    const SDL_Color title_color{241, 226, 205, 255};
    const SDL_Color text_color{218, 200, 174, 255};
    const SDL_Color sub_color{176, 157, 132, 255};
    const SDL_Color accent_color{208, 170, 79, 255};
    const SDL_Color green_color{128, 181, 93, 255};

    ui::Panel equipment_panel(layout_.equipment_panel_rect().x, layout_.equipment_panel_rect().y, layout_.equipment_panel_rect().w, layout_.equipment_panel_rect().h);
    equipment_panel.set_title(tr("panel.equipment", "Equipment").c_str()).render(renderer_, assets.panel_skin, font_, presentation_rect_, title_color, 244);
    ui::Panel bag_panel(layout_.bag_panel_rect().x, layout_.bag_panel_rect().y, layout_.bag_panel_rect().w, layout_.bag_panel_rect().h);
    char bag_title[64];
    std::snprintf(bag_title, sizeof(bag_title), "%s  %d/%d", tr("panel.backpack", "Backpack").c_str(), inventory.bag_used_slots(), inventory.bag_capacity());
    bag_panel.set_title(bag_title).render(renderer_, assets.panel_skin, font_, presentation_rect_, title_color, 244);
    ui::Panel resources_panel(layout_.resources_panel_rect().x, layout_.resources_panel_rect().y, layout_.resources_panel_rect().w, layout_.resources_panel_rect().h);
    resources_panel.set_title(tr("panel.resources", "Resources (Base Storage)").c_str()).render(renderer_, assets.panel_skin, font_, presentation_rect_, title_color, 244);
    ui::Panel weight_panel(layout_.weight_panel_rect().x, layout_.weight_panel_rect().y, layout_.weight_panel_rect().w, layout_.weight_panel_rect().h);
    weight_panel.set_title(tr("panel.weight", "Carry Info").c_str()).render(renderer_, assets.panel_skin, font_, presentation_rect_, title_color, 244);
    ui::Panel backpack_panel(layout_.backpack_panel_rect().x, layout_.backpack_panel_rect().y, layout_.backpack_panel_rect().w, layout_.backpack_panel_rect().h);
    backpack_panel.set_title(tr("panel.backpack_types", "Backpack Types").c_str()).render(renderer_, assets.panel_skin, font_, presentation_rect_, title_color, 244);

    const SDL_FRect paperdoll = layout_.paperdoll_rect();
    render_text_centered("SURVIVOR", paperdoll, sub_color, -90.0f);
    const SDL_FRect paperdoll_screen = ui_logical_to_present_rect(paperdoll, presentation_rect_);
    SDL_RenderTexture(renderer_, assets.hero.get(), nullptr, &paperdoll_screen);

    const WeaponSlot* weapon_slots = weapon_state.slots();
    const std::string& equip_primary = tr("equip.primary", "Primary");
    const std::string& equip_secondary = tr("equip.secondary", "Secondary");
    const std::string& equip_melee = tr("equip.melee", "Melee");
    const std::string& equip_melee_value = tr("equip.melee.value", "Baseball Bat");
    const std::string& equip_armor = tr("equip.armor", "Armor");
    const std::string& equip_armor_value = tr("equip.armor.value", "Leather Jacket");
    struct EquipmentEntry {
        const char* title;
        const char* subtitle;
        const Texture* icon;
        const char* meta;
    };
    const EquipmentEntry entries[] = {
        {equip_primary.c_str(), weapon_slots[0].definition != nullptr ? weapon_slots[0].definition->name.c_str() : "-", weapon_slots[0].definition != nullptr ? &weapon_slots[0].definition->preview_texture : nullptr, ""},
        {equip_secondary.c_str(), weapon_slots[1].definition != nullptr ? weapon_slots[1].definition->name.c_str() : "-", weapon_slots[1].definition != nullptr ? &weapon_slots[1].definition->preview_texture : nullptr, ""},
        {equip_melee.c_str(), equip_melee_value.c_str(), nullptr, ""},
        {equip_armor.c_str(), equip_armor_value.c_str(), nullptr, ""},
    };
    for (int i = 0; i < 4; ++i) {
        const SDL_FRect rect = layout_.equipment_entry_rect(i);
        ui::Card card(rect.x, rect.y, rect.w, rect.h, true);
        card.set_title(entries[i].title).set_subtitle(entries[i].subtitle).set_meta(entries[i].meta).set_icon(entries[i].icon);
        card.render(renderer_, assets.weapon_card_style, font_, font_, presentation_rect_, -1000.0f, -1000.0f, false, false, false, title_color, text_color, sub_color, 240);
    }

    const char* paperdoll_labels[] = {"Head", "Body", "Legs", "Tool"};
    for (int i = 0; i < 4; ++i) {
        const SDL_FRect rect = layout_.paperdoll_slot_rect(i);
        assets.title_button_skin.render(renderer_, ui_logical_to_present_rect(rect, presentation_rect_), ControlVisualState::Normal, 208);
        render_text_centered(paperdoll_labels[i], rect, sub_color, 0.0f);
    }

    for (int i = 0; i < inventory.bag_slot_count(); ++i) {
        const InventorySlot& slot = inventory.bag_slots()[i];
        const InventoryItemDefinition* item = inventory.item_definition(slot.item_index);
        const SDL_FRect rect = layout_.bag_slot_rect(i);
        ui::Card card(rect.x, rect.y, rect.w, rect.h, !slot.locked);
        if (item != nullptr) {
            char meta[16];
            std::snprintf(meta, sizeof(meta), "%d", slot.quantity);
            card.set_title(item->name.c_str()).set_subtitle(item->subtitle.c_str()).set_meta(meta).set_icon(item->icon.valid() ? &item->icon : nullptr);
        } else {
            card.set_title("").set_subtitle("").set_meta("");
        }
        card.set_selected(i == 0);
        card.render(renderer_, assets.weapon_card_style, font_, font_, presentation_rect_, -1000.0f, -1000.0f, false, false, false, title_color, text_color, sub_color, 236);
    }

    const ResourceStockpile& resources = inventory.resources();
    const std::string& wood_label = tr("resource.wood", "Wood");
    const std::string& metal_label = tr("resource.metal", "Metal");
    const std::string& cloth_label = tr("resource.cloth", "Cloth");
    const std::string& electronics_label = tr("resource.electronics", "Electronics");
    const std::string& fuel_label = tr("resource.fuel", "Fuel");
    const std::string& parts_label = tr("resource.parts", "Parts");
    const std::string& glass_label = tr("resource.glass", "Glass");
    const std::string& food_label = tr("resource.food", "Food");
    const std::string& water_label = tr("resource.water", "Water");
    const std::string& medicine_label = tr("resource.medicine", "Medicine");
    struct ResourceLine { const char* label; int value; };
    const ResourceLine resource_lines[] = {
        {wood_label.c_str(), resources.wood},
        {metal_label.c_str(), resources.metal},
        {cloth_label.c_str(), resources.cloth},
        {electronics_label.c_str(), resources.electronics},
        {fuel_label.c_str(), resources.fuel},
        {parts_label.c_str(), resources.parts},
        {glass_label.c_str(), resources.glass},
        {food_label.c_str(), resources.food},
        {water_label.c_str(), resources.water},
        {medicine_label.c_str(), resources.medicine},
    };
    const SDL_FRect resources_rect = layout_.resources_panel_rect();
    float line_y = resources_rect.y + 60.0f;
    for (int i = 0; i < 10; ++i) {
        char value_text[16];
        std::snprintf(value_text, sizeof(value_text), "%d", resource_lines[i].value);
        render_text(resource_lines[i].label, resources_rect.x + 20.0f, line_y, text_color);
        render_text(value_text, resources_rect.x + resources_rect.w - 52.0f, line_y, title_color);
        line_y += 38.0f;
    }
    render_text(tr("resource.note", "* Return home to auto-store items").c_str(), resources_rect.x + 20.0f, resources_rect.y + resources_rect.h - 42.0f, sub_color);

    const SDL_FRect weight_rect = layout_.weight_panel_rect();
    char weight_text[32];
    char max_weight_text[32];
    char remain_text[32];
    std::snprintf(weight_text, sizeof(weight_text), "%.1f kg", inventory.current_weight());
    std::snprintf(max_weight_text, sizeof(max_weight_text), "%.1f kg", inventory.max_weight());
    std::snprintf(remain_text, sizeof(remain_text), "%.1f kg", inventory.remaining_weight());
    render_text(tr("weight.current", "Current Weight").c_str(), weight_rect.x + 20.0f, weight_rect.y + 66.0f, text_color);
    render_text(weight_text, weight_rect.x + weight_rect.w - 108.0f, weight_rect.y + 66.0f, title_color);
    render_text(tr("weight.max", "Max Weight").c_str(), weight_rect.x + 20.0f, weight_rect.y + 104.0f, text_color);
    render_text(max_weight_text, weight_rect.x + weight_rect.w - 108.0f, weight_rect.y + 104.0f, title_color);
    render_text(tr("weight.remaining", "Remaining").c_str(), weight_rect.x + 20.0f, weight_rect.y + 142.0f, text_color);
    render_text(remain_text, weight_rect.x + weight_rect.w - 108.0f, weight_rect.y + 142.0f, green_color);
    ui::ProgressBar weight_bar(weight_rect.x + 22.0f, weight_rect.y + weight_rect.h - 42.0f, weight_rect.w - 44.0f, 16.0f, ui::ProgressBarOrientation::Horizontal);
    weight_bar.set_progress(inventory.max_weight() > 0.0f ? inventory.current_weight() / inventory.max_weight() : 0.0f);
    weight_bar.render(renderer_, assets.progressbar_horizontal_track_style, assets.progressbar_horizontal_fill_style, presentation_rect_, 238);

    for (int i = 0; i < inventory.backpack_definition_count(); ++i) {
        const BackpackDefinition& backpack = inventory.backpack_definitions()[i];
        const SDL_FRect rect = layout_.backpack_option_rect(i);
        ui::Card card(rect.x, rect.y, rect.w, rect.h, true);
        char meta[32];
        std::snprintf(meta, sizeof(meta), "%.0f kg", backpack.capacity_kg);
        card.set_selected(backpack.selected).set_title(backpack.name.c_str()).set_subtitle(backpack.mobility.c_str()).set_meta(meta);
        card.render(renderer_, assets.weapon_card_style, font_, font_, presentation_rect_, -1000.0f, -1000.0f, false, false, false, title_color, text_color, accent_color, 236);
    }

    const std::string& action_sort = tr("action.sort", "R  Sort");
    const std::string& action_use = tr("action.use", "E  Use");
    const std::string& action_drop = tr("action.drop", "T  Drop");
    const std::string& action_split = tr("action.split", "F  Split");
    const std::string& action_close = tr("action.close", "Esc  Close");
    const char* action_labels[] = {
        action_sort.c_str(),
        action_use.c_str(),
        action_drop.c_str(),
        action_split.c_str(),
        action_close.c_str(),
    };
    for (int i = 0; i < 5; ++i) {
        const SDL_FRect rect = layout_.action_hint_rect(i);
        assets.title_button_skin.render(renderer_, ui_logical_to_present_rect(rect, presentation_rect_), ControlVisualState::Normal, 232);
        render_text_centered(action_labels[i], rect, title_color, 0.0f);
    }
}

void InventoryScreen::ensure_font(float ui_scale)
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

void InventoryScreen::render_text(const char* text, float x, float y, SDL_Color color) const
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
        presentation_rect_.x + x * ui_presentation_scale_x(presentation_rect_),
        presentation_rect_.y + y * ui_presentation_scale_y(presentation_rect_),
        static_cast<float>(surface->w),
        static_cast<float>(surface->h)
    };
    SDL_RenderTexture(renderer_, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void InventoryScreen::render_text_centered(const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset) const
{
    if (font_ == nullptr || text == nullptr || text[0] == '\0') {
        return;
    }
    int text_width = 0;
    int text_height = 0;
    if (!TTF_GetStringSize(font_, text, 0, &text_width, &text_height)) {
        return;
    }
    const float x_scale = ui_presentation_scale_x(presentation_rect_);
    const float y_scale = ui_presentation_scale_y(presentation_rect_);
    const float logical_text_width = static_cast<float>(text_width) / x_scale;
    const float logical_text_height = static_cast<float>(text_height) / y_scale;
    render_text(
        text,
        rect.x + std::floor((rect.w - logical_text_width) * 0.5f),
        rect.y + std::floor((rect.h - logical_text_height) * 0.5f) + y_offset,
        color);
}

const std::string& InventoryScreen::tr(const char* key, const char* fallback) const
{
    return strings_.get(key, fallback);
}

} // namespace zg
