#include "InventoryScreen.h"

#include "../Assets.h"
#include "../AssetPaths.h"
#include "../Constants.h"
#include "../InventoryState.h"
#include "../Presentation.h"
#include "../Texture.h"
#include "../Weapon.h"
#include "Card.h"
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

constexpr float kDragThreshold = 8.0f;
constexpr float kTooltipFadeDistance = 120.0f;
constexpr float kTooltipFadeSpeed = 4.0f;
constexpr float kCloseFlashDuration = 0.18f;

float snap_ui_logical_x(float logical_x, const SDL_FRect& presentation_rect)
{
    const float scale = ui_presentation_scale_x(presentation_rect);
    if (scale <= 0.0f) {
        return logical_x;
    }
    return std::round(logical_x * scale) / scale;
}

float snap_ui_logical_y(float logical_y, const SDL_FRect& presentation_rect)
{
    const float scale = ui_presentation_scale_y(presentation_rect);
    if (scale <= 0.0f) {
        return logical_y;
    }
    return std::round(logical_y * scale) / scale;
}

TTF_Font* load_font_from_candidates(const char* const* candidates, int point_size)
{
    for (int i = 0; candidates[i] != nullptr; ++i) {
        const std::string path = resolve_asset_path(candidates[i]);
        TTF_Font* font = TTF_OpenFont(path.c_str(), point_size);
        if (font != nullptr) {
            return font;
        }
    }
    return nullptr;
}

TTF_Font* load_section_font(int point_size)
{
    static const char* kCandidates[] = {
        "assets/fonts/NotoSansSC-Bold.ttf",
        "assets/fonts/NotoSansSC-SemiBold.ttf",
        "C:\\Windows\\Fonts\\NotoSansSC-Bold.ttf",
        "C:\\Windows\\Fonts\\NotoSansSC-Regular.ttf",
        "C:\\Windows\\Fonts\\simhei.ttf",
        "C:\\Windows\\Fonts\\msyhbd.ttc",
        nullptr
    };
    return load_font_from_candidates(kCandidates, point_size);
}

TTF_Font* load_title_font(int point_size)
{
    static const char* kCandidates[] = {
        "assets/fonts/NotoSansSC-Medium.ttf",
        "assets/fonts/NotoSansSC-Regular.ttf",
        "C:\\Windows\\Fonts\\NotoSansSC-Medium.ttf",
        "C:\\Windows\\Fonts\\NotoSansSC-Regular.ttf",
        "C:\\Windows\\Fonts\\msyh.ttc",
        "C:\\Windows\\Fonts\\simsun.ttc",
        "C:\\Windows\\Fonts\\msyh.ttc",
        nullptr
    };
    return load_font_from_candidates(kCandidates, point_size);
}

TTF_Font* load_body_font(int point_size)
{
    static const char* kCandidates[] = {
        "assets/fonts/NotoSansSC-Regular.ttf",
        "assets/fonts/NotoSansSC-Medium.ttf",
        "C:\\Windows\\Fonts\\NotoSansSC-Regular.ttf",
        "C:\\Windows\\Fonts\\NotoSansSC-Medium.ttf",
        "C:\\Windows\\Fonts\\msyh.ttc",
        "C:\\Windows\\Fonts\\simsun.ttc",
        nullptr
    };
    return load_font_from_candidates(kCandidates, point_size);
}

TTF_Font* load_key_font(int point_size)
{
    static const char* kCandidates[] = {
        "assets/fonts/BebasNeue-Regular.ttf",
        "C:\\Windows\\Fonts\\BebasNeue-Regular.ttf",
        "C:\\Windows\\Fonts\\BebasNeue.ttf",
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\consolab.ttf",
        nullptr
    };
    return load_font_from_candidates(kCandidates, point_size);
}

TTF_Font* load_number_font(int point_size)
{
    static const char* kCandidates[] = {
        "assets/fonts/DINCondensed-Bold.ttf",
        "assets/fonts/DINCondensed-Regular.ttf",
        "C:\\Windows\\Fonts\\DINCondensed-Bold.ttf",
        "C:\\Windows\\Fonts\\DINCondensed-Regular.ttf",
        "C:\\Windows\\Fonts\\arialbd.ttf",
        "C:\\Windows\\Fonts\\bahnschrift.ttf",
        nullptr
    };
    return load_font_from_candidates(kCandidates, point_size);
}

float ui_scale_for_rect(const SDL_FRect& rect)
{
    return std::min(
        rect.w / static_cast<float>(kUiDesignWidth),
        rect.h / static_cast<float>(kUiDesignHeight));
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
    if (skin_name == "card2_inventory_slot") {
        return assets.inventory_card_style;
    }
    if (skin_name == "button_square_bronze") {
        return assets.title_button_skin;
    }
    return fallback;
}

void fill_darken_overlay(SDL_Renderer* renderer, const SDL_FRect& presentation_rect)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 118);
    SDL_RenderFillRect(renderer, &presentation_rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

bool point_in_logical_rect(float x, float y, const SDL_FRect& rect, bool mouse_in_view)
{
    return mouse_in_view &&
        x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h;
}

float squared_distance(float x0, float y0, float x1, float y1)
{
    const float dx = x1 - x0;
    const float dy = y1 - y0;
    return dx * dx + dy * dy;
}

SDL_FRect child_rect_in_parent(const SDL_FRect& parent, float x, float y, float w, float h)
{
    return SDL_FRect{parent.x + x, parent.y + y, w, h};
}

SDL_FRect action_hint_rect_for_index(const SDL_FRect& action_content_rect, int index)
{
    const float item_width = 116.0f;
    const float item_height = action_content_rect.h - 6.0f;
    const float gap = 10.0f;
    return child_rect_in_parent(action_content_rect, index * (item_width + gap), 3.0f, item_width, item_height);
}

SDL_FRect header_close_rect(const SDL_FRect& page_rect)
{
    return child_rect_in_parent(page_rect, page_rect.w - 34.0f, 6.0f, 28.0f, 28.0f);
}

} // namespace

