#include "Grenade.h"

#include "Effects.h"
#include "Player.h"
#include "Zombie.h"

#include <cmath>

namespace zg {

namespace {

float distance_between(float x1, float y1, float x2, float y2)
{
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

bool in_explosion_halfspace(SurfaceImpactDirection direction, float source_x, float source_y, float target_x, float target_y)
{
    switch (direction) {
    case SurfaceImpactDirection::Top:
        return target_y <= source_y;
    case SurfaceImpactDirection::Bottom:
        return target_y >= source_y;
    case SurfaceImpactDirection::Left:
        return target_x <= source_x;
    case SurfaceImpactDirection::Right:
        return target_x >= source_x;
    case SurfaceImpactDirection::None:
    default:
        return true;
    }
}

} // namespace

void GrenadeSystem::update(
    const CollisionMap& collision_map,
    Player* player,
    Zombie* zombies,
    int zombie_count,
    EffectsSystem* effects,
    float dt)
{
    if (throw_cooldown_ > 0.0f) {
        throw_cooldown_ -= dt;
    }

    for (Grenade& grenade : grenades_) {
        if (!grenade.active) {
            continue;
        }

        grenade.prev_x = grenade.x;
        grenade.prev_y = grenade.y;
        grenade.vy += kGrenadeGravity * dt;
        grenade.x += grenade.vx * dt;
        grenade.y += grenade.vy * dt;

        if (grenade.clip_tail &&
            distance_between(grenade.x, grenade.y, grenade.clip_x, grenade.clip_y) >= kGrenadeRenderLength) {
            grenade.clip_tail = false;
        }

        if (grenade.x < 0.0f || grenade.x >= kWorldWidth || grenade.y < 0.0f || grenade.y >= kGameplayViewHeight) {
            grenade.active = false;
            continue;
        }

        const int sample_x = static_cast<int>(std::round(grenade.x));
        const int sample_y = static_cast<int>(std::round(grenade.y));
        if (collision_map.is_solid(sample_x, sample_y)) {
            const SurfaceImpactDirection direction = collision_map.impact_direction_at(sample_x, sample_y);
            const float distance_to_player = player != nullptr
                ? distance_between(grenade.x, grenade.y, player->x + 8.0f, player->y + 13.0f)
                : 100000.0f;
            const bool safe_bounce = grenade.can_explode &&
                direction == SurfaceImpactDirection::Top &&
                grenade.vy > 0.0f &&
                distance_to_player < kGrenadeSafeBounceRadius;

            if (safe_bounce) {
                grenade.can_explode = false;
                if (distance_to_player < kGrenadeSafeBounceVanishRadius) {
                    grenade.active = false;
                    continue;
                }

                grenade.clip_x = grenade.x;
                grenade.clip_y = grenade.y;
                grenade.clip_tail = true;
                grenade.x = grenade.prev_x;
                grenade.y = grenade.prev_y;
                grenade.vy = -grenade.vy * kGrenadeBounceDamping;
                grenade.vx *= 0.88f;
                ++grenade.bounces;
                if (std::fabs(grenade.vy) >= kGrenadeMinBounceSpeed) {
                    continue;
                }
                grenade.active = false;
                continue;
            }

            grenade.active = false;
            if (grenade.can_explode) {
                explode(grenade.x, grenade.y, direction, player, zombies, zombie_count, effects);
            }
        }
    }

    for (Explosion& explosion : explosions_) {
        if (!explosion.active) {
            continue;
        }

        explosion.frame += kGrenadeExplosionFrameRate * dt;
        if (explosion.frame >= static_cast<float>(kGrenadeExplosionFrameCount)) {
            explosion.active = false;
        }
    }
}

bool GrenadeSystem::try_throw(const Player& player, bool throw_pressed, EffectsSystem* effects)
{
    if (!throw_pressed || throw_cooldown_ > 0.0f) {
        return false;
    }

    Grenade& grenade = grenades_[next_grenade_];
    next_grenade_ = (next_grenade_ + 1) % kMaxGrenades;
    grenade.x = player.x + 8.0f;
    grenade.y = player.y + 13.0f;
    grenade.prev_x = grenade.x;
    grenade.prev_y = grenade.y;
    grenade.vx = std::cos(player.aim_angle_radians) * kGrenadeThrowSpeed;
    grenade.vy = -std::sin(player.aim_angle_radians) * kGrenadeThrowSpeed - 40.0f;
    grenade.clip_x = grenade.x;
    grenade.clip_y = grenade.y;
    grenade.bounces = 0;
    grenade.can_explode = true;
    grenade.clip_tail = false;
    grenade.active = true;
    throw_cooldown_ = kGrenadeCooldownSeconds;
    if (effects != nullptr) {
        effects->spawn_smoke(grenade.x, grenade.y, player.aim_angle_radians, kGrenadeLaunchSmokeSpeed, kGrenadeLaunchSmokeCount);
    }
    return true;
}

const Grenade* GrenadeSystem::grenades() const
{
    return grenades_;
}

int GrenadeSystem::grenade_count() const
{
    return kMaxGrenades;
}

const Explosion* GrenadeSystem::explosions() const
{
    return explosions_;
}

int GrenadeSystem::explosion_count() const
{
    return kMaxExplosions;
}

bool GrenadeSystem::consume_explosion_event()
{
    const bool pending = explosion_event_pending_;
    explosion_event_pending_ = false;
    return pending;
}

void GrenadeSystem::explode(
    float x,
    float y,
    SurfaceImpactDirection direction,
    Player* player,
    Zombie* zombies,
    int zombie_count,
    EffectsSystem* effects)
{
    Explosion& explosion = explosions_[next_explosion_];
    next_explosion_ = (next_explosion_ + 1) % kMaxExplosions;
    explosion.x = x;
    explosion.y = y;
    explosion.frame = 0.0f;
    explosion.direction = direction;
    explosion.active = true;
    explosion_event_pending_ = true;

    if (player != nullptr) {
        const float px = player->x + 9.0f;
        const float py = player->y + 16.0f;
        const float distance = distance_between(x, y, px, py);
        if (distance <= kGrenadeBlastPlayerRadius && in_explosion_halfspace(direction, x, y, px, py)) {
            const float angle = std::atan2(py - y, px - x);
            const float knockback = kExplosionKnockbackSpeed * (1.0f - distance / kGrenadeBlastPlayerRadius);
            player->damage(static_cast<float>(std::round(kGrenadePlayerDamage * (2.0f / (0.1f * distance + 2.0f)))));
            player->apply_impulse(std::cos(angle) * knockback, std::sin(angle) * knockback - 60.0f);
        }
    }

    for (int i = 0; i < zombie_count; ++i) {
        Zombie& zombie = zombies[i];
        if (!zombie.active || !zombie.alive) {
            continue;
        }

        const float zx = zombie.x + 9.0f;
        const float zy = zombie.y + 16.0f;
        const float distance = distance_between(x, y, zx, zy);
        if (distance > kGrenadeBlastRadius) {
            continue;
        }
        if (!in_explosion_halfspace(direction, x, y, zx, zy)) {
            continue;
        }

        const int damage = static_cast<int>(std::round(kGrenadeDamage * (2.0f / (0.1f * distance + 2.0f))));
        zombie.damage(damage);
        const float angle = std::atan2(zy - y, zx - x);
        const float knockback = kExplosionKnockbackSpeed * (1.0f - distance / kGrenadeBlastRadius);
        zombie.apply_impulse(std::cos(angle) * knockback, std::sin(angle) * knockback - 60.0f);
    }
}

} // namespace zg
