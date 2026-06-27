#pragma once

#include "Texture.h"
#include "WeaponRelight.h"

#include <future>
#include <string>
#include <vector>

struct SDL_Renderer;

namespace zg {

enum class WeaponType {
    Gun = 101,
    Grenade = 102
};

struct WeaponDefinition {
    struct LocalPoint {
        float x = 0.0f;
        float y = 0.0f;

        LocalPoint() = default;
        LocalPoint(float x_value, float y_value)
            : x(x_value), y(y_value)
        {
        }
    };

    std::string name;
    std::string image_path;
    std::string hold_metadata_path;
    std::string preview_image_path;
    std::string icon_image_path;
    std::string ui_card_template = "default";
    std::string shoot_sound_path;
    bool mirrored_pair_sprite = true;
    float hold_scale = 1.0f;
    int route_x = 4;
    int route_y = 3;
    LocalPoint rear_wrist{};
    LocalPoint front_wrist{};
    LocalPoint muzzle{};
    WeaponType type = WeaponType::Gun;
    int magazine_size = 0;
    int damage = 0;
    int speed_rpm = 600;
    int price = 0;
    int initial_reserve = 0;
    bool full_auto = false;
    float reload_duration = 1.6f;
    float diameter = 0.0f;
    float up = 0.0f;
    float shake_duration = 0.12f;
    float shake_magnitude = 2.0f;
    float loudness = 0.85f;
    Texture texture;
    Texture preview_texture;
    Texture icon_texture;
    Texture workbench_lit_texture;
    Texture workbench_shadow_texture;
    WorkbenchShadowPlacement workbench_shadow_placement;

    float fire_interval_seconds() const;
};

class WeaponCatalog {
public:
    bool load(SDL_Renderer* renderer, const char* path);
    void update_relight_jobs(SDL_Renderer* renderer);
    int count() const;
    const WeaponDefinition* definition(int index) const;

private:
    struct RelightJob {
        bool scheduled = false;
        bool uploaded = false;
        std::future<WorkbenchRelightBakeResult> future;
    };

    void schedule_relight_job(int index);

    std::vector<WeaponDefinition> definitions_;
    std::vector<RelightJob> relight_jobs_;
};

struct WeaponSlot {
    const WeaponDefinition* definition = nullptr;
    int ammo_in_mag = 0;
    int ammo_reserve = 0;
};

struct WeaponState {
    static constexpr int kMaxSlots = 5;

    int ammo_in_mag = 0;
    int ammo_reserve = 0;
    int magazine_size = 0;
    float reload_duration = 0.0f;
    float reload_timer = 0.0f;
    float reload_flash_timer = 0.0f;
    bool reloading = false;
    bool reload_flash_on = false;

    WeaponState();

    bool load_default_inventory(const WeaponCatalog& catalog);
    bool switch_to_slot(int slot_index);
    bool cycle(int delta);
    bool can_fire() const;
    void consume_round();
    void start_reload();
    void update(float dt);
    float indicator_ratio() const;
    int slot_count() const;
    int active_slot_index() const;
    const WeaponDefinition* current_definition() const;
    const WeaponSlot* slots() const;

private:
    void sync_from_slot();
    void sync_to_slot();

    WeaponSlot slots_[kMaxSlots];
    int slot_count_ = 0;
    int active_slot_ = 0;
};

} // namespace zg