InventoryScreen::InventoryScreen(SDL_Renderer* renderer)
    : renderer_(renderer)
{
    layout_hot_reload_.set_path("assets/ui/layouts/inventory.json");
    layout_.load("assets/ui/layouts/inventory.json");
    layout_hot_reload_.mark_loaded();
    if (!strings_.load("assets/localization/zh-cn/inventory.loc")) {
        strings_.load("assets/localization/us-en/inventory.loc");
    }
    base_point_size_ = static_cast<int>(kHudFontPointSize);
    section_font_ = load_section_font(base_point_size_ + 12);
    title_font_ = load_title_font(base_point_size_ + 5);
    body_font_ = load_body_font(base_point_size_ + 3);
    small_font_ = load_body_font(std::max(12, base_point_size_ + 1));
    key_font_ = load_key_font(base_point_size_ + 14);
    number_font_ = load_number_font(base_point_size_ + 8);
}

InventoryScreen::~InventoryScreen()
{
    if (section_font_ != nullptr) {
        TTF_CloseFont(section_font_);
        section_font_ = nullptr;
    }
    if (title_font_ != nullptr) {
        TTF_CloseFont(title_font_);
        title_font_ = nullptr;
    }
    if (body_font_ != nullptr) {
        TTF_CloseFont(body_font_);
        body_font_ = nullptr;
    }
    if (small_font_ != nullptr) {
        TTF_CloseFont(small_font_);
        small_font_ = nullptr;
    }
    if (key_font_ != nullptr) {
        TTF_CloseFont(key_font_);
        key_font_ = nullptr;
    }
    if (number_font_ != nullptr) {
        TTF_CloseFont(number_font_);
        number_font_ = nullptr;
    }
}

