#pragma once

#include "CollisionMap.h"
#include "Constants.h"

namespace zg {

class Texture;
struct Camera;

enum class ImpactDirection {
    Left,
    Right,
    Top,
    Bottom
};

struct BloodParticle {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float lifetime = 0.0f;
    bool settled = false;
    bool active = false;
};

struct SmokeParticle {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float age = 0.0f;
    float lifetime = 0.0f;
    bool active = false;
};

class EffectsSystem {
public:
    void update(const CollisionMap& collision_map, float dt);
    void spawn_blood(float x, float y, ImpactDirection direction);
    void spawn_smoke(float x, float y, float angle_radians, float speed, int count);

    const BloodParticle* blood_particles() const;
    int blood_particle_count() const;
    const SmokeParticle* smoke_particles() const;
    int smoke_particle_count() const;

private:
    BloodParticle blood_particles_[kMaxBloodParticles];
    SmokeParticle smoke_particles_[kMaxSmokeParticles];
    int next_blood_particle_ = 0;
    int next_smoke_particle_ = 0;
};

} // namespace zg
