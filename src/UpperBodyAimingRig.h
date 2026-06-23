#pragma once

#include <SDL3/SDL.h>

namespace zg {

struct Player;
struct WeaponDefinition;

struct TwoBoneIkResult {
    SDL_FPoint shoulder{};
    SDL_FPoint elbow{};
    SDL_FPoint hand{};
    bool reachable = false;
};

struct UpperBodyAimPoseSample {
    float angle_deg = 0.0f;
    float head_rotation_deg = 0.0f;
    float torso_rotation_deg = 0.0f;
    SDL_FPoint front_shoulder_offset{};
    SDL_FPoint back_shoulder_offset{};
    SDL_FPoint front_elbow_hint{};
    SDL_FPoint back_elbow_hint{};
    SDL_FPoint weapon_anchor_offset{};
};

struct UpperBodyAimingRigState {
    bool valid = false;
    bool facing_right = true;
    float aim_local_deg = 0.0f;
    float weapon_rotation_deg = 0.0f;
    SDL_FPoint pelvis{};
    SDL_FPoint torso_base{};
    SDL_FPoint torso_top{};
    SDL_FPoint head_center{};
    SDL_FPoint rear_grip{};
    SDL_FPoint front_grip{};
    SDL_FPoint muzzle{};
    SDL_FPoint front_shoulder{};
    SDL_FPoint back_shoulder{};
    SDL_FPoint front_elbow_hint{};
    SDL_FPoint back_elbow_hint{};
    TwoBoneIkResult front_arm{};
    TwoBoneIkResult back_arm{};
};

class UpperBodyAimingRig {
public:
    UpperBodyAimingRigState solve(const Player& player, const WeaponDefinition* weapon_definition) const;

private:
    UpperBodyAimPoseSample sample_pose(float local_aim_deg) const;
};

} // namespace zg