bool InventoryScreen::update_and_render(
    const Assets& assets,
    InventoryState& inventory,
    const WeaponState& weapon_state,
    const SDL_FRect& presentation_rect,
    float dt,
    float mouse_x,
    float mouse_y,
    bool mouse_in_view,
    bool mouse_down,
    bool mouse_pressed,
    bool mouse_released,
    bool sort_pressed,
    bool use_pressed,
    bool drop_pressed,
    bool split_pressed)
{
    (void)dt;
    if (kEnableUiLayoutHotReload && layout_hot_reload_.poll_changed()) {
        layout_.load("assets/ui/layouts/inventory.json");
    }
    presentation_rect_ = presentation_rect;
    ensure_fonts(ui_scale_for_rect(presentation_rect_));
    fill_darken_overlay(renderer_, presentation_rect_);

    const SDL_Color title_color{241, 226, 205, 255};
    const SDL_Color text_color{218, 200, 174, 255};
    const SDL_Color sub_color{176, 157, 132, 255};
    const SDL_Color accent_color{208, 170, 79, 255};
    const SDL_Color green_color{128, 181, 93, 255};
    const SDL_Color hotkey_color{245, 218, 155, 255};
    bool close_requested = false;
    if (close_flash_timer_ > 0.0f) {
        close_flash_timer_ = std::max(0.0f, close_flash_timer_ - dt);
    }
    if (close_pending_ && close_flash_timer_ <= 0.0f) {
        close_pending_ = false;
        close_requested = true;
    }

    if (mouse_pressed) {
        press_source_ = DragSource::None;
        press_index_ = -1;
        armed_action_ = InventoryAction::None;
    }

    const int available_bag_slots = std::min(inventory.bag_slot_count(), std::max(0, inventory.bag_capacity()));
    for (int i = 0; i < available_bag_slots; ++i) {
        if (mouse_pressed && point_in_logical_rect(mouse_x, mouse_y, layout_.bag_slot_rect(i), mouse_in_view)) {
            selected_bag_index_ = i;
            if (inventory.bag_slots()[i].item_index >= 0 && !inventory.bag_slots()[i].locked) {
                press_source_ = DragSource::Bag;
                press_index_ = i;
                press_start_x_ = mouse_x;
                press_start_y_ = mouse_y;
            }
        }
    }
    for (int i = 2; i < inventory.equipment_slot_count(); ++i) {
        if (mouse_pressed && point_in_logical_rect(mouse_x, mouse_y, layout_.equipment_entry_rect(i), mouse_in_view)) {
            if (inventory.equipped_slots()[i].item_index >= 0) {
                press_source_ = DragSource::Equipment;
                press_index_ = i;
                press_start_x_ = mouse_x;
                press_start_y_ = mouse_y;
            }
        }
    }
    const SDL_FRect backpack_rect = layout_.backpack_option_rect(0);
    if (mouse_pressed &&
        point_in_logical_rect(mouse_x, mouse_y, backpack_rect, mouse_in_view) &&
        inventory.equipped_backpack_slot().item_index >= 0) {
        press_source_ = DragSource::Backpack;
        press_index_ = 0;
        press_start_x_ = mouse_x;
        press_start_y_ = mouse_y;
    }

    if (mouse_down && drag_source_ == DragSource::None && press_source_ != DragSource::None) {
        if (squared_distance(mouse_x, mouse_y, press_start_x_, press_start_y_) >= kDragThreshold * kDragThreshold) {
            drag_source_ = press_source_;
            drag_index_ = press_index_;
            press_source_ = DragSource::None;
            press_index_ = -1;
        }
    }
    if (sort_pressed) {
        inventory.sort_bag();
    }
    if (use_pressed) {
        inventory.use_bag_slot(selected_bag_index_);
    }
    if (drop_pressed) {
        inventory.drop_bag_slot(selected_bag_index_);
    }
    if (split_pressed) {
        inventory.split_bag_slot(selected_bag_index_);
    }
    if (mouse_released && drag_source_ != DragSource::None) {
        if (drag_source_ == DragSource::Bag) {
            bool handled = false;
            for (int i = 0; i < inventory.equipment_slot_count(); ++i) {
                if (point_in_logical_rect(mouse_x, mouse_y, layout_.equipment_entry_rect(i), mouse_in_view)) {
                    inventory.equip_bag_item_to_slot(drag_index_, i);
                    selected_bag_index_ = std::max(0, std::min(drag_index_, available_bag_slots - 1));
                    handled = true;
                    break;
                }
            }
            if (!handled && point_in_logical_rect(mouse_x, mouse_y, backpack_rect, mouse_in_view)) {
                inventory.equip_backpack_from_bag(drag_index_);
                selected_bag_index_ = std::max(0, std::min(drag_index_, available_bag_slots - 1));
                handled = true;
            }
            if (!handled) {
                for (int i = 0; i < available_bag_slots; ++i) {
                    if (point_in_logical_rect(mouse_x, mouse_y, layout_.bag_slot_rect(i), mouse_in_view)) {
                        if (inventory.move_bag_item(drag_index_, i)) {
                            selected_bag_index_ = i;
                            handled = true;
                        }
                        break;
                    }
                }
            }
        } else if (drag_source_ == DragSource::Equipment || drag_source_ == DragSource::Backpack) {
            for (int i = 0; i < available_bag_slots; ++i) {
                if (point_in_logical_rect(mouse_x, mouse_y, layout_.bag_slot_rect(i), mouse_in_view)) {
                    if (drag_source_ == DragSource::Backpack) {
                        inventory.unequip_backpack_to_bag(i);
                    } else {
                        inventory.unequip_slot_to_bag_at(drag_index_, i);
                    }
                    break;
                }
            }
        }
        drag_source_ = DragSource::None;
        drag_index_ = -1;
    }
    if (mouse_released && press_source_ != DragSource::None) {
        tooltip_source_ = press_source_;
        tooltip_index_ = press_index_;
        tooltip_anchor_x_ = mouse_x;
        tooltip_anchor_y_ = mouse_y;
        tooltip_alpha_ = 1.0f;
        press_source_ = DragSource::None;
        press_index_ = -1;
    }
    const SDL_FRect page_rect = layout_.page_bounds_rect();
    const SDL_FRect close_rect = header_close_rect(page_rect);
    ui::Button close_button("X", close_rect.x, close_rect.y, close_rect.w, close_rect.h, true);
    const ControlStyle& close_button_skin = resolve_skin(assets, layout_.close_button_skin(), assets.title_button_skin);
    const ControlStyle& equipment_card_skin = resolve_skin(assets, layout_.equipment_card_skin(), assets.weapon_card_style);
    const ControlStyle& bag_slot_skin = resolve_skin(assets, layout_.bag_slot_skin(), assets.inventory_card_style);
    const ControlStyle& backpack_card_skin = resolve_skin(assets, layout_.backpack_card_skin(), assets.inventory_card_style);
    const ControlStyle& action_button_skin = resolve_skin(assets, layout_.action_button_skin(), assets.title_button_skin);
    const ControlStyle& paperdoll_slot_skin = resolve_skin(assets, layout_.paperdoll_slot_skin(), assets.title_button_skin);
    const bool close_hovered = point_in_logical_rect(mouse_x, mouse_y, close_rect, mouse_in_view);
    const bool close_clicked = close_button.process_pointer(
        mouse_x,
        mouse_y,
        mouse_in_view,
        mouse_pressed,
        mouse_released,
        close_button_armed_);
    if (close_clicked) {
        close_flash_timer_ = kCloseFlashDuration;
        close_pending_ = true;
        close_button_armed_ = false;
    } else if (mouse_released && mouse_in_view && !point_in_logical_rect(mouse_x, mouse_y, page_rect, mouse_in_view)) {
        close_flash_timer_ = kCloseFlashDuration;
        close_pending_ = true;
        close_button_armed_ = false;
    }

    selected_bag_index_ = std::max(0, std::min(selected_bag_index_, inventory.bag_slot_count() - 1));

    ui::Panel equipment_panel(layout_.equipment_panel_rect().x, layout_.equipment_panel_rect().y, layout_.equipment_panel_rect().w, layout_.equipment_panel_rect().h);
    equipment_panel.set_title(tr("panel.equipment", "Equipment").c_str()).render(renderer_, resolve_skin(assets, layout_.equipment_panel_skin(), assets.panel_skin), section_font_, presentation_rect_, title_color, 244);
    ui::Panel bag_panel(layout_.bag_panel_rect().x, layout_.bag_panel_rect().y, layout_.bag_panel_rect().w, layout_.bag_panel_rect().h);
    char bag_title[64];
    std::snprintf(bag_title, sizeof(bag_title), "%s  %d/%d", tr("panel.backpack", "Backpack").c_str(), inventory.bag_used_slots(), inventory.bag_capacity());
    bag_panel.set_title(bag_title).render(renderer_, resolve_skin(assets, layout_.bag_panel_skin(), assets.panel_skin), section_font_, presentation_rect_, title_color, 244);
    ui::Panel resources_panel(layout_.resources_panel_rect().x, layout_.resources_panel_rect().y, layout_.resources_panel_rect().w, layout_.resources_panel_rect().h);
    resources_panel.set_title(tr("panel.resources", "Resources (Base Storage)").c_str()).render(renderer_, resolve_skin(assets, layout_.resources_panel_skin(), assets.panel_skin), section_font_, presentation_rect_, title_color, 244);
    ui::Panel weight_panel(layout_.weight_panel_rect().x, layout_.weight_panel_rect().y, layout_.weight_panel_rect().w, layout_.weight_panel_rect().h);
    weight_panel.set_title(tr("panel.weight", "Carry Info").c_str()).render(renderer_, resolve_skin(assets, layout_.weight_panel_skin(), assets.panel_skin), section_font_, presentation_rect_, title_color, 244);

    const SDL_FRect resources_rect = layout_.resources_panel_rect();

    const SDL_FRect paperdoll = layout_.paperdoll_rect();
    render_text_centered(small_font_, "SURVIVOR", paperdoll, sub_color, -90.0f);
    const SDL_FRect paperdoll_screen = ui_logical_to_present_rect(paperdoll, presentation_rect_);
    SDL_RenderTexture(renderer_, assets.hero.get(), nullptr, &paperdoll_screen);

    const WeaponSlot* weapon_slots = weapon_state.slots();
    const InventorySlot* equipped_slots = inventory.equipped_slots();
    const std::string& equip_primary = tr("equip.primary", "Primary");
    const std::string& equip_secondary = tr("equip.secondary", "Secondary");
    const std::string& equip_melee = tr("equip.melee", "Melee");
    const std::string& equip_melee_value = tr("equip.empty", "Drag item here");
    const std::string& equip_armor = tr("equip.armor", "Armor");
    const std::string& equip_armor_value = tr("equip.empty", "Drag item here");
    struct EquipmentEntry {
        const char* title;
        const char* subtitle;
        const Texture* icon;
    };
    const InventoryItemDefinition* melee_item = inventory.item_definition(equipped_slots[2].item_index);
    const InventoryItemDefinition* armor_item = inventory.item_definition(equipped_slots[3].item_index);
    const EquipmentEntry entries[] = {
        {equip_primary.c_str(), weapon_slots[0].definition != nullptr ? weapon_slots[0].definition->name.c_str() : "-", weapon_slots[0].definition != nullptr ? (weapon_slots[0].definition->icon_texture.valid() ? &weapon_slots[0].definition->icon_texture : &weapon_slots[0].definition->preview_texture) : nullptr},
        {equip_secondary.c_str(), weapon_slots[1].definition != nullptr ? weapon_slots[1].definition->name.c_str() : "-", weapon_slots[1].definition != nullptr ? (weapon_slots[1].definition->icon_texture.valid() ? &weapon_slots[1].definition->icon_texture : &weapon_slots[1].definition->preview_texture) : nullptr},
        {equip_melee.c_str(), melee_item != nullptr ? melee_item->name.c_str() : equip_melee_value.c_str(), melee_item != nullptr && melee_item->icon.valid() ? &melee_item->icon : nullptr},
        {equip_armor.c_str(), armor_item != nullptr ? armor_item->name.c_str() : equip_armor_value.c_str(), armor_item != nullptr && armor_item->icon.valid() ? &armor_item->icon : nullptr},
    };
    for (int i = 0; i < 4; ++i) {
        const SDL_FRect rect = layout_.equipment_entry_rect(i);
        ui::Card card(rect.x, rect.y, rect.w, rect.h, true);
        card.set_title(entries[i].title).set_subtitle(entries[i].subtitle).set_icon(entries[i].icon);
        const bool hovered = point_in_logical_rect(mouse_x, mouse_y, rect, mouse_in_view);
        card.render(renderer_, equipment_card_skin, title_font_, body_font_, presentation_rect_, mouse_x, mouse_y, mouse_in_view, hovered && mouse_down, false, title_color, text_color, sub_color, 240);
    }

    const std::string& paperdoll_head = tr("paperdoll.head", "Head");
    const std::string& paperdoll_body = tr("paperdoll.body", "Body");
    const std::string& paperdoll_legs = tr("paperdoll.legs", "Legs");
    const std::string& paperdoll_tool = tr("paperdoll.tool", "Tool");
    const char* paperdoll_labels[] = {
        paperdoll_head.c_str(),
        paperdoll_body.c_str(),
        paperdoll_legs.c_str(),
        paperdoll_tool.c_str()
    };
    for (int i = 0; i < 4; ++i) {
        const SDL_FRect rect = layout_.paperdoll_slot_rect(i);
        paperdoll_slot_skin.render(renderer_, ui_logical_to_present_rect(rect, presentation_rect_), ControlVisualState::Normal, 208);
        render_text_centered(small_font_, paperdoll_labels[i], rect, sub_color, 0.0f);
    }

    for (int i = 0; i < inventory.bag_slot_count(); ++i) {
        const InventorySlot& slot = inventory.bag_slots()[i];
        const InventoryItemDefinition* item = inventory.item_definition(slot.item_index);
        const SDL_FRect rect = layout_.bag_slot_rect(i);
        const bool slot_enabled = i < available_bag_slots && !slot.locked;
        ui::Card card(rect.x, rect.y, rect.w, rect.h, slot_enabled);
        if (item != nullptr) {
            char meta[16];
            std::snprintf(meta, sizeof(meta), "%d", slot.quantity);
            card.set_title(item->name.c_str()).set_subtitle(item->subtitle.c_str()).set_meta(meta).set_icon(item->icon.valid() ? &item->icon : nullptr);
        } else {
            card.set_title("").set_subtitle("").set_meta("");
        }
        card.set_selected(i == selected_bag_index_ && i < available_bag_slots);
        const bool hovered = point_in_logical_rect(mouse_x, mouse_y, rect, mouse_in_view);
        card.render(renderer_, bag_slot_skin, body_font_, small_font_, presentation_rect_, mouse_x, mouse_y, mouse_in_view, hovered && mouse_down, false, title_color, text_color, sub_color, 236);
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
    ui::Canvas resources_canvas(resources_rect.x, resources_rect.y, resources_rect.w, resources_rect.h);
    for (int i = 0; i < 10; ++i) {
        char value_text[16];
        std::snprintf(value_text, sizeof(value_text), "%d", resource_lines[i].value);

        std::unique_ptr<ui::TextItem> label(new ui::TextItem());
        label->set_rect(20.0f, 72.0f + i * 38.0f, resources_rect.w - 96.0f, 22.0f);
        label->set_font(body_font_);
        label->set_text(resource_lines[i].label);
        label->set_color(text_color);
        label->set_fit_to_bounds(false);
        resources_canvas.add_child(std::move(label));

        std::unique_ptr<ui::TextItem> value(new ui::TextItem());
        value->set_rect(resources_rect.w - 72.0f, 68.0f + i * 38.0f, 52.0f, 24.0f);
        value->set_font(number_font_);
        value->set_text(value_text);
        value->set_color(title_color);
        value->set_fit_to_bounds(false);
        value->set_horizontal_align(ui::HorizontalAlign::Right);
        resources_canvas.add_child(std::move(value));
    }
    std::unique_ptr<ui::TextItem> note(new ui::TextItem());
    note->set_rect(20.0f, resources_rect.h - 34.0f, resources_rect.w - 40.0f, 18.0f);
    note->set_font(small_font_);
    note->set_text(tr("resource.note", "* Return home to auto-store items").c_str());
    note->set_color(sub_color);
    note->set_fit_to_bounds(false);
    resources_canvas.add_child(std::move(note));
    ui::RenderContext resources_context;
    resources_context.renderer = renderer_;
    resources_context.presentation_rect = presentation_rect_;
    resources_context.alpha = 255;
    resources_canvas.render(resources_context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});

    const SDL_FRect weight_rect = layout_.weight_panel_rect();
    char weight_text[32];
    char max_weight_text[32];
    char remain_text[32];
    std::snprintf(weight_text, sizeof(weight_text), "%.1f kg", inventory.current_weight());
    std::snprintf(max_weight_text, sizeof(max_weight_text), "%.1f kg", inventory.max_weight());
    std::snprintf(remain_text, sizeof(remain_text), "%.1f kg", inventory.remaining_weight());
    ui::Canvas weight_canvas(weight_rect.x, weight_rect.y, weight_rect.w, weight_rect.h);
    const char* weight_labels[] = {
        tr("weight.current", "Current Weight").c_str(),
        tr("weight.max", "Max Weight").c_str(),
        tr("weight.remaining", "Remaining").c_str(),
    };
    const char* weight_values[] = {weight_text, max_weight_text, remain_text};
    const SDL_Color weight_colors[] = {title_color, title_color, green_color};
    for (int i = 0; i < 3; ++i) {
        std::unique_ptr<ui::TextItem> label(new ui::TextItem());
        label->set_rect(20.0f, 66.0f + i * 38.0f, weight_rect.w - 140.0f, 22.0f);
        label->set_font(body_font_);
        label->set_text(weight_labels[i]);
        label->set_color(text_color);
        label->set_fit_to_bounds(false);
        weight_canvas.add_child(std::move(label));

        std::unique_ptr<ui::TextItem> value(new ui::TextItem());
        value->set_rect(weight_rect.w - 128.0f, 62.0f + i * 38.0f, 108.0f, 24.0f);
        value->set_font(number_font_);
        value->set_text(weight_values[i]);
        value->set_color(weight_colors[i]);
        value->set_fit_to_bounds(false);
        value->set_horizontal_align(ui::HorizontalAlign::Right);
        weight_canvas.add_child(std::move(value));
    }
    ui::RenderContext weight_context;
    weight_context.renderer = renderer_;
    weight_context.presentation_rect = presentation_rect_;
    weight_context.alpha = 255;
    weight_canvas.render(weight_context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});
    ui::ProgressBar weight_bar(weight_rect.x + 22.0f, weight_rect.y + weight_rect.h - 42.0f, weight_rect.w - 44.0f, 16.0f, ui::ProgressBarOrientation::Horizontal);
    weight_bar.set_progress(inventory.max_weight() > 0.0f ? inventory.current_weight() / inventory.max_weight() : 0.0f);
    weight_bar.render(renderer_, assets.progressbar_horizontal_track_style, assets.progressbar_horizontal_fill_style, presentation_rect_, 238);

    const InventorySlot& equipped_backpack_slot = inventory.equipped_backpack_slot();
    const InventoryItemDefinition* equipped_backpack_item = inventory.item_definition(equipped_backpack_slot.item_index);
    const BackpackDefinition* equipped_backpack = inventory.equipped_backpack_definition();
    ui::Card backpack_card(backpack_rect.x, backpack_rect.y, backpack_rect.w, backpack_rect.h, true);
    char backpack_meta[32];
    if (equipped_backpack != nullptr) {
        std::snprintf(backpack_meta, sizeof(backpack_meta), "%.0f kg", equipped_backpack->capacity_kg);
        backpack_card
            .set_title(equipped_backpack_item != nullptr ? equipped_backpack_item->name.c_str() : equipped_backpack->name.c_str())
            .set_subtitle(equipped_backpack->mobility.c_str())
            .set_meta(backpack_meta)
            .set_icon(equipped_backpack_item != nullptr && equipped_backpack_item->icon.valid() ? &equipped_backpack_item->icon : nullptr)
            .set_selected(false);
    } else {
        backpack_card
            .set_title(tr("backpack.none", "No Backpack").c_str())
            .set_subtitle(tr("backpack.empty_hint", "Drag a backpack item here").c_str())
            .set_meta("")
            .set_icon(nullptr)
            .set_selected(false);
    }
    const bool backpack_hovered = point_in_logical_rect(mouse_x, mouse_y, backpack_rect, mouse_in_view);
    backpack_card.render(renderer_, backpack_card_skin, body_font_, small_font_, presentation_rect_, mouse_x, mouse_y, mouse_in_view, backpack_hovered && mouse_down, false, title_color, text_color, accent_color, 236);

    ui::Panel action_panel(layout_.action_bar_rect().x, layout_.action_bar_rect().y, layout_.action_bar_rect().w, layout_.action_bar_rect().h);
    const ControlStyle& action_panel_skin = resolve_skin(assets, layout_.action_bar_skin(), assets.panel_skin);
    action_panel.render(renderer_, action_panel_skin, nullptr, presentation_rect_, title_color, 236);
    const SDL_FRect action_content_rect = ui_present_to_logical_rect(action_panel.content_rect(action_panel_skin, presentation_rect_), presentation_rect_);
    if (close_flash_timer_ > 0.0f) {
        close_button_skin.render(
            renderer_,
            ui_logical_to_present_rect(close_rect, presentation_rect_),
            ControlVisualState::Pressed,
            236);
        render_text_centered(key_font_, "X", close_rect, hotkey_color, -1.0f);
    } else {
        close_button.render(
            renderer_,
            close_button_skin,
            key_font_,
            presentation_rect_,
            mouse_x,
            mouse_y,
            mouse_in_view,
            mouse_down,
            close_button_armed_,
            hotkey_color,
            hotkey_color,
            236);
    }

    const SDL_FRect sort_rect = action_hint_rect_for_index(action_content_rect, 0);
    const SDL_FRect use_rect = action_hint_rect_for_index(action_content_rect, 1);
    const SDL_FRect drop_rect = action_hint_rect_for_index(action_content_rect, 2);
    const SDL_FRect split_rect = action_hint_rect_for_index(action_content_rect, 3);
    const SDL_FRect esc_rect = action_hint_rect_for_index(action_content_rect, 4);

    ui::Button sort_action("", sort_rect.x, sort_rect.y, sort_rect.w, sort_rect.h, true);
    ui::Button use_action("", use_rect.x, use_rect.y, use_rect.w, use_rect.h, true);
    ui::Button drop_action("", drop_rect.x, drop_rect.y, drop_rect.w, drop_rect.h, true);
    ui::Button split_action("", split_rect.x, split_rect.y, split_rect.w, split_rect.h, true);
    ui::Button esc_action("", esc_rect.x, esc_rect.y, esc_rect.w, esc_rect.h, true);

    bool sort_armed = armed_action_ == InventoryAction::Sort;
    bool use_armed = armed_action_ == InventoryAction::Use;
    bool drop_armed = armed_action_ == InventoryAction::Drop;
    bool split_armed = armed_action_ == InventoryAction::Split;
    bool esc_armed = armed_action_ == InventoryAction::Close && !close_pending_;

    const bool sort_clicked = sort_action.process_pointer(mouse_x, mouse_y, mouse_in_view, mouse_pressed, mouse_released, sort_armed);
    const bool use_clicked = use_action.process_pointer(mouse_x, mouse_y, mouse_in_view, mouse_pressed, mouse_released, use_armed);
    const bool drop_clicked = drop_action.process_pointer(mouse_x, mouse_y, mouse_in_view, mouse_pressed, mouse_released, drop_armed);
    const bool split_clicked = split_action.process_pointer(mouse_x, mouse_y, mouse_in_view, mouse_pressed, mouse_released, split_armed);
    const bool esc_clicked = esc_action.process_pointer(mouse_x, mouse_y, mouse_in_view, mouse_pressed, mouse_released, esc_armed);

    if (sort_clicked) {
        inventory.sort_bag();
        sort_pressed = true;
    } else if (use_clicked) {
        inventory.use_bag_slot(selected_bag_index_);
        use_pressed = true;
    } else if (drop_clicked) {
        inventory.drop_bag_slot(selected_bag_index_);
        drop_pressed = true;
    } else if (split_clicked) {
        inventory.split_bag_slot(selected_bag_index_);
        split_pressed = true;
    } else if (esc_clicked) {
        close_flash_timer_ = kCloseFlashDuration;
        close_pending_ = true;
    }

    if (sort_armed) {
        armed_action_ = InventoryAction::Sort;
    } else if (use_armed) {
        armed_action_ = InventoryAction::Use;
    } else if (drop_armed) {
        armed_action_ = InventoryAction::Drop;
    } else if (split_armed) {
        armed_action_ = InventoryAction::Split;
    } else if (esc_armed) {
        armed_action_ = InventoryAction::Close;
    } else if (!close_pending_) {
        armed_action_ = InventoryAction::None;
    }

    render_action_hint(action_button_skin, sort_rect, "R", tr("action.sort_label", "Sort").c_str(), sort_pressed || sort_armed);
    render_action_hint(action_button_skin, use_rect, "E", tr("action.use_label", "Use").c_str(), use_pressed || use_armed);
    render_action_hint(action_button_skin, drop_rect, "T", tr("action.drop_label", "Drop").c_str(), drop_pressed || drop_armed);
    render_action_hint(action_button_skin, split_rect, "F", tr("action.split_label", "Split").c_str(), split_pressed || split_armed);
    render_action_hint(action_button_skin, esc_rect, "Esc", tr("action.close_label", "Close").c_str(), close_flash_timer_ > 0.0f || esc_armed);
    render_text(small_font_, tr("hint.mode_switch", "F1-F4 switch HUD modes").c_str(), layout_.action_bar_rect().x + 12.0f, layout_.action_bar_rect().y - 24.0f, sub_color);

    if (drag_source_ == DragSource::Bag && drag_index_ >= 0 && drag_index_ < inventory.bag_slot_count()) {
        const InventorySlot& slot = inventory.bag_slots()[drag_index_];
        const InventoryItemDefinition* item = inventory.item_definition(slot.item_index);
        if (item != nullptr) {
            const float drag_x = snap_ui_logical_x(mouse_x - 86.0f, presentation_rect_);
            const float drag_y = snap_ui_logical_y(mouse_y - 40.0f, presentation_rect_);
            ui::Card drag_card(drag_x, drag_y, 172.0f, 78.0f, true);
            char meta[16];
            std::snprintf(meta, sizeof(meta), "%d", slot.quantity);
            drag_card.set_title(item->name.c_str()).set_subtitle(item->subtitle.c_str()).set_meta(meta).set_icon(item->icon.valid() ? &item->icon : nullptr);
            drag_card.render(renderer_, bag_slot_skin, body_font_, small_font_, presentation_rect_, -1000.0f, -1000.0f, false, false, false, title_color, text_color, sub_color, 220);
        }
    } else if (drag_source_ == DragSource::Equipment && drag_index_ >= 0 && drag_index_ < inventory.equipment_slot_count()) {
        const InventorySlot& slot = inventory.equipped_slots()[drag_index_];
        const InventoryItemDefinition* item = inventory.item_definition(slot.item_index);
        if (item != nullptr) {
            const float drag_x = snap_ui_logical_x(mouse_x - 86.0f, presentation_rect_);
            const float drag_y = snap_ui_logical_y(mouse_y - 40.0f, presentation_rect_);
            ui::Card drag_card(drag_x, drag_y, 172.0f, 78.0f, true);
            drag_card.set_title(item->name.c_str()).set_subtitle(item->subtitle.c_str()).set_meta("").set_icon(item->icon.valid() ? &item->icon : nullptr);
            drag_card.render(renderer_, equipment_card_skin, body_font_, small_font_, presentation_rect_, -1000.0f, -1000.0f, false, false, false, title_color, text_color, sub_color, 220);
        }
    } else if (drag_source_ == DragSource::Backpack) {
        const InventoryItemDefinition* item = inventory.item_definition(inventory.equipped_backpack_slot().item_index);
        if (item != nullptr) {
            const float drag_x = snap_ui_logical_x(mouse_x - 86.0f, presentation_rect_);
            const float drag_y = snap_ui_logical_y(mouse_y - 40.0f, presentation_rect_);
            ui::Card drag_card(drag_x, drag_y, 172.0f, 78.0f, true);
            drag_card.set_title(item->name.c_str()).set_subtitle(tr("backpack.dragging", "Backpack").c_str()).set_meta("").set_icon(item->icon.valid() ? &item->icon : nullptr);
            drag_card.render(renderer_, backpack_card_skin, body_font_, small_font_, presentation_rect_, -1000.0f, -1000.0f, false, false, false, title_color, text_color, sub_color, 220);
        }
    }

    render_tooltip(assets, inventory, mouse_x, mouse_y);
    return close_requested;
}

