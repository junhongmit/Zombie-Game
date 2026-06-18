#include "InventoryState.h"

#include "Weapon.h"

#include <algorithm>

namespace zg {

namespace {

InventorySlot make_slot(int item_index, int quantity, bool locked)
{
    InventorySlot slot;
    slot.item_index = item_index;
    slot.quantity = quantity;
    slot.locked = locked;
    return slot;
}

} // namespace

bool InventoryState::load_demo(SDL_Renderer* renderer)
{
    item_definitions_.clear();
    backpacks_.clear();
    for (int i = 0; i < kBagSlotCount; ++i) {
        bag_slots_[i] = InventorySlot{};
    }
    for (int i = 0; i < kQuickSlotCount; ++i) {
        quick_slots_[i] = InventorySlot{};
        build_slots_[i] = InventorySlot{};
        blueprint_slots_[i] = InventorySlot{};
        support_slots_[i] = InventorySlot{};
    }

    const int medkit = add_item_definition(renderer, "medkit", "Medkit", "Medical", "assets/ui/icons/medkit.png", 1.4f);
    const int bandage = add_item_definition(renderer, "bandage", "Bandage", "Medical", "assets/ui/icons/bandage.png", 0.2f);
    const int pills = add_item_definition(renderer, "pills", "Pills", "Consumable", "assets/ui/icons/pills.png", 0.1f);
    const int syringe = add_item_definition(renderer, "syringe", "Injector", "Medical", "assets/ui/icons/syringe.png", 0.2f);
    const int ammo_box = add_item_definition(renderer, "ammo_box", "Ammo Box", "7.62mm", "assets/ui/icons/ammo_box.png", 3.0f);
    const int crate = add_item_definition(renderer, "crate", "Supply Crate", "Storage", "assets/ui/icons/crate.png", 5.5f);
    const int rifle_rounds = add_item_definition(renderer, "rifle_rounds", "Rifle Rounds", "5.56mm", "assets/ui/icons/rifle_rounds.png", 1.7f);
    const int wrench = add_item_definition(renderer, "wrench", "Wrench", "Tool", "assets/ui/icons/wrench.png", 1.0f);
    const int flashlight = add_item_definition(renderer, "flashlight", "Flashlight", "Utility", "assets/ui/icons/flashlight.png", 0.4f);
    const int lens = add_item_definition(renderer, "lens", "Optic Lens", "Parts", "assets/ui/icons/lens.png", 0.4f);
    const int beans = add_item_definition(renderer, "beans", "Beans", "Food", "assets/ui/icons/beans.png", 0.6f);
    const int water = add_item_definition(renderer, "water", "Water", "Drink", "assets/ui/icons/water.png", 1.0f);
    const int snack = add_item_definition(renderer, "snack", "Ration Bar", "Food", "assets/ui/icons/snack.png", 0.2f);
    const int fuel = add_item_definition(renderer, "fuel", "Fuel Can", "Fuel", "assets/ui/icons/fuel.png", 4.0f);
    const int manual = add_item_definition(renderer, "manual", "Field Guide", "Document", "assets/ui/icons/manual.png", 0.7f);
    const int radio = add_item_definition(renderer, "radio", "Hand Radio", "Utility", "assets/ui/icons/radio.png", 0.5f);
    const int backpack = add_item_definition(renderer, "pack", "Backpack", "Gear", "assets/ui/icons/backpack.png", 1.2f);
    const int bottle = add_item_definition(renderer, "bottle", "Molotov", "Utility", "assets/ui/icons/bottle.png", 0.9f);
    const int hammer = add_item_definition(renderer, "hammer", "Hammer", "Build", "assets/ui/icons/hammer.png", 1.0f);
    const int planks = add_item_definition(renderer, "planks", "Boards", "Build", "assets/ui/icons/planks.png", 2.0f);
    const int barricade = add_item_definition(renderer, "barricade", "Barricade", "Blueprint", "assets/ui/icons/barricade.png", 0.0f);
    const int trap = add_item_definition(renderer, "trap", "Spike Trap", "Blueprint", "assets/ui/icons/trap.png", 0.0f);
    const int flare = add_item_definition(renderer, "flare", "Flare", "Support", "assets/ui/icons/flare.png", 0.3f);
    const int toolkit = add_item_definition(renderer, "toolkit", "Toolkit", "Support", "assets/ui/icons/toolkit.png", 2.5f);

    set_bag_slot(0, medkit, 3);
    set_bag_slot(1, bandage, 2);
    set_bag_slot(2, pills, 1);
    set_bag_slot(3, syringe, 1);
    set_bag_slot(4, ammo_box, 50);
    set_bag_slot(5, crate, 30);
    set_bag_slot(6, ammo_box, 60);
    set_bag_slot(7, rifle_rounds, 120);
    set_bag_slot(8, rifle_rounds, 60);
    set_bag_slot(9, wrench, 1);
    set_bag_slot(10, flashlight, 1);
    set_bag_slot(11, lens, 1);
    set_bag_slot(12, beans, 2);
    set_bag_slot(13, water, 1);
    set_bag_slot(14, snack, 2);
    set_bag_slot(15, fuel, 1);
    set_bag_slot(16, manual, 1);
    set_bag_slot(17, radio, 1);

    set_quick_slot(2, backpack, 1);
    set_quick_slot(3, medkit, 3);
    set_quick_slot(4, wrench, 1);
    set_quick_slot(5, manual, 2);
    set_quick_slot(6, bottle, 4);
    set_quick_slot(7, -1, 0, true);
    set_quick_slot(8, -1, 0, true);

    build_slots_[0] = make_slot(hammer, 1, false);
    build_slots_[1] = make_slot(planks, 12, false);
    build_slots_[2] = make_slot(wrench, 1, false);
    build_slots_[3] = make_slot(crate, 2, false);
    build_slots_[4] = make_slot(-1, 0, true);
    build_slots_[5] = make_slot(-1, 0, true);
    build_slots_[6] = make_slot(-1, 0, true);
    build_slots_[7] = make_slot(-1, 0, true);
    build_slots_[8] = make_slot(-1, 0, true);

    blueprint_slots_[0] = make_slot(barricade, 1, false);
    blueprint_slots_[1] = make_slot(trap, 1, false);
    blueprint_slots_[2] = make_slot(crate, 1, false);
    blueprint_slots_[3] = make_slot(-1, 0, true);
    blueprint_slots_[4] = make_slot(-1, 0, true);
    blueprint_slots_[5] = make_slot(-1, 0, true);
    blueprint_slots_[6] = make_slot(-1, 0, true);
    blueprint_slots_[7] = make_slot(-1, 0, true);
    blueprint_slots_[8] = make_slot(-1, 0, true);

    support_slots_[0] = make_slot(medkit, 3, false);
    support_slots_[1] = make_slot(water, 1, false);
    support_slots_[2] = make_slot(beans, 2, false);
    support_slots_[3] = make_slot(radio, 1, false);
    support_slots_[4] = make_slot(flare, 2, false);
    support_slots_[5] = make_slot(toolkit, 1, false);
    support_slots_[6] = make_slot(-1, 0, true);
    support_slots_[7] = make_slot(-1, 0, true);
    support_slots_[8] = make_slot(-1, 0, true);

    BackpackDefinition bag;
    bag.name = "Field Pack";
    bag.capacity_kg = 40.0f;
    bag.mobility = "Normal";
    bag.selected = true;
    backpacks_.push_back(bag);

    bag = BackpackDefinition{};
    bag.name = "Climber Pack";
    bag.capacity_kg = 60.0f;
    bag.mobility = "Normal";
    backpacks_.push_back(bag);

    bag = BackpackDefinition{};
    bag.name = "Military Pack";
    bag.capacity_kg = 80.0f;
    bag.mobility = "Slower";
    backpacks_.push_back(bag);

    bag = BackpackDefinition{};
    bag.name = "Cart";
    bag.capacity_kg = 120.0f;
    bag.mobility = "Slow";
    backpacks_.push_back(bag);

    bag = BackpackDefinition{};
    bag.name = "Hand Truck";
    bag.capacity_kg = 150.0f;
    bag.mobility = "Very Slow";
    backpacks_.push_back(bag);

    current_bag_capacity_ = 30;
    current_mode_ = ToolMode::Combat;
    return true;
}

void InventoryState::sync_from_weapon_state(const WeaponState& weapon_state)
{
    for (int i = 0; i < 2; ++i) {
        quick_slots_[i] = InventorySlot{};
    }

    const WeaponSlot* weapon_slots = weapon_state.slots();
    for (int i = 0; i < std::min(2, weapon_state.slot_count()); ++i) {
        if (weapon_slots[i].definition != nullptr) {
            const int item_index = add_item_definition(
                nullptr,
                weapon_slots[i].definition->name.c_str(),
                weapon_slots[i].definition->name.c_str(),
                weapon_slots[i].definition->full_auto ? "Auto" : "Semi",
                weapon_slots[i].definition->preview_image_path.c_str(),
                3.0f);
            quick_slots_[i].item_index = item_index;
            quick_slots_[i].quantity = weapon_slots[i].ammo_in_mag;
            quick_slots_[i].locked = false;
        }
    }
}

const InventoryItemDefinition* InventoryState::item_definition(int index) const
{
    if (index < 0 || index >= static_cast<int>(item_definitions_.size())) {
        return nullptr;
    }
    return &item_definitions_[index];
}

void InventoryState::set_current_mode(ToolMode mode)
{
    current_mode_ = mode;
}

void InventoryState::set_current_mode_by_index(int mode_index)
{
    if (mode_index < 0 || mode_index >= mode_count()) {
        return;
    }
    current_mode_ = static_cast<ToolMode>(mode_index);
}

const InventorySlot* InventoryState::visible_tool_slots() const
{
    switch (current_mode_) {
    case ToolMode::Build:
        return build_slots_;
    case ToolMode::Blueprint:
        return blueprint_slots_;
    case ToolMode::Support:
        return support_slots_;
    case ToolMode::Combat:
    case ToolMode::Count:
    default:
        return quick_slots_;
    }
}

int InventoryState::bag_used_slots() const
{
    int count = 0;
    for (int i = 0; i < kBagSlotCount; ++i) {
        if (bag_slots_[i].item_index >= 0) {
            ++count;
        }
    }
    return count;
}

float InventoryState::current_weight() const
{
    float total = 0.0f;
    for (int i = 0; i < kBagSlotCount; ++i) {
        const InventorySlot& slot = bag_slots_[i];
        const InventoryItemDefinition* definition = item_definition(slot.item_index);
        if (definition != nullptr) {
            total += definition->unit_weight * slot.quantity;
        }
    }
    return total;
}

float InventoryState::max_weight() const
{
    if (backpacks_.empty()) {
        return 40.0f;
    }
    for (size_t i = 0; i < backpacks_.size(); ++i) {
        if (backpacks_[i].selected) {
            return backpacks_[i].capacity_kg;
        }
    }
    return backpacks_[0].capacity_kg;
}

float InventoryState::remaining_weight() const
{
    return std::max(0.0f, max_weight() - current_weight());
}

int InventoryState::add_item_definition(SDL_Renderer* renderer, const char* id, const char* name, const char* subtitle, const char* icon_path, float unit_weight)
{
    for (size_t i = 0; i < item_definitions_.size(); ++i) {
        if (item_definitions_[i].id == (id != nullptr ? id : "")) {
            return static_cast<int>(i);
        }
    }

    InventoryItemDefinition definition;
    definition.id = id != nullptr ? id : "";
    definition.name = name != nullptr ? name : "";
    definition.subtitle = subtitle != nullptr ? subtitle : "";
    definition.icon_path = icon_path != nullptr ? icon_path : "";
    definition.unit_weight = unit_weight;
    if (renderer != nullptr && !definition.icon_path.empty()) {
        definition.icon.load(renderer, definition.icon_path.c_str(), true);
    }

    item_definitions_.push_back(std::move(definition));
    return static_cast<int>(item_definitions_.size()) - 1;
}

void InventoryState::set_bag_slot(int index, int item_index, int quantity, bool locked)
{
    if (index < 0 || index >= kBagSlotCount) {
        return;
    }
    bag_slots_[index].item_index = item_index;
    bag_slots_[index].quantity = quantity;
    bag_slots_[index].locked = locked;
}

void InventoryState::set_quick_slot(int index, int item_index, int quantity, bool locked)
{
    if (index < 0 || index >= kQuickSlotCount) {
        return;
    }
    quick_slots_[index].item_index = item_index;
    quick_slots_[index].quantity = quantity;
    quick_slots_[index].locked = locked;
}

} // namespace zg
