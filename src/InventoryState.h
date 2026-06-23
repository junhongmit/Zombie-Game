#pragma once

#include "Texture.h"

#include <SDL3/SDL.h>

#include <string>
#include <vector>

namespace zg {

struct WeaponDefinition;
struct WeaponState;

struct ResourceStockpile {
    int coins = 350;
    int gems = 0;
    int wood = 126;
    int metal = 78;
    int cloth = 23;
    int electronics = 15;
    int fuel = 10;
    int parts = 32;
    int glass = 18;
    int food = 45;
    int water = 36;
    int medicine = 22;
};

struct InventoryItemDefinition {
    std::string id;
    std::string name;
    std::string subtitle;
    std::string icon_path;
    float unit_weight = 0.0f;
    int backpack_profile_index = -1;
    Texture icon;
};

struct InventorySlot {
    int item_index = -1;
    int quantity = 0;
    bool locked = false;
};

enum class ToolMode {
    Combat = 0,
    Build,
    Blueprint,
    Support,
    Count
};

struct BackpackDefinition {
    std::string name;
    float capacity_kg = 0.0f;
    std::string mobility;
    bool selected = false;
};

enum class EquipmentSlotType {
    Primary = 0,
    Secondary,
    Melee,
    Armor,
    Count
};

class InventoryState {
public:
    static constexpr int kBagSlotCount = 24;
    static constexpr int kQuickSlotCount = 9;

    bool load_demo(SDL_Renderer* renderer);
    void sync_from_weapon_state(const WeaponState& weapon_state);
    void set_current_mode(ToolMode mode);
    void set_current_mode_by_index(int mode_index);
    ToolMode current_mode() const { return current_mode_; }
    int current_mode_index() const { return static_cast<int>(current_mode_); }
    int mode_count() const { return static_cast<int>(ToolMode::Count); }
    bool sort_bag();
    bool use_bag_slot(int index);
    bool drop_bag_slot(int index);
    bool split_bag_slot(int index);
    bool select_backpack(int index);
    bool equip_bag_item_to_slot(int bag_index, int slot_index);
    bool unequip_slot_to_bag(int slot_index);
    bool unequip_slot_to_bag_at(int slot_index, int bag_index);
    bool move_bag_item(int from_index, int to_index);
    bool equip_backpack_from_bag(int bag_index);
    bool unequip_backpack_to_bag(int bag_index);

    const ResourceStockpile& resources() const { return resources_; }
    const InventoryItemDefinition* item_definition(int index) const;
    const InventorySlot* bag_slots() const { return bag_slots_; }
    int bag_slot_count() const { return kBagSlotCount; }
    const InventorySlot* quick_slots() const { return quick_slots_; }
    int quick_slot_count() const { return kQuickSlotCount; }
    const InventorySlot* visible_tool_slots() const;
    int visible_tool_slot_count() const { return kQuickSlotCount; }
    const InventorySlot* equipped_slots() const { return equipped_slots_; }
    int equipment_slot_count() const { return static_cast<int>(EquipmentSlotType::Count); }
    const InventorySlot& equipped_backpack_slot() const { return equipped_backpack_slot_; }
    const BackpackDefinition* equipped_backpack_definition() const;
    const BackpackDefinition* backpack_definitions() const { return backpacks_.empty() ? nullptr : &backpacks_[0]; }
    int backpack_definition_count() const { return static_cast<int>(backpacks_.size()); }
    int bag_used_slots() const;
    int bag_capacity() const { return current_bag_capacity_; }
    float current_weight() const;
    float max_weight() const;
    float remaining_weight() const;

private:
    int add_item_definition(SDL_Renderer* renderer, const char* id, const char* name, const char* subtitle, const char* icon_path, float unit_weight);
    void set_bag_slot(int index, int item_index, int quantity, bool locked = false);
    void set_quick_slot(int index, int item_index, int quantity, bool locked = false);
    void compact_bag();
    int first_empty_bag_slot() const;
    bool can_equip_item_to_slot(int item_index, int slot_index) const;
    int max_stack_for_item(int item_index) const;
    bool insert_bag_slot(int from_index, int to_index);
    void refresh_carry_stats();

    ResourceStockpile resources_{};
    std::vector<InventoryItemDefinition> item_definitions_;
    std::vector<BackpackDefinition> backpacks_;
    InventorySlot bag_slots_[kBagSlotCount] = {};
    InventorySlot quick_slots_[kQuickSlotCount] = {};
    InventorySlot build_slots_[kQuickSlotCount] = {};
    InventorySlot blueprint_slots_[kQuickSlotCount] = {};
    InventorySlot support_slots_[kQuickSlotCount] = {};
    InventorySlot equipped_slots_[static_cast<int>(EquipmentSlotType::Count)] = {};
    InventorySlot equipped_backpack_slot_{};
    ToolMode current_mode_ = ToolMode::Combat;
    int current_bag_capacity_ = 5;
    float current_max_weight_ = 12.0f;
};

} // namespace zg
