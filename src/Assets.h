#pragma once

#include "SpriteSheet.h"
#include "Texture.h"
#include "SurfaceMask.h"
#include "ui/ControlStyle.h"

#include <string>
#include <unordered_map>

struct SDL_Renderer;

namespace zg {

struct CharacterRigPart {
    SDL_Rect frame{};
    SDL_FPoint pivot{};
    SDL_FPoint distal_joint{};
    bool has_distal_joint = false;
    std::string anchor;
    int z_order = 0;
    float solver_length = 0.0f;
};

struct CharacterRigAsset {
    Texture sheet;
    bool loaded = false;
    float torso_scale_x_min = 0.98f;
    float torso_scale_x_max = 1.02f;
    float torso_scale_y_min = 0.96f;
    float torso_scale_y_max = 1.04f;
    CharacterRigPart head;
    CharacterRigPart torso;
    CharacterRigPart front_upper_arm;
    CharacterRigPart front_forearm;
    CharacterRigPart back_upper_arm;
    CharacterRigPart back_forearm;
    SDL_FPoint torso_front_shoulder{};
    SDL_FPoint torso_back_shoulder{};
    SDL_FPoint torso_pelvis{};
};

struct Assets {
    static constexpr int kExplosionFrameCount = 15;

    Texture sky;
    Texture backcity1;
    Texture backcity2;
    Texture backcity3;
    Texture building;
    Texture bench;
    Texture market;
    Texture notebook;
    Texture hero;
    CharacterRigAsset hero_rig;
    Texture bullet_icon;
    Texture smoke;
    Texture zombie;
    Texture explosion_sheet;
    Texture heavy_explosion_sheet;
    Texture grenade_effect_sheet;
    ControlStyle title_button_skin;
    ControlStyle weapon_card_style;
    ControlStyle inventory_card_style;
    ControlStyle panel_skin;
    ControlStyle scrollbar_vertical_track_style;
    ControlStyle scrollbar_vertical_fill_style;
    ControlStyle scrollbar_vertical_thumb_style;
    ControlStyle scrollbar_horizontal_track_style;
    ControlStyle scrollbar_horizontal_fill_style;
    ControlStyle scrollbar_horizontal_thumb_style;
    ControlStyle progressbar_horizontal_track_style;
    ControlStyle progressbar_horizontal_fill_style;
    ControlStyle progressbar_vertical_track_style;
    ControlStyle progressbar_vertical_fill_style;
    SpriteSheet explosion_sheet_meta;
    SpriteSheet heavy_explosion_sheet_meta;
    SpriteSheet grenade_effect_sheet_meta;
    bool has_explosion_sheet = false;
    bool has_heavy_explosion_sheet = false;
    bool has_grenade_effect_sheet = false;
    Texture explosions[kExplosionFrameCount];
    SurfaceMask zombie_mask;
    std::unordered_map<std::string, ControlStyle> ui_skins;
    std::unordered_map<std::string, std::string> scene_asset_paths;

    bool load(SDL_Renderer* renderer);
    const ControlStyle* find_ui_skin(const std::string& name) const;
    const std::string* find_scene_asset_path(const std::string& name) const;
};

} // namespace zg
