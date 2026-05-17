#include "Weapon.h"

#include "AssetPaths.h"
#include "Constants.h"
#include "MathUtil.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

namespace zg {

namespace {

std::string trim_copy(const std::string& value)
{
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return value.substr(start, end - start);
}

std::string lower_copy(std::string value)
{
    for (char& ch : value) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

int parse_int(const std::string& text)
{
    return std::atoi(text.c_str());
}

float parse_float(const std::string& text)
{
    return static_cast<float>(std::atof(text.c_str()));
}

void apply_default_feedback(WeaponDefinition* definition)
{
    if (definition->name == "Glock") {
        definition->shake_duration = 0.10f;
        definition->shake_magnitude = 1.4f;
        definition->loudness = 0.78f;
    } else if (definition->name == "AK47") {
        definition->shake_duration = 0.14f;
        definition->shake_magnitude = 3.4f;
        definition->loudness = 0.96f;
    } else if (definition->name == "AWM") {
        definition->shake_duration = 0.20f;
        definition->shake_magnitude = 5.0f;
        definition->loudness = 1.0f;
    } else if (definition->name == "AUG") {
        definition->shake_duration = 0.13f;
        definition->shake_magnitude = 2.8f;
        definition->loudness = 0.92f;
    } else if (definition->name == "M249") {
        definition->shake_duration = 0.15f;
        definition->shake_magnitude = 3.8f;
        definition->loudness = 0.98f;
    } else if (definition->full_auto) {
        definition->shake_duration = 0.13f;
        definition->shake_magnitude = 3.0f;
        definition->loudness = 0.94f;
    }
}

} // namespace

float WeaponDefinition::fire_interval_seconds() const
{
    return speed_rpm > 0 ? 60.0f / static_cast<float>(speed_rpm) : kGlockFireIntervalSeconds;
}

bool WeaponCatalog::load(SDL_Renderer* renderer, const char* path)
{
    definitions_.clear();

    const std::string resolved = resolve_asset_path(path);
    std::FILE* file = std::fopen(resolved.c_str(), "r");
    if (file == nullptr) {
        return false;
    }

    WeaponDefinition current;
    bool has_current = false;
    char buffer[512];
    while (std::fgets(buffer, sizeof(buffer), file) != nullptr) {
        std::string line = trim_copy(buffer);
        if (line.empty()) {
            continue;
        }

        if (!line.empty() && line[0] == '#') {
            if (lower_copy(line).find("#weapon") == 0) {
                if (has_current && !current.name.empty() && !current.image_path.empty()) {
                    if (current.initial_reserve <= 0) {
                        current.initial_reserve = current.magazine_size * 5;
                    }
                    apply_default_feedback(&current);
                    if (!current.texture.load(renderer, current.image_path.c_str(), true)) {
                        std::fclose(file);
                        return false;
                    }
                    definitions_.push_back(std::move(current));
                    current = WeaponDefinition{};
                }
                has_current = true;
            }
            continue;
        }

        const size_t equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        const std::string key = lower_copy(trim_copy(line.substr(0, equals)));
        const std::string value = trim_copy(line.substr(equals + 1));
        if (key == "name") {
            current.name = value;
        } else if (key == "imagepath") {
            current.image_path = value;
        } else if (key == "type") {
            current.type = parse_int(value) == 102 ? WeaponType::Grenade : WeaponType::Gun;
        } else if (key == "magazine") {
            current.magazine_size = parse_int(value);
        } else if (key == "reloadtime") {
            current.reload_duration = parse_int(value) * 0.05f;
        } else if (key == "diameter" || key == "diamter") {
            current.diameter = parse_float(value);
        } else if (key == "up") {
            current.up = parse_float(value);
        } else if (key == "damage") {
            current.damage = parse_int(value);
        } else if (key == "running" || key == "ruuning") {
            current.full_auto = parse_int(value) != 0;
        } else if (key == "speed") {
            current.speed_rpm = parse_int(value);
        } else if (key == "price") {
            current.price = parse_int(value);
        } else if (key == "sound") {
            current.shoot_sound_path = value;
        } else if (key == "routex") {
            current.route_x = parse_int(value);
        } else if (key == "routey") {
            current.route_y = parse_int(value);
        } else if (key == "shakeduration") {
            current.shake_duration = parse_float(value);
        } else if (key == "shakemagnitude") {
            current.shake_magnitude = parse_float(value);
        } else if (key == "loudness") {
            current.loudness = parse_float(value);
        } else if (key == "reserveammo") {
            current.initial_reserve = parse_int(value);
        }
    }
    std::fclose(file);

    if (has_current && !current.name.empty() && !current.image_path.empty()) {
        if (current.initial_reserve <= 0) {
            current.initial_reserve = current.magazine_size * 5;
        }
        apply_default_feedback(&current);
        if (!current.texture.load(renderer, current.image_path.c_str(), true)) {
            return false;
        }
        definitions_.push_back(std::move(current));
    }

    return !definitions_.empty();
}

int WeaponCatalog::count() const
{
    return static_cast<int>(definitions_.size());
}

const WeaponDefinition* WeaponCatalog::definition(int index) const
{
    if (index < 0 || index >= static_cast<int>(definitions_.size())) {
        return nullptr;
    }
    return &definitions_[static_cast<size_t>(index)];
}

WeaponState::WeaponState()
{
}

bool WeaponState::load_default_inventory(const WeaponCatalog& catalog)
{
    slot_count_ = 0;
    active_slot_ = 0;

    for (int i = 0; i < catalog.count() && slot_count_ < kMaxSlots; ++i) {
        const WeaponDefinition* definition = catalog.definition(i);
        if (definition == nullptr || definition->type != WeaponType::Gun) {
            continue;
        }

        WeaponSlot& slot = slots_[slot_count_++];
        slot.definition = definition;
        slot.ammo_in_mag = definition->magazine_size;
        slot.ammo_reserve = definition->initial_reserve;
    }

    if (slot_count_ <= 0) {
        return false;
    }

    sync_from_slot();
    return true;
}

bool WeaponState::switch_to_slot(int slot_index)
{
    if (slot_index < 0 || slot_index >= slot_count_ || slot_index == active_slot_) {
        return false;
    }

    sync_to_slot();
    active_slot_ = slot_index;
    reloading = false;
    reload_timer = 0.0f;
    reload_flash_timer = 0.0f;
    reload_flash_on = false;
    sync_from_slot();
    return true;
}

bool WeaponState::cycle(int delta)
{
    if (slot_count_ <= 1 || delta == 0) {
        return false;
    }

    const int next = (active_slot_ + delta + slot_count_) % slot_count_;
    return switch_to_slot(next);
}

bool WeaponState::can_fire() const
{
    return current_definition() != nullptr && !reloading && ammo_in_mag > 0;
}

void WeaponState::consume_round()
{
    if (ammo_in_mag > 0) {
        --ammo_in_mag;
        sync_to_slot();
    }
    if (ammo_in_mag <= 0) {
        ammo_in_mag = 0;
        sync_to_slot();
        start_reload();
    }
}

void WeaponState::start_reload()
{
    if (reloading || ammo_in_mag >= magazine_size || ammo_reserve <= 0) {
        return;
    }

    reloading = true;
    reload_timer = reload_duration;
    reload_flash_on = false;
    reload_flash_timer = kReloadFlashIntervalSeconds;
}

void WeaponState::update(float dt)
{
    if (!reloading) {
        return;
    }

    reload_timer -= dt;
    reload_flash_timer -= dt;
    while (reload_flash_timer <= 0.0f) {
        reload_flash_on = !reload_flash_on;
        reload_flash_timer += kReloadFlashIntervalSeconds;
    }
    if (reload_timer > 0.0f) {
        return;
    }

    const int needed = magazine_size - ammo_in_mag;
    const int loaded = needed < ammo_reserve ? needed : ammo_reserve;
    ammo_in_mag += loaded;
    ammo_reserve -= loaded;
    reload_timer = 0.0f;
    reload_flash_timer = 0.0f;
    reloading = false;
    reload_flash_on = false;
    sync_to_slot();
}

float WeaponState::indicator_ratio() const
{
    if (reloading) {
        return reload_duration > 0.0f ? clamp_float(reload_timer / reload_duration, 0.0f, 1.0f) : 0.0f;
    }
    return magazine_size > 0 ? clamp_float(static_cast<float>(ammo_in_mag) / static_cast<float>(magazine_size), 0.0f, 1.0f) : 0.0f;
}

int WeaponState::slot_count() const
{
    return slot_count_;
}

int WeaponState::active_slot_index() const
{
    return active_slot_;
}

const WeaponDefinition* WeaponState::current_definition() const
{
    return slot_count_ > 0 ? slots_[active_slot_].definition : nullptr;
}

const WeaponSlot* WeaponState::slots() const
{
    return slots_;
}

void WeaponState::sync_from_slot()
{
    const WeaponDefinition* definition = current_definition();
    if (definition == nullptr) {
        ammo_in_mag = 0;
        ammo_reserve = 0;
        magazine_size = 0;
        reload_duration = 0.0f;
        return;
    }

    const WeaponSlot& slot = slots_[active_slot_];
    ammo_in_mag = slot.ammo_in_mag;
    ammo_reserve = slot.ammo_reserve;
    magazine_size = definition->magazine_size;
    reload_duration = definition->reload_duration;
}

void WeaponState::sync_to_slot()
{
    if (slot_count_ <= 0) {
        return;
    }

    WeaponSlot& slot = slots_[active_slot_];
    slot.ammo_in_mag = ammo_in_mag;
    slot.ammo_reserve = ammo_reserve;
}

} // namespace zg
