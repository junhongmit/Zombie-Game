#include "UpperBodyAimingRig.h"

#include "Constants.h"
#include "Player.h"
#include "Weapon.h"

#include <algorithm>
#include <cmath>

namespace zg {

namespace {

constexpr float kPi = 3.1415926535f;

float degrees_to_radians(float degrees)
{
    return degrees * kPi / 180.0f;
}

float radians_to_degrees(float radians)
{
    return radians * 180.0f / kPi;
}

float clampf(float value, float min_value, float max_value)
{
    return std::max(min_value, std::min(max_value, value));
}

SDL_FPoint add(SDL_FPoint a, SDL_FPoint b)
{
    return SDL_FPoint{a.x + b.x, a.y + b.y};
}

SDL_FPoint subtract(SDL_FPoint a, SDL_FPoint b)
{
    return SDL_FPoint{a.x - b.x, a.y - b.y};
}

SDL_FPoint scale(SDL_FPoint v, float s)
{
    return SDL_FPoint{v.x * s, v.y * s};
}

float length(SDL_FPoint v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

float cross_z(SDL_FPoint a, SDL_FPoint b)
{
    return a.x * b.y - a.y * b.x;
}

SDL_FPoint normalize(SDL_FPoint v)
{
    const float len = length(v);
    if (len <= 0.0001f) {
        return SDL_FPoint{1.0f, 0.0f};
    }
    return scale(v, 1.0f / len);
}

SDL_FPoint perpendicular(SDL_FPoint v)
{
    return SDL_FPoint{-v.y, v.x};
}

SDL_FPoint rotate(SDL_FPoint v, float degrees)
{
    const float r = degrees_to_radians(degrees);
    const float c = std::cos(r);
    const float s = std::sin(r);
    return SDL_FPoint{
        v.x * c - v.y * s,
        v.x * s + v.y * c
    };
}

SDL_FPoint mirror_local(SDL_FPoint local, bool facing_right)
{
    return SDL_FPoint{facing_right ? local.x : -local.x, local.y};
}

SDL_FPoint apply_torso_space_offset(SDL_FPoint origin, SDL_FPoint local, bool facing_right, SDL_FPoint torso_up)
{
    const SDL_FPoint up = normalize(torso_up);
    const SDL_FPoint right = perpendicular(up);
    const float side = facing_right ? 1.0f : -1.0f;
    return add(
        origin,
        add(
            scale(right, local.x * side),
            scale(up, local.y)));
}

float normalize_local_aim_deg(const Player& player)
{
    const float forward_x = std::cos(player.aim_angle_radians);
    const float forward_y = -std::sin(player.aim_angle_radians);
    const float facing_sign = player.facing_right ? 1.0f : -1.0f;
    return radians_to_degrees(std::atan2(-forward_y, forward_x * facing_sign));
}

UpperBodyAimPoseSample interpolate(const UpperBodyAimPoseSample& a, const UpperBodyAimPoseSample& b, float t)
{
    const auto lerp = [t](float x, float y) { return x + (y - x) * t; };
    const auto lerp_point = [t, &lerp](SDL_FPoint p0, SDL_FPoint p1) {
        return SDL_FPoint{lerp(p0.x, p1.x), lerp(p0.y, p1.y)};
    };

    UpperBodyAimPoseSample out;
    out.angle_deg = lerp(a.angle_deg, b.angle_deg);
    out.head_rotation_deg = lerp(a.head_rotation_deg, b.head_rotation_deg);
    out.torso_rotation_deg = lerp(a.torso_rotation_deg, b.torso_rotation_deg);
    out.front_shoulder_offset = lerp_point(a.front_shoulder_offset, b.front_shoulder_offset);
    out.back_shoulder_offset = lerp_point(a.back_shoulder_offset, b.back_shoulder_offset);
    out.front_elbow_hint = lerp_point(a.front_elbow_hint, b.front_elbow_hint);
    out.back_elbow_hint = lerp_point(a.back_elbow_hint, b.back_elbow_hint);
    out.weapon_anchor_offset = lerp_point(a.weapon_anchor_offset, b.weapon_anchor_offset);
    return out;
}

TwoBoneIkResult solve_two_bone_ik(SDL_FPoint shoulder, SDL_FPoint target, SDL_FPoint hint, float upper_len, float lower_len, int preferred_side)
{
    TwoBoneIkResult result;
    result.shoulder = shoulder;

    SDL_FPoint to_target = subtract(target, shoulder);
    float dist = length(to_target);
    if (dist <= 0.0001f) {
        result.elbow = shoulder;
        result.hand = shoulder;
        return result;
    }

    const float max_reach = std::max(0.001f, upper_len + lower_len - 0.001f);
    const float min_reach = std::max(0.001f, std::fabs(upper_len - lower_len) + 0.001f);
    dist = clampf(dist, min_reach, max_reach);

    const SDL_FPoint dir = normalize(to_target);
    const float along = (upper_len * upper_len - lower_len * lower_len + dist * dist) / (2.0f * dist);
    const float height_sq = std::max(0.0f, upper_len * upper_len - along * along);
    const float height = std::sqrt(height_sq);

    const SDL_FPoint mid = add(shoulder, scale(dir, along));
    const SDL_FPoint perp = normalize(perpendicular(dir));
    const SDL_FPoint elbow_a = add(mid, scale(perp, height));
    const SDL_FPoint elbow_b = add(mid, scale(perp, -height));
    const float side_a = cross_z(dir, subtract(elbow_a, shoulder));
    const float side_b = cross_z(dir, subtract(elbow_b, shoulder));

    if (preferred_side > 0) {
        result.elbow = side_a >= side_b ? elbow_a : elbow_b;
        result.hand = add(shoulder, scale(dir, dist));
        result.reachable = true;
        return result;
    }
    if (preferred_side < 0) {
        result.elbow = side_a <= side_b ? elbow_a : elbow_b;
        result.hand = add(shoulder, scale(dir, dist));
        result.reachable = true;
        return result;
    }

    const float da = (elbow_a.x - hint.x) * (elbow_a.x - hint.x) + (elbow_a.y - hint.y) * (elbow_a.y - hint.y);
    const float db = (elbow_b.x - hint.x) * (elbow_b.x - hint.x) + (elbow_b.y - hint.y) * (elbow_b.y - hint.y);

    result.elbow = da <= db ? elbow_a : elbow_b;
    result.hand = add(shoulder, scale(dir, dist));
    result.reachable = true;
    return result;
}

} // namespace

UpperBodyAimPoseSample UpperBodyAimingRig::sample_pose(float local_aim_deg) const
{
    const auto make_sample = [](
        float angle_deg,
        float head_rotation_deg,
        float torso_rotation_deg,
        SDL_FPoint front_shoulder_offset,
        SDL_FPoint back_shoulder_offset,
        SDL_FPoint front_elbow_hint,
        SDL_FPoint back_elbow_hint,
        SDL_FPoint weapon_anchor_offset) {
        UpperBodyAimPoseSample sample;
        sample.angle_deg = angle_deg;
        sample.head_rotation_deg = head_rotation_deg;
        sample.torso_rotation_deg = torso_rotation_deg;
        sample.front_shoulder_offset = front_shoulder_offset;
        sample.back_shoulder_offset = back_shoulder_offset;
        sample.front_elbow_hint = front_elbow_hint;
        sample.back_elbow_hint = back_elbow_hint;
        sample.weapon_anchor_offset = weapon_anchor_offset;
        return sample;
    };

    static const UpperBodyAimPoseSample kSamples[] = {
        make_sample(-75.0f, -34.0f, -15.0f, SDL_FPoint{2.6f, 0.6f}, SDL_FPoint{-2.0f, 1.2f}, SDL_FPoint{7.0f, 9.5f}, SDL_FPoint{-6.5f, 8.0f}, SDL_FPoint{2.5f, -1.5f}),
        make_sample(-35.0f, -16.0f, -7.0f, SDL_FPoint{2.9f, 0.2f}, SDL_FPoint{-2.2f, 0.8f}, SDL_FPoint{7.5f, 10.0f}, SDL_FPoint{-7.0f, 8.8f}, SDL_FPoint{2.5f, -0.8f}),
        make_sample(0.0f, 0.0f, 0.0f, SDL_FPoint{3.0f, 0.0f}, SDL_FPoint{-2.3f, 0.3f}, SDL_FPoint{7.5f, 10.8f}, SDL_FPoint{-7.2f, 9.6f}, SDL_FPoint{2.2f, 0.0f}),
        make_sample(35.0f, 15.0f, 7.0f, SDL_FPoint{2.7f, -0.8f}, SDL_FPoint{-2.0f, -0.3f}, SDL_FPoint{6.8f, 10.8f}, SDL_FPoint{-6.6f, 9.8f}, SDL_FPoint{1.5f, -0.6f}),
        make_sample(75.0f, 30.0f, 14.0f, SDL_FPoint{2.3f, -1.5f}, SDL_FPoint{-1.8f, -1.0f}, SDL_FPoint{6.0f, 10.5f}, SDL_FPoint{-6.0f, 9.8f}, SDL_FPoint{0.8f, -1.2f}),
    };

    const float clamped = clampf(local_aim_deg, kSamples[0].angle_deg, kSamples[4].angle_deg);
    for (int i = 0; i < 4; ++i) {
        if (clamped >= kSamples[i].angle_deg && clamped <= kSamples[i + 1].angle_deg) {
            const float span = kSamples[i + 1].angle_deg - kSamples[i].angle_deg;
            const float t = span > 0.0f ? (clamped - kSamples[i].angle_deg) / span : 0.0f;
            return interpolate(kSamples[i], kSamples[i + 1], t);
        }
    }
    return kSamples[2];
}

UpperBodyAimingRigState UpperBodyAimingRig::solve(const Player& player, const WeaponDefinition* weapon_definition) const
{
    UpperBodyAimingRigState state;
    state.valid = true;
    state.facing_right = player.facing_right;
    state.aim_local_deg = normalize_local_aim_deg(player);
    const UpperBodyAimPoseSample pose = sample_pose(state.aim_local_deg);

    const SDL_FPoint torso_base{player.x + 9.0f, player.y + 18.5f};
    const float facing_sign = player.facing_right ? 1.0f : -1.0f;
    const float torso_rot = -pose.torso_rotation_deg * facing_sign;
    const float head_rot = -pose.head_rotation_deg * facing_sign;

    state.pelvis = SDL_FPoint{player.x + 9.0f, player.y + 25.5f};
    state.torso_base = torso_base;
    state.torso_top = add(torso_base, rotate(SDL_FPoint{0.0f, -kRigTorsoLength}, torso_rot));
    state.head_center = add(state.torso_top, rotate(SDL_FPoint{0.0f, -(kRigNeckLength + kRigHeadRadius)}, head_rot));

    // In side view, the "weapon hand" should come from the shoulder nearer to the camera.
    // For a facing-right character that is the screen-left shoulder; for facing-left it mirrors.
    const SDL_FPoint torso_up = subtract(state.torso_top, state.torso_base);
    const SDL_FPoint torso_up_n = normalize(torso_up);
    const SDL_FPoint torso_right_n = normalize(perpendicular(torso_up_n));
    const SDL_FPoint torso_down_n = scale(torso_up_n, -1.0f);
    state.front_shoulder = apply_torso_space_offset(state.torso_top, pose.back_shoulder_offset, player.facing_right, torso_up);
    state.back_shoulder = apply_torso_space_offset(state.torso_top, pose.front_shoulder_offset, player.facing_right, torso_up);

    const float frame_width = weapon_definition != nullptr && weapon_definition->texture.valid()
        ? weapon_definition->texture.width() * 0.5f
        : 12.0f;
    const float route_x = weapon_definition != nullptr ? static_cast<float>(weapon_definition->route_x) : 4.0f;
    const float route_y = weapon_definition != nullptr ? static_cast<float>(weapon_definition->route_y) : 3.0f;
    const float front_grip_dx_base = std::max(3.6f, frame_width * 0.30f);
    const float front_grip_dx = front_grip_dx_base;
    const float front_grip_dy = 0.7f;
    const float muzzle_dx = std::max(front_grip_dx + 4.0f, frame_width - 1.0f);
    const float muzzle_dy = -0.4f;

    const float world_weapon_angle = player.facing_right ? state.aim_local_deg : 180.0f - state.aim_local_deg;
    state.weapon_rotation_deg = world_weapon_angle;
    const float weapon_rad = degrees_to_radians(world_weapon_angle);
    const SDL_FPoint forward{std::cos(weapon_rad), -std::sin(weapon_rad)};
    const SDL_FPoint down{forward.y, -forward.x};
    const float pivot_world_x = player.x + (player.facing_right ? 8.0f : 10.0f);
    const float pivot_world_y = player.y + 13.0f;
    const float aim_sin = std::sin(degrees_to_radians(state.aim_local_deg));
    const float vertical_extreme = std::fabs(aim_sin);
    const float aim_up_factor = clampf(aim_sin, 0.0f, 1.0f);
    const float aim_down_factor = clampf(-aim_sin, 0.0f, 1.0f);
    const float forward_slide = 3.5f * aim_up_factor - 2.4f * aim_down_factor;
    const float down_slide = 0.8f * aim_up_factor + 1.0f * aim_down_factor;
    state.rear_grip = add(
        SDL_FPoint{pivot_world_x, pivot_world_y},
        SDL_FPoint{
            forward.x * forward_slide + down.x * down_slide,
            forward.y * forward_slide + down.y * down_slide
        });
    state.front_grip = add(
        state.rear_grip,
        SDL_FPoint{
            forward.x * front_grip_dx + down.x * front_grip_dy,
            forward.y * front_grip_dx + down.y * front_grip_dy
        });
    state.muzzle = add(
        state.rear_grip,
        SDL_FPoint{
            forward.x * muzzle_dx + down.x * muzzle_dy,
            forward.y * muzzle_dx + down.y * muzzle_dy
        });

    const float front_side_sign = player.facing_right ? -1.0f : 1.0f;
    const float back_side_sign = -front_side_sign;

    state.front_elbow_hint = add(
        state.front_shoulder,
        add(
            add(
                scale(torso_right_n, front_side_sign * (5.5f + aim_up_factor * 1.2f)),
                scale(torso_down_n, 4.8f + aim_down_factor * 3.0f)),
            scale(forward, -1.4f)));
    state.back_elbow_hint = add(
        state.back_shoulder,
        add(
            add(
                scale(torso_right_n, back_side_sign * (4.8f + aim_up_factor * 0.8f)),
                scale(torso_down_n, 5.2f + aim_down_factor * 2.4f)),
            scale(forward, -0.6f)));

    state.front_arm = solve_two_bone_ik(
        state.front_shoulder,
        state.rear_grip,
        state.front_elbow_hint,
        kRigUpperArmLength,
        kRigForearmLength,
        player.facing_right ? 1 : -1);
    state.back_arm = solve_two_bone_ik(
        state.back_shoulder,
        state.front_grip,
        state.back_elbow_hint,
        kRigUpperArmLength,
        kRigForearmLength,
        player.facing_right ? 1 : -1);
    return state;
}

} // namespace zg
