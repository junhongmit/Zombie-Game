#include "Zombie.h"

#include "CollisionMap.h"
#include "Constants.h"
#include "MathUtil.h"

#include <cmath>

namespace zg {

namespace {

bool zombie_alive_has_floor_support(const CollisionMap& collision_map, float x, float y)
{
    const int foot_y = static_cast<int>(std::round(y + kZombieHeight + 1.0f));
    for (int probe = 5; probe <= 12; ++probe) {
        const int probe_x = static_cast<int>(std::round(x + probe));
        if (collision_map.is_solid(probe_x, foot_y)) {
            return true;
        }
    }
    return false;
}

bool zombie_corpse_has_floor_support(const CollisionMap& collision_map, float x, float y)
{
    const int foot_y = static_cast<int>(std::round(y + kZombieCorpseYOffset + kZombieCorpseHeight + 1.0f));
    for (int probe = 1; probe <= 31; ++probe) {
        const int probe_x = static_cast<int>(std::round(x + probe));
        if (collision_map.is_solid(probe_x, foot_y)) {
            return true;
        }
    }
    return false;
}

bool zombie_corpse_overlaps_solid(const CollisionMap& collision_map, float x, float y)
{
    for (int px = 1; px <= 31; ++px) {
        const int probe_x = static_cast<int>(std::round(x + px));
        for (int py = 0; py <= 17; ++py) {
            const int probe_y = static_cast<int>(std::round(y + kZombieCorpseYOffset + py));
            if (collision_map.is_solid(probe_x, probe_y)) {
                return true;
            }
        }
    }
    return false;
}

float snap_corpse_to_floor(const CollisionMap& collision_map, float x, float y)
{
    float snapped_y = std::round(y);
    while (snapped_y > 0.0f && zombie_corpse_overlaps_solid(collision_map, x, snapped_y)) {
        snapped_y -= 1.0f;
    }
    while (snapped_y < static_cast<float>(kLogicalHeight) &&
           !zombie_corpse_has_floor_support(collision_map, x, snapped_y)) {
        snapped_y += 1.0f;
    }
    return snapped_y;
}

void update_dead_zombie(const CollisionMap& collision_map, Zombie* zombie, float dt)
{
    if (!zombie->airborne && !zombie_corpse_has_floor_support(collision_map, zombie->x, zombie->y)) {
        zombie->airborne = true;
    }

    if (zombie->airborne) {
        zombie->vy = clamp_float(zombie->vy + kGravity * dt, -kPlayerJumpSpeed, kTerminalVelocity);
        float remaining_y = zombie->vy * dt;
        while (std::abs(remaining_y) > 0.0f) {
            const float step = clamp_float(remaining_y, -1.0f, 1.0f);
            const float trial_y = zombie->y + step;
            if (step > 0.0f && zombie_corpse_has_floor_support(collision_map, zombie->x, trial_y)) {
                zombie->y = snap_corpse_to_floor(collision_map, zombie->x, zombie->y);
                zombie->vy = 0.0f;
                zombie->vx = 0.0f;
                zombie->airborne = false;
                zombie->corpse_angle_degrees = zombie->walking_right ? 90.0f : -90.0f;
                break;
            }
            zombie->y = trial_y;
            remaining_y -= step;
        }

        float remaining_x = zombie->vx * dt;
        while (std::abs(remaining_x) > 0.0f) {
            const float step = clamp_float(remaining_x, -1.0f, 1.0f);
            zombie->x = clamp_float(zombie->x + step, 0.0f, kWorldWidth - kZombieCorpseWidth);
            remaining_x -= step;
        }

        zombie->corpse_angle_degrees += (zombie->walking_right ? 1.0f : -1.0f) * kZombieCorpseSpinSpeed * dt;
    } else {
        zombie->corpse_fade_timer += dt;
        if (zombie->corpse_fade_timer > kZombieCorpseFadeDelay) {
            const float fade_t = (zombie->corpse_fade_timer - kZombieCorpseFadeDelay) / kZombieCorpseFadeSeconds;
            zombie->corpse_alpha = clamp_float(255.0f * (1.0f - fade_t), 0.0f, 255.0f);
            if (zombie->corpse_alpha <= 0.0f) {
                zombie->active = false;
            }
        }
    }
}

} // namespace

Zombie::Zombie(float x_in, float y_in, float speed_in, bool walking_right_in, int walk_frame_in)
    : x(x_in),
      y(y_in),
      speed(speed_in),
      walking_right(walking_right_in),
      walk_frame(walk_frame_in),
      walk_frame_distance(0.0f)
{
}

void Zombie::update(const CollisionMap& collision_map, float dt)
{
    if (!active) {
        return;
    }

    if (hit_flash > 0.0f) {
        hit_flash -= dt;
        if (hit_flash < 0.0f) {
            hit_flash = 0.0f;
        }
    }

    if (!alive) {
        update_dead_zombie(collision_map, this, dt);
        return;
    }

    const float old_x = x;
    x += (walking_right ? speed : -speed) * dt;
    if (x < 0.0f) {
        x = 0.0f;
        walking_right = true;
    } else if (x > kWorldWidth - kZombieWidth) {
        x = kWorldWidth - kZombieWidth;
        walking_right = false;
    }

    walk_frame_distance += std::abs(x - old_x);
    while (walk_frame_distance >= kWalkPixelsPerFrame) {
        walk_frame_distance -= kWalkPixelsPerFrame;
        walk_frame = (walk_frame + 1) % 24;
    }
}

void Zombie::damage(int amount)
{
    if (!alive) {
        return;
    }

    hp -= amount;
    if (hp <= 0) {
        kill();
    } else {
        hit_flash = 0.08f;
    }
}

void Zombie::kill()
{
    hp = 0;
    alive = false;
    airborne = false;
    vx = walking_right ? kZombieCorpseDriftSpeed : -kZombieCorpseDriftSpeed;
    vy = -70.0f;
    corpse_angle_degrees = 0.0f;
    corpse_alpha = 255.0f;
    corpse_fade_timer = 0.0f;
    walk_frame = 0;
    walk_frame_distance = 0.0f;
}

ZombieHitRegion Zombie::hit_test(float world_x, float world_y, bool use_sprite_mask) const
{
    if (!active || !alive) {
        return ZombieHitRegion::None;
    }

    if (world_x < x || world_x >= x + kZombieWidth || world_y < y || world_y >= y + kZombieHeight) {
        return ZombieHitRegion::None;
    }

    if (use_sprite_mask) {
        const float local_x = world_x - x;
        const float sample_x = walking_right ? local_x : (kZombieWidth - 1.0f - local_x);
        const int sprite_x = static_cast<int>(std::floor(sample_x)) + walk_frame * 17;
        const int sprite_y = static_cast<int>(std::floor(world_y - y));
        if (sprite_x < 0 || sprite_y < 0) {
            return ZombieHitRegion::None;
        }
        return sprite_y < kZombieHeadHeight ? ZombieHitRegion::Head : ZombieHitRegion::Body;
    }

    const float local_y = world_y - y;
    return local_y < kZombieHeadHeight ? ZombieHitRegion::Head : ZombieHitRegion::Body;
}

} // namespace zg
