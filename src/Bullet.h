#pragma once

namespace zg {

class CollisionMap;
class EffectsSystem;
struct Player;
class SurfaceMask;
struct Zombie;

enum class FireMode {
    SemiAuto,
    FullAuto
};

struct Bullet {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float prev_x = 0.0f;
    float prev_y = 0.0f;
    float age = 0.0f;
    bool active = false;
};

class BulletSystem {
public:
    static constexpr int kMaxBullets = 128;

    void update(float dt);
    void try_fire(
        const Player& player,
        bool trigger_down,
        bool trigger_pressed,
        FireMode fire_mode,
        float fire_interval_seconds,
        float dt,
        EffectsSystem* effects,
        bool* fired);
    void resolve_collisions(
        const CollisionMap& collision_map,
        const SurfaceMask& zombie_mask,
        Zombie* zombies,
        int zombie_count,
        EffectsSystem* effects);

    const Bullet* bullets() const;
    int bullet_count() const;

private:
    void fire(const Player& player);

    Bullet bullets_[kMaxBullets];
    float fire_cooldown_ = 0.0f;
    int next_bullet_ = 0;
};

} // namespace zg