void InventoryScreen::ensure_fonts(float ui_scale)
{
    const int desired_base = std::max(
        static_cast<int>(kHudFontPointSize),
        static_cast<int>(std::round(kHudFontPointSize * ui_scale)));
    if (section_font_ != nullptr &&
        title_font_ != nullptr &&
        body_font_ != nullptr &&
        key_font_ != nullptr &&
        number_font_ != nullptr &&
        desired_base == base_point_size_) {
        return;
    }

    if (section_font_ != nullptr) {
        TTF_CloseFont(section_font_);
        section_font_ = nullptr;
    }
    if (title_font_ != nullptr) {
        TTF_CloseFont(title_font_);
        title_font_ = nullptr;
    }
    if (body_font_ != nullptr) {
        TTF_CloseFont(body_font_);
        body_font_ = nullptr;
    }
    if (small_font_ != nullptr) {
        TTF_CloseFont(small_font_);
        small_font_ = nullptr;
    }
    if (key_font_ != nullptr) {
        TTF_CloseFont(key_font_);
        key_font_ = nullptr;
    }
    if (number_font_ != nullptr) {
        TTF_CloseFont(number_font_);
        number_font_ = nullptr;
    }

    base_point_size_ = desired_base;
    section_font_ = load_section_font(base_point_size_ + 12);
    title_font_ = load_title_font(base_point_size_ + 5);
    body_font_ = load_body_font(base_point_size_ + 3);
    small_font_ = load_body_font(std::max(12, base_point_size_ + 1));
    key_font_ = load_key_font(base_point_size_ + 14);
    number_font_ = load_number_font(base_point_size_ + 8);
}

