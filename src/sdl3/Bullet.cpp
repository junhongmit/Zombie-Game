#include "Bullet.h"

#include "CollisionMap.h"
#include "Constants.h"
#include "Player.h"
#include "SurfaceMask.h"
#include "Zombie.h"

#include <algorithm>
#include <cmath>

namespace zg {

namespace {

bool zombie_mask_hit(
    const SurfaceMask& zombie_mask,
    const Zombie& zombie,
    float world_x,
    float world_y,
    ZombieHitRegion* out_region)
{
    const ZombieHitRegion region = zombie.hit_test(world_x, world_y, true);
    if (region == ZombieHitRegion::None) {
        return false;
    }

    const float local_x = world_x - zombie.x;
    const float sample_x = zombie.walking_right ? local_x : (kZombieWidth - 1.0f - local_x);
    const int sprite_x = static_cast<int>(std::floor(sample_x)) + zombie.walk_frame * 17;
    const int sprite_y = static_cast<int>(std::floor(world_y - zombie.y));
    if (!zombie_mask.is_opaque_nonwhite(sprite_x, sprite_y)) {
        return false;
    }

    *out_region = region;
    return true;
}

} // namespace

void BulletSystem::update(float dt)
{
    for (Bullet& bullet : bullets_) {
        if (!bullet.active) {
            continue;
        }

        bullet.prev_x = bullet.x;
        bullet.prev_y = bullet.y;
        bullet.x += bullet.vx * dt;
        bullet.y += bullet.vy * dt;
        bullet.age += dt;

        if (bullet.age >= kBulletLifetimeSeconds ||
            bullet.x < 0.0f ||
            bullet.x > kWorldWidth ||
            bullet.y < 0.0f ||
            bullet.y > kLogicalHeight) {
            bullet.active = false;
        }
    }
}

void BulletSystem::try_fire(
    const Player& player,
    bool trigger_down,
    bool trigger_pressed,
    FireMode fire_mode,
    float dt)
{
    if (fire_cooldown_ > 0.0f) {
        fire_cooldown_ -= dt;
    }

    const bool wants_fire = fire_mode == FireMode::FullAuto ? trigger_down : trigger_pressed;
    if (wants_fire && fire_cooldown_ <= 0.0f) {
        fire(player);
        fire_cooldown_ = kGlockFireIntervalSeconds;
    }
}

const Bullet* BulletSystem::bullets() const
{
    return bullets_;
}

int BulletSystem::bullet_count() const
{
    return kMaxBullets;
}

void BulletSystem::resolve_collisions(
    const CollisionMap& collision_map,
    const SurfaceMask& zombie_mask,
    Zombie* zombies,
    int zombie_count)
{
    for (Bullet& bullet : bullets_) {
        if (!bullet.active) {
            continue;
        }

        const float dx = bullet.x - bullet.prev_x;
        const float dy = bullet.y - bullet.prev_y;
        const float distance = std::sqrt(dx * dx + dy * dy);
        const int steps = std::max(1, static_cast<int>(std::ceil(distance / kBulletTraceStep)));

        for (int step = 1; step <= steps && bullet.active; ++step) {
            const float t = static_cast<float>(step) / static_cast<float>(steps);
            const float sample_x = bullet.prev_x + dx * t;
            const float sample_y = bullet.prev_y + dy * t;

            if (collision_map.is_solid(
                    static_cast<int>(std::round(sample_x)),
                    static_cast<int>(std::round(sample_y)))) {
                bullet.active = false;
                break;
            }

            for (int i = 0; i < zombie_count; ++i) {
                Zombie& zombie = zombies[i];
                if (!zombie.active || !zombie.alive) {
                    continue;
                }

                ZombieHitRegion hit_region = ZombieHitRegion::None;
                if (!zombie_mask_hit(zombie_mask, zombie, sample_x, sample_y, &hit_region)) {
                    continue;
                }

                bullet.active = false;
                zombie.damage(hit_region == ZombieHitRegion::Head ? kHeadShotDamage : kBodyShotDamage);
                break;
            }
        }
    }
}

void BulletSystem::fire(const Player& player)
{
    Bullet& bullet = bullets_[next_bullet_];
    next_bullet_ = (next_bullet_ + 1) % kMaxBullets;

    const float pivot_x = player.x + (player.facing_right ? 8.0f : 10.0f);
    const float pivot_y = player.y + 13.0f;
    const float forward_x = std::cos(player.aim_angle_radians);
    const float forward_y = std::sin(player.aim_angle_radians);
    bullet.x = pivot_x + forward_x * 14.0f;
    bullet.y = pivot_y + forward_y * 14.0f;
    bullet.prev_x = bullet.x;
    bullet.prev_y = bullet.y;
    bullet.vx = std::cos(player.aim_angle_radians) * kBulletSpeed;
    bullet.vy = std::sin(player.aim_angle_radians) * kBulletSpeed;
    bullet.age = 0.0f;
    bullet.active = true;
}

} // namespace zg
