#include "InventoryState.h"

#include "AssetPaths.h"

#include <SDL3/SDL_filesystem.h>

#include "Weapon.h"

#include <algorithm>
#include <string>

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

bool asset_file_exists(const std::string& path)
{
    if (path.empty()) {
        return false;
    }
    SDL_PathInfo info;
    const std::string resolved = resolve_asset_path(path.c_str());
    return SDL_GetPathInfo(resolved.c_str(), &info) && info.type == SDL_PATHTYPE_FILE;
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
    const int field_pack = add_item_definition(renderer, "field_pack", "Field Pack", "Backpack", "assets/ui/icons/backpack.png", 1.2f);
    const int climber_pack = add_item_definition(renderer, "climber_pack", "Climber Pack", "Backpack", "assets/ui/icons/backpack.png", 1.6f);
    const int military_pack = add_item_definition(renderer, "military_pack", "Military Pack", "Backpack", "assets/ui/icons/backpack.png", 2.0f);
    const int bottle = add_item_definition(renderer, "bottle", "Molotov", "Utility", "assets/ui/icons/bottle.png", 0.9f);
    const int hammer = add_item_definition(renderer, "hammer", "Hammer", "Build", "assets/ui/icons/hammer.png", 1.0f);
    const int planks = add_item_definition(renderer, "planks", "Boards", "Build", "assets/ui/icons/planks.png", 2.0f);
    const int barricade = add_item_definition(renderer, "barricade", "Barricade", "Blueprint", "assets/ui/icons/barricade.png", 0.0f);
    const int trap = add_item_definition(renderer, "trap", "Spike Trap", "Blueprint", "assets/ui/icons/trap.png", 0.0f);
    const int flare = add_item_definition(renderer, "flare", "Flare", "Support", "assets/ui/icons/flare.png", 0.3f);
    const int toolkit = add_item_definition(renderer, "toolkit", "Toolkit", "Support", "assets/ui/icons/toolkit.png", 2.5f);
    const int bat = add_item_definition(renderer, "bat", "Baseball Bat", "Melee", "", 1.6f);
    const int jacket = add_item_definition(renderer, "jacket", "Leather Jacket", "Armor", "", 3.4f);

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
    set_bag_slot(18, bat, 1);
    set_bag_slot(19, jacket, 1);

    item_definitions_[field_pack].backpack_profile_index = 0;
    item_definitions_[climber_pack].backpack_profile_index = 1;
    item_definitions_[military_pack].backpack_profile_index = 2;

    set_bag_slot(20, field_pack, 1);
    set_bag_slot(21, climber_pack, 1);
    set_bag_slot(22, military_pack, 1);

    set_quick_slot(2, field_pack, 1);
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

    refresh_carry_stats();
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
                weapon_slots[i].definition->icon_image_path.empty()
                    ? weapon_slots[i].definition->preview_image_path.c_str()
                    : weapon_slots[i].definition->icon_image_path.c_str(),
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

bool InventoryState::sort_bag()
{
    std::vector<InventorySlot> filled;
    filled.reserve(kBagSlotCount);
    for (int i = 0; i < kBagSlotCount; ++i) {
        if (bag_slots_[i].item_index >= 0 && !bag_slots_[i].locked) {
            filled.push_back(bag_slots_[i]);
        }
    }

    std::sort(filled.begin(), filled.end(), [this](const InventorySlot& a, const InventorySlot& b) {
        const InventoryItemDefinition* item_a = item_definition(a.item_index);
        const InventoryItemDefinition* item_b = item_definition(b.item_index);
        const std::string name_a = item_a != nullptr ? item_a->name : "";
        const std::string name_b = item_b != nullptr ? item_b->name : "";
        if (name_a == name_b) {
            return a.quantity > b.quantity;
        }
        return name_a < name_b;
    });

    int write_index = 0;
    for (size_t i = 0; i < filled.size(); ++i) {
        bag_slots_[write_index++] = filled[i];
    }
    while (write_index < kBagSlotCount) {
        bag_slots_[write_index++] = InventorySlot{};
    }
    return true;
}

bool InventoryState::use_bag_slot(int index)
{
    if (index < 0 || index >= kBagSlotCount) {
        return false;
    }
    InventorySlot& slot = bag_slots_[index];
    if (slot.item_index < 0 || slot.locked) {
        return false;
    }

    const InventoryItemDefinition* item = item_definition(slot.item_index);
    if (item == nullptr) {
        return false;
    }

    bool consumed = false;
    if (item->id == "medkit" || item->id == "bandage" || item->id == "pills" || item->id == "syringe" ||
        item->id == "beans" || item->id == "water" || item->id == "snack" || item->id == "flare" || item->id == "bottle") {
        consumed = true;
    }

    if (consumed && slot.quantity > 0) {
        --slot.quantity;
        if (slot.quantity <= 0) {
            slot = InventorySlot{};
        }
        compact_bag();
    }
    return true;
}

bool InventoryState::drop_bag_slot(int index)
{
    if (index < 0 || index >= kBagSlotCount) {
        return false;
    }
    InventorySlot& slot = bag_slots_[index];
    if (slot.item_index < 0 || slot.locked) {
        return false;
    }
    if (slot.quantity > 1) {
        --slot.quantity;
    } else {
        slot = InventorySlot{};
        compact_bag();
    }
    return true;
}

bool InventoryState::split_bag_slot(int index)
{
    if (index < 0 || index >= kBagSlotCount) {
        return false;
    }
    InventorySlot& slot = bag_slots_[index];
    if (slot.item_index < 0 || slot.locked || slot.quantity < 2) {
        return false;
    }

    int empty_index = -1;
    for (int i = 0; i < kBagSlotCount; ++i) {
        if (bag_slots_[i].item_index < 0) {
            empty_index = i;
            break;
        }
    }
    if (empty_index < 0) {
        return false;
    }

    const int moved = slot.quantity / 2;
    slot.quantity -= moved;
    bag_slots_[empty_index] = make_slot(slot.item_index, moved, false);
    return true;
}

bool InventoryState::select_backpack(int index)
{
    if (index < 0 || index >= static_cast<int>(backpacks_.size())) {
        return false;
    }
    for (size_t i = 0; i < backpacks_.size(); ++i) {
        backpacks_[i].selected = false;
    }
    backpacks_[index].selected = true;
    return true;
}

bool InventoryState::equip_bag_item_to_slot(int bag_index, int slot_index)
{
    if (bag_index < 0 || bag_index >= kBagSlotCount) {
        return false;
    }
    if (slot_index < 0 || slot_index >= equipment_slot_count()) {
        return false;
    }

    InventorySlot& bag_slot = bag_slots_[bag_index];
    if (bag_slot.item_index < 0 || bag_slot.locked || !can_equip_item_to_slot(bag_slot.item_index, slot_index)) {
        return false;
    }

    InventorySlot previous_equipped = equipped_slots_[slot_index];
    InventorySlot equipped_item = make_slot(bag_slot.item_index, 1, false);
    equipped_slots_[slot_index] = equipped_item;

    if (bag_slot.quantity > 1) {
        --bag_slot.quantity;
    } else {
        bag_slot = InventorySlot{};
    }

    if (previous_equipped.item_index >= 0) {
        const int return_index = bag_slot.item_index < 0 ? bag_index : first_empty_bag_slot();
        if (return_index >= 0) {
            bag_slots_[return_index] = previous_equipped;
        }
    }

    compact_bag();
    return true;
}

bool InventoryState::unequip_slot_to_bag(int slot_index)
{
    if (slot_index < 0 || slot_index >= equipment_slot_count()) {
        return false;
    }

    InventorySlot& equipped = equipped_slots_[slot_index];
    if (equipped.item_index < 0) {
        return false;
    }

    const int empty_index = first_empty_bag_slot();
    if (empty_index < 0) {
        return false;
    }

    bag_slots_[empty_index] = equipped;
    equipped = InventorySlot{};
    compact_bag();
    return true;
}

bool InventoryState::unequip_slot_to_bag_at(int slot_index, int bag_index)
{
    if (slot_index < 0 || slot_index >= equipment_slot_count()) {
        return false;
    }
    if (bag_index < 0 || bag_index >= kBagSlotCount) {
        return false;
    }

    InventorySlot& equipped = equipped_slots_[slot_index];
    if (equipped.item_index < 0) {
        return false;
    }

    if (bag_slots_[bag_index].item_index < 0) {
        bag_slots_[bag_index] = equipped;
        equipped = InventorySlot{};
        return true;
    }

    const int empty_index = first_empty_bag_slot();
    if (empty_index < 0) {
        return false;
    }

    bag_slots_[empty_index] = bag_slots_[bag_index];
    bag_slots_[bag_index] = equipped;
    equipped = InventorySlot{};
    return true;
}

bool InventoryState::move_bag_item(int from_index, int to_index)
{
    if (from_index < 0 || from_index >= kBagSlotCount || to_index < 0 || to_index >= kBagSlotCount || from_index == to_index) {
        return false;
    }

    InventorySlot& from = bag_slots_[from_index];
    InventorySlot& to = bag_slots_[to_index];
    if (from.item_index < 0 || from.locked) {
        return false;
    }

    if (to.item_index >= 0 && to.item_index == from.item_index && !to.locked) {
        const int max_stack = max_stack_for_item(from.item_index);
        if (max_stack > 1 && to.quantity < max_stack) {
            const int transferable = std::min(from.quantity, max_stack - to.quantity);
            to.quantity += transferable;
            from.quantity -= transferable;
            if (from.quantity <= 0) {
                from = InventorySlot{};
                compact_bag();
            }
            return transferable > 0;
        }
    }

    return insert_bag_slot(from_index, to_index);
}

bool InventoryState::equip_backpack_from_bag(int bag_index)
{
    if (bag_index < 0 || bag_index >= kBagSlotCount) {
        return false;
    }

    InventorySlot& bag_slot = bag_slots_[bag_index];
    const InventoryItemDefinition* item = item_definition(bag_slot.item_index);
    if (item == nullptr || item->backpack_profile_index < 0 || bag_slot.locked) {
        return false;
    }

    InventorySlot previous = equipped_backpack_slot_;
    equipped_backpack_slot_ = make_slot(bag_slot.item_index, 1, false);
    if (bag_slot.quantity > 1) {
        --bag_slot.quantity;
    } else {
        bag_slot = InventorySlot{};
    }

    if (previous.item_index >= 0) {
        const int empty_index = bag_slot.item_index < 0 ? bag_index : first_empty_bag_slot();
        if (empty_index >= 0) {
            bag_slots_[empty_index] = previous;
        }
    }

    refresh_carry_stats();
    compact_bag();
    return true;
}

bool InventoryState::unequip_backpack_to_bag(int bag_index)
{
    if (bag_index < 0 || bag_index >= kBagSlotCount) {
        return false;
    }
    if (equipped_backpack_slot_.item_index < 0) {
        return false;
    }

    if (bag_slots_[bag_index].item_index < 0) {
        bag_slots_[bag_index] = equipped_backpack_slot_;
    } else {
        const int empty_index = first_empty_bag_slot();
        if (empty_index < 0) {
            return false;
        }
        bag_slots_[empty_index] = bag_slots_[bag_index];
        bag_slots_[bag_index] = equipped_backpack_slot_;
    }

    equipped_backpack_slot_ = InventorySlot{};
    refresh_carry_stats();
    return true;
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
    return current_max_weight_;
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
    if (renderer != nullptr && !definition.icon_path.empty() && asset_file_exists(definition.icon_path)) {
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

void InventoryState::compact_bag()
{
    int write_index = 0;
    for (int i = 0; i < kBagSlotCount; ++i) {
        if (bag_slots_[i].item_index >= 0) {
            if (write_index != i) {
                bag_slots_[write_index] = bag_slots_[i];
                bag_slots_[i] = InventorySlot{};
            }
            ++write_index;
        }
    }
    while (write_index < kBagSlotCount) {
        bag_slots_[write_index++] = InventorySlot{};
    }
}

int InventoryState::first_empty_bag_slot() const
{
    for (int i = 0; i < kBagSlotCount; ++i) {
        if (bag_slots_[i].item_index < 0) {
            return i;
        }
    }
    return -1;
}

bool InventoryState::can_equip_item_to_slot(int item_index, int slot_index) const
{
    const InventoryItemDefinition* item = item_definition(item_index);
    if (item == nullptr) {
        return false;
    }

    switch (static_cast<EquipmentSlotType>(slot_index)) {
    case EquipmentSlotType::Melee:
        return item->subtitle == "Melee" || item->id == "bat";
    case EquipmentSlotType::Armor:
        return item->subtitle == "Armor" || item->id == "jacket";
    case EquipmentSlotType::Primary:
    case EquipmentSlotType::Secondary:
    case EquipmentSlotType::Count:
    default:
        return false;
    }
}

int InventoryState::max_stack_for_item(int item_index) const
{
    const InventoryItemDefinition* item = item_definition(item_index);
    if (item == nullptr) {
        return 1;
    }

    if (item->id == "ammo_box" || item->id == "rifle_rounds") {
        return 999;
    }
    if (item->subtitle == "Medical" || item->subtitle == "Consumable" || item->subtitle == "Food" ||
        item->subtitle == "Drink" || item->subtitle == "Fuel" || item->subtitle == "Build" ||
        item->subtitle == "Support" || item->subtitle == "Parts") {
        return 99;
    }
    return 1;
}

bool InventoryState::insert_bag_slot(int from_index, int to_index)
{
    if (from_index < 0 || from_index >= kBagSlotCount || to_index < 0 || to_index >= kBagSlotCount || from_index == to_index) {
        return false;
    }

    InventorySlot moving = bag_slots_[from_index];
    if (moving.item_index < 0 || moving.locked) {
        return false;
    }

    if (from_index < to_index) {
        for (int i = from_index; i < to_index; ++i) {
            bag_slots_[i] = bag_slots_[i + 1];
        }
    } else {
        for (int i = from_index; i > to_index; --i) {
            bag_slots_[i] = bag_slots_[i - 1];
        }
    }
    bag_slots_[to_index] = moving;
    return true;
}

const BackpackDefinition* InventoryState::equipped_backpack_definition() const
{
    const InventoryItemDefinition* item = item_definition(equipped_backpack_slot_.item_index);
    if (item == nullptr || item->backpack_profile_index < 0 || item->backpack_profile_index >= static_cast<int>(backpacks_.size())) {
        return nullptr;
    }
    return &backpacks_[static_cast<size_t>(item->backpack_profile_index)];
}

void InventoryState::refresh_carry_stats()
{
    current_bag_capacity_ = 5;
    current_max_weight_ = 12.0f;
    for (size_t i = 0; i < backpacks_.size(); ++i) {
        backpacks_[i].selected = false;
    }

    const BackpackDefinition* equipped = equipped_backpack_definition();
    if (equipped == nullptr) {
        return;
    }

    const InventoryItemDefinition* item = item_definition(equipped_backpack_slot_.item_index);
    if (item == nullptr || item->backpack_profile_index < 0 || item->backpack_profile_index >= static_cast<int>(backpacks_.size())) {
        return;
    }

    backpacks_[static_cast<size_t>(item->backpack_profile_index)].selected = true;
    current_bag_capacity_ = 5 + static_cast<int>(equipped->capacity_kg / 4.0f);
    current_max_weight_ = equipped->capacity_kg;
}

} // namespace zg