void InventoryScreen::render_text(TTF_Font* font, const char* text, float x, float y, SDL_Color color) const
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

void InventoryScreen::render_text_centered(TTF_Font* font, const char* text, const SDL_FRect& rect, SDL_Color color, float y_offset) const
{
    if (font == nullptr || text == nullptr || text[0] == '\0') {
        return;
    }
    int text_width = 0;
    int text_height = 0;
    if (!TTF_GetStringSize(font, text, 0, &text_width, &text_height)) {
        return;
    }
    const float x_scale = ui_presentation_scale_x(presentation_rect_);
    const float y_scale = ui_presentation_scale_y(presentation_rect_);
    const float logical_text_width = static_cast<float>(text_width) / x_scale;
    const float logical_text_height = static_cast<float>(text_height) / y_scale;
    render_text(
        font,
        text,
        rect.x + std::floor((rect.w - logical_text_width) * 0.5f),
        rect.y + std::floor((rect.h - logical_text_height) * 0.5f) + y_offset,
        color);
}

void InventoryScreen::render_action_hint(const ControlStyle& skin, const SDL_FRect& rect, const char* key, const char* label, bool highlighted) const
{
    const float button_size = rect.h - 12.0f;
    const SDL_FRect button_rect{rect.x + 6.0f, rect.y + 6.0f, button_size, button_size};
    ui::Button key_button(key, button_rect.x, button_rect.y, button_rect.w, button_rect.h, true);
    key_button.set_style(&skin);
    key_button.set_font(key_font_);
    key_button.set_text_colors(SDL_Color{245, 218, 155, 255}, SDL_Color{245, 218, 155, 255});
    key_button.set_alpha(236);
    key_button.set_visual_override(highlighted, ControlVisualState::Pressed);
    key_button.render(
        renderer_,
        skin,
        key_font_,
        presentation_rect_,
        -1000.0f,
        -1000.0f,
        false,
        false,
        false,
        SDL_Color{245, 218, 155, 255},
        SDL_Color{245, 218, 155, 255},
        236);

    std::unique_ptr<ui::TextItem> label_text(new ui::TextItem());
    label_text->set_rect(rect.x + 6.0f + button_size + 12.0f, rect.y + 6.0f, std::max(0.0f, rect.w - button_size - 24.0f), rect.h - 12.0f);
    label_text->set_font(title_font_);
    label_text->set_text(label);
    label_text->set_color(SDL_Color{218, 200, 174, 255});
    label_text->set_fit_to_bounds(false);
    label_text->set_vertical_align(ui::VerticalAlign::Middle);

    ui::RenderContext context;
    context.renderer = renderer_;
    context.presentation_rect = presentation_rect_;
    context.alpha = 255;
    label_text->render(context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});
}

