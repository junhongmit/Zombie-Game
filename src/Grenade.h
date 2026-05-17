#pragma once

#include "CollisionMap.h"
#include "Constants.h"

namespace zg {

class EffectsSystem;
struct Player;
struct Zombie;

struct Grenade {
    float x = 0.0f;
    float y = 0.0f;
    float prev_x = 0.0f;
    float prev_y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float clip_x = 0.0f;
    float clip_y = 0.0f;
    int bounces = 0;
    bool can_explode = true;
    bool clip_tail = false;
    bool active = false;
};

struct Explosion {
    float x = 0.0f;
    float y = 0.0f;
    float frame = static_cast<float>(kGrenadeExplosionFrameCount);
    SurfaceImpactDirection direction = SurfaceImpactDirection::Top;
    bool active = false;
};

class GrenadeSystem {
public:
    static constexpr int kMaxGrenades = 16;
    static constexpr int kMaxExplosions = 16;

    void update(
        const CollisionMap& collision_map,
        Player* player,
        Zombie* zombies,
        int zombie_count,
        EffectsSystem* effects,
        float dt);
    bool try_throw(const Player& player, bool throw_pressed, EffectsSystem* effects);

    const Grenade* grenades() const;
    int grenade_count() const;
    const Explosion* explosions() const;
    int explosion_count() const;
    bool consume_explosion_event();

private:
    void explode(
        float x,
        float y,
        SurfaceImpactDirection direction,
        Player* player,
        Zombie* zombies,
        int zombie_count,
        EffectsSystem* effects);

    Grenade grenades_[kMaxGrenades];
    Explosion explosions_[kMaxExplosions];
    int next_grenade_ = 0;
    int next_explosion_ = 0;
    float throw_cooldown_ = 0.0f;
    bool explosion_event_pending_ = false;
};

} // namespace zg
