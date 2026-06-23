#pragma once

#include "LayoutTypes.h"

#include <SDL3/SDL.h>

namespace zg {

class InventoryLayout {
public:
    bool load(const char* asset_path);

    SDL_FRect equipment_panel_rect() const;
    const std::string& equipment_panel_skin() const { return equipment_panel_.skin; }
    SDL_FRect bag_panel_rect() const;
    const std::string& bag_panel_skin() const { return bag_panel_.skin; }
    SDL_FRect resources_panel_rect() const;
    const std::string& resources_panel_skin() const { return resources_panel_.skin; }
    SDL_FRect weight_panel_rect() const;
    const std::string& weight_panel_skin() const { return weight_panel_.skin; }
    SDL_FRect backpack_panel_rect() const;
    const std::string& backpack_panel_skin() const { return backpack_panel_.skin; }
    SDL_FRect action_bar_rect() const;
    const std::string& action_bar_skin() const { return action_bar_.skin; }
    const std::string& equipment_card_skin() const { return equipment_card_skin_; }
    const std::string& bag_slot_skin() const { return bag_slot_skin_; }
    const std::string& backpack_card_skin() const { return backpack_card_skin_; }
    const std::string& action_button_skin() const { return action_button_skin_; }
    const std::string& close_button_skin() const { return close_button_skin_; }
    const std::string& paperdoll_slot_skin() const { return paperdoll_slot_skin_; }
    SDL_FRect equipment_entry_rect(int index) const;
    SDL_FRect paperdoll_rect() const;
    SDL_FRect paperdoll_slot_rect(int index) const;
    SDL_FRect bag_slot_rect(int index) const;
    SDL_FRect backpack_option_rect(int index) const;
    SDL_FRect action_hint_rect(int index) const;
    SDL_FRect close_button_rect() const;
    SDL_FRect page_bounds_rect() const;

private:
    SDL_FRect to_logical_rect(const NormalizedRect& rect) const;
    float normalized_x(float value) const;
    float normalized_y(float value) const;

    LayoutPanelNode equipment_panel_{NormalizedRect{0.123958f, 0.125f, 0.246875f, 0.56f}, "panel_square_bronze"};
    LayoutPanelNode bag_panel_{NormalizedRect{0.369792f, 0.125f, 0.335417f, 0.56f}, "panel_square_bronze"};
    LayoutPanelNode resources_panel_{NormalizedRect{0.708333f, 0.125f, 0.177083f, 0.416667f}, "panel_square_bronze"};
    LayoutPanelNode weight_panel_{NormalizedRect{0.708333f, 0.553704f, 0.177083f, 0.185185f}, "panel_square_bronze"};
    LayoutPanelNode backpack_panel_{NormalizedRect{0.2500f, 0.690741f, 0.455208f, 0.192593f}, "panel_square_bronze"};
    LayoutPanelNode action_bar_{NormalizedRect{0.2500f, 0.893519f, 0.455208f, 0.053704f}, "panel_square_bronze"};
    std::string equipment_card_skin_ = "card1_weapon_row";
    std::string bag_slot_skin_ = "card2_inventory_slot";
    std::string backpack_card_skin_ = "card2_inventory_slot";
    std::string action_button_skin_ = "button_square_bronze";
    std::string close_button_skin_ = "button_square_bronze";
    std::string paperdoll_slot_skin_ = "button_square_bronze";
    NormalizedRect equipment_entry_start_{0.133333f, 0.162963f, 0.104167f, 0.12037f};
    float equipment_entry_gap_ = 0.009259f;
    int equipment_entry_count_ = 4;
    NormalizedRect paperdoll_rect_{0.252083f, 0.205556f, 0.09375f, 0.283333f};
    NormalizedRect paperdoll_slot_start_{0.328125f, 0.190741f, 0.036458f, 0.057407f};
    float paperdoll_slot_gap_ = 0.014815f;
    int paperdoll_slot_count_ = 4;
    NormalizedRect bag_slot_start_{0.383333f, 0.162963f, 0.05f, 0.083333f};
    float bag_slot_gap_x_ = 0.004167f;
    float bag_slot_gap_y_ = 0.007407f;
    int bag_slot_columns_ = 6;
    int bag_slot_rows_ = 4;
    NormalizedRect backpack_option_start_{0.260417f, 0.512963f, 0.078125f, 0.146296f};
    float backpack_option_gap_ = 0.010417f;
    int backpack_option_count_ = 1;
    NormalizedRect action_hint_start_{0.2625f, 0.901852f, 0.079167f, 0.040741f};
    NormalizedRect close_button_{0.664583f, 0.127778f, 0.028125f, 0.05f};
    NormalizedRect page_bounds_{0.123958f, 0.125f, 0.761458f, 0.822222f};
    float action_hint_gap_ = 0.010417f;
    int action_hint_count_ = 5;
};

} // namespace zg