void InventoryScreen::render_tooltip(const Assets& assets, const InventoryState& inventory, float mouse_x, float mouse_y) const
{
    if (tooltip_source_ == DragSource::None || tooltip_alpha_ <= 0.01f) {
        return;
    }

    const InventorySlot* slot = nullptr;
    if (tooltip_source_ == DragSource::Bag && tooltip_index_ >= 0 && tooltip_index_ < inventory.bag_slot_count()) {
        slot = &inventory.bag_slots()[tooltip_index_];
    } else if (tooltip_source_ == DragSource::Equipment && tooltip_index_ >= 0 && tooltip_index_ < inventory.equipment_slot_count()) {
        slot = &inventory.equipped_slots()[tooltip_index_];
    } else if (tooltip_source_ == DragSource::Backpack) {
        slot = &inventory.equipped_backpack_slot();
    }
    if (slot == nullptr || slot->item_index < 0) {
        return;
    }

    const InventoryItemDefinition* item = inventory.item_definition(slot->item_index);
    if (item == nullptr) {
        return;
    }

    const float tooltip_w = 210.0f;
    const float tooltip_h = 124.0f;
    float x = tooltip_anchor_x_ + 18.0f;
    float y = tooltip_anchor_y_ - 8.0f;
    if (x + tooltip_w > static_cast<float>(kUiDesignWidth) - 12.0f) {
        x = mouse_x - tooltip_w - 18.0f;
    }
    if (y + tooltip_h > static_cast<float>(kUiDesignHeight) - 12.0f) {
        y = static_cast<float>(kUiDesignHeight) - tooltip_h - 12.0f;
    }
    if (y < 12.0f) {
        y = 12.0f;
    }

    const float distance = std::sqrt(squared_distance(mouse_x, mouse_y, tooltip_anchor_x_, tooltip_anchor_y_));
    const_cast<InventoryScreen*>(this)->tooltip_alpha_ = distance > kTooltipFadeDistance
        ? std::max(0.0f, tooltip_alpha_ - (1.0f / 60.0f) * kTooltipFadeSpeed)
        : std::min(1.0f, tooltip_alpha_ + (1.0f / 60.0f) * kTooltipFadeSpeed);
    if (const_cast<InventoryScreen*>(this)->tooltip_alpha_ <= 0.01f) {
        const_cast<InventoryScreen*>(this)->tooltip_source_ = DragSource::None;
        return;
    }

    const Uint8 alpha = static_cast<Uint8>(std::round(246.0f * tooltip_alpha_));
    ui::Panel tooltip_panel(x, y, tooltip_w, tooltip_h);
    tooltip_panel
        .set_title(item->name.c_str())
        .set_style(&assets.panel_skin)
        .set_font(title_font_)
        .set_text_color(SDL_Color{241, 226, 205, alpha})
        .set_alpha(alpha);

    char quantity_text[32];
    char weight_text[32];
    std::snprintf(quantity_text, sizeof(quantity_text), "%d", slot->quantity);
    std::snprintf(weight_text, sizeof(weight_text), "%.1f kg", item->unit_weight * std::max(1, slot->quantity));

    std::unique_ptr<ui::TextItem> desc_label(new ui::TextItem());
    desc_label->set_rect(2.0f, 12.0f, 80.0f, 18.0f);
    desc_label->set_font(small_font_);
    desc_label->set_text(tr("tooltip.desc", "Description").c_str());
    desc_label->set_color(SDL_Color{176, 157, 132, alpha});
    desc_label->set_fit_to_bounds(false);
    tooltip_panel.add_child_to_slot("body", std::move(desc_label));

    std::unique_ptr<ui::TextItem> desc_value(new ui::TextItem());
    desc_value->set_rect(2.0f, 32.0f, tooltip_w - 36.0f, 20.0f);
    desc_value->set_font(body_font_);
    desc_value->set_text(item->subtitle.c_str());
    desc_value->set_color(SDL_Color{218, 200, 174, alpha});
    desc_value->set_fit_to_bounds(false);
    tooltip_panel.add_child_to_slot("body", std::move(desc_value));

    std::unique_ptr<ui::TextItem> qty_label(new ui::TextItem());
    qty_label->set_rect(2.0f, 56.0f, 48.0f, 18.0f);
    qty_label->set_font(small_font_);
    qty_label->set_text(tr("tooltip.qty", "Qty").c_str());
    qty_label->set_color(SDL_Color{176, 157, 132, alpha});
    qty_label->set_fit_to_bounds(false);
    tooltip_panel.add_child_to_slot("body", std::move(qty_label));

    std::unique_ptr<ui::TextItem> qty_value(new ui::TextItem());
    qty_value->set_rect(58.0f, 74.0f, 34.0f, 18.0f);
    qty_value->set_font(number_font_);
    qty_value->set_text(quantity_text);
    qty_value->set_color(SDL_Color{241, 226, 205, alpha});
    qty_value->set_fit_to_bounds(false);
    tooltip_panel.add_child_to_slot("body", std::move(qty_value));

    std::unique_ptr<ui::TextItem> weight_label(new ui::TextItem());
    weight_label->set_rect(96.0f, 56.0f, 68.0f, 18.0f);
    weight_label->set_font(small_font_);
    weight_label->set_text(tr("tooltip.weight", "Weight").c_str());
    weight_label->set_color(SDL_Color{176, 157, 132, alpha});
    weight_label->set_fit_to_bounds(false);
    tooltip_panel.add_child_to_slot("body", std::move(weight_label));

    std::unique_ptr<ui::TextItem> weight_value(new ui::TextItem());
    weight_value->set_rect(96.0f, 74.0f, tooltip_w - 120.0f, 18.0f);
    weight_value->set_font(number_font_);
    weight_value->set_text(weight_text);
    weight_value->set_color(SDL_Color{241, 226, 205, alpha});
    weight_value->set_fit_to_bounds(false);
    tooltip_panel.add_child_to_slot("body", std::move(weight_value));

    ui::RenderContext context;
    context.renderer = renderer_;
    context.presentation_rect = presentation_rect_;
    context.alpha = 255;
    tooltip_panel.render(context, SDL_FRect{0.0f, 0.0f, static_cast<float>(kUiDesignWidth), static_cast<float>(kUiDesignHeight)});
}

const std::string& InventoryScreen::tr(const char* key, const char* fallback) const
{
    return strings_.get(key, fallback);
}

} // namespace zg
