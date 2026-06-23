#pragma once

#include "LayoutTypes.h"

#include <SDL3/SDL.h>
#include <string>

namespace zg {

struct WeaponRowTemplateLayout {
    NormalizedRect icon{0.05f, 0.12f, 0.34f, 0.76f};
    NormalizedRect title{0.42f, 0.18f, 0.42f, 0.26f};
    NormalizedRect subtitle{0.42f, 0.52f, 0.34f, 0.18f};
    float title_scale = 1.0f;
    float subtitle_scale = 1.0f;
    float title_y_offset = 0.0f;
    float subtitle_y_offset = 0.0f;
};

class WorkbenchLayout {
public:
    bool load(const char* asset_path);

    SDL_FRect resource_strip_rect() const;
    SDL_FRect close_button_rect() const;
    SDL_FRect weapon_list_panel_rect() const;
    const std::string& weapon_list_panel_skin() const { return weapon_list_.skin; }
    SDL_FRect weapon_list_title_rect() const;
    SDL_FRect weapon_list_view_rect() const;
    const std::string& weapon_list_title_text() const { return weapon_list_.title.text; }
    SDL_FRect preview_panel_rect() const;
    const std::string& preview_panel_skin() const { return preview_panel_.skin; }
    SDL_FRect preview_nameplate_rect() const;
    SDL_FRect stats_panel_rect() const;
    const std::string& stats_panel_skin() const { return stats_panel_.skin; }
    SDL_FRect effects_panel_rect() const;
    const std::string& effects_panel_skin() const { return effects_panel_.skin; }
    SDL_FRect upgrade_panel_rect() const;
    const std::string& upgrade_panel_skin() const { return upgrade_panel_.skin; }
    SDL_FRect attachments_panel_rect() const;
    const std::string& attachments_panel_skin() const { return attachments_.skin; }
    SDL_FRect attachments_title_rect() const;
    SDL_FRect attachments_view_rect() const;
    const std::string& attachments_title_text() const { return attachments_.title.text; }
    SDL_FRect parts_panel_rect() const;
    const std::string& parts_panel_skin() const { return parts_panel_.skin; }

    SDL_FRect weapon_row_rect(const SDL_FRect& viewport_rect, int index, float scroll_offset) const;
    float weapon_row_height() const;
    float weapon_row_pitch() const;
    float weapon_list_content_height(int slot_count) const;

    SDL_FRect attachment_slot_rect(int index) const;
    SDL_FRect attachment_label_rect(int index) const;
    int attachment_slot_count() const { return attachment_slot_count_; }
    const WeaponRowTemplateLayout& weapon_row_template(const std::string& name) const;

private:
    SDL_FRect to_logical_rect(const NormalizedRect& rect) const;
    SDL_FRect child_rect(const LayoutContainerNode& container, const NormalizedRect& child) const;
    float normalized_x(float value) const;
    float normalized_y(float value) const;

    NormalizedRect resource_strip_{0.0125f, 0.0166667f, 0.975f, 0.0555556f};
    NormalizedRect close_button_{0.954167f, 0.018519f, 0.028125f, 0.05f};
    LayoutContainerNode weapon_list_{
        NormalizedRect{0.0166667f, 0.111111f, 0.158333f, 0.685185f},
        "panel_square_bronze",
        LayoutTextNode{NormalizedRect{0.05f, 0.035f, 0.6f, 0.08f}, "$panel.weapon_list"},
        NormalizedRect{0.04f, 0.12f, 0.92f, 0.84f}
    };
    float weapon_list_row_height_ = 0.0962963f;
    float weapon_list_row_pitch_ = 0.109259f;

    LayoutPanelNode preview_panel_{NormalizedRect{0.192708f, 0.116667f, 0.484375f, 0.518519f}, "panel_square_bronze"};
    NormalizedRect preview_nameplate_{0.323958f, 0.122222f, 0.221875f, 0.0537037f};
    LayoutPanelNode stats_panel_{NormalizedRect{0.703125f, 0.116667f, 0.276042f, 0.388889f}, "panel_square_bronze"};
    LayoutPanelNode effects_panel_{NormalizedRect{0.703125f, 0.524074f, 0.276042f, 0.142593f}, "panel_square_bronze"};
    LayoutPanelNode upgrade_panel_{NormalizedRect{0.703125f, 0.685185f, 0.276042f, 0.268519f}, "panel_square_bronze"};
    LayoutContainerNode attachments_{
        NormalizedRect{0.192708f, 0.662963f, 0.484375f, 0.144444f},
        "panel_square_bronze",
        LayoutTextNode{NormalizedRect{0.03f, 0.04f, 0.5f, 0.12f}, "$panel.attachment_slots"},
        NormalizedRect{0.025f, 0.18f, 0.95f, 0.76f}
    };
    LayoutPanelNode parts_panel_{NormalizedRect{0.192708f, 0.825926f, 0.484375f, 0.127778f}, "panel_square_bronze"};

    NormalizedRect attachment_slot_{0.178125f, 0.742593f, 0.0541667f, 0.0666667f};
    NormalizedRect attachment_label_{0.178125f, 0.712963f, 0.0541667f, 0.0185185f};
    float attachment_slot_gap_ = 0.0104167f;
    int attachment_slot_count_ = 6;
    WeaponRowTemplateLayout weapon_row_template_default_{};
    WeaponRowTemplateLayout weapon_row_template_pistol_{};
    WeaponRowTemplateLayout weapon_row_template_rifle_{};
    WeaponRowTemplateLayout weapon_row_template_smg_{};
    WeaponRowTemplateLayout weapon_row_template_sniper_{};
    WeaponRowTemplateLayout weapon_row_template_shotgun_{};
};

} // namespace zg
