#include "Effects.h"

#include "Constants.h"
#include "MathUtil.h"

#include <cmath>
#include <cstdlib>

namespace zg {

namespace {

float random_unit()
{
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

float random_range(float min_value, float max_value)
{
    return min_value + (max_value - min_value) * random_unit();
}

bool point_hits_solid(const CollisionMap& collision_map, float x, float y)
{
    return collision_map.is_solid(
        static_cast<int>(std::round(x)),
        static_cast<int>(std::round(y)));
}

bool has_surface_support(const CollisionMap& collision_map, float x, float y)
{
    return point_hits_solid(collision_map, x, y + 1.0f);
}

} // namespace

void EffectsSystem::update(const CollisionMap& collision_map, float dt)
{
    for (BloodParticle& particle : blood_particles_) {
        if (!particle.active) {
            continue;
        }

        particle.lifetime -= dt;
        if (particle.lifetime <= 0.0f) {
            particle.active = false;
            continue;
        }

        if (!particle.settled) {
            particle.vy += kGravity * 0.22f * dt;
            const float next_x = particle.x + particle.vx * dt;
            const float next_y = particle.y + particle.vy * dt;

            const bool hit_x = point_hits_solid(collision_map, next_x, particle.y);
            const bool hit_y = point_hits_solid(collision_map, particle.x, next_y);
            const bool hit_xy = point_hits_solid(collision_map, next_x, next_y);

            if (hit_xy || hit_y) {
                particle.settled = true;
                particle.vy = 0.0f;
                particle.vx *= 0.22f;

                float snapped_y = particle.y;
                while (snapped_y > 0.0f && point_hits_solid(collision_map, particle.x, snapped_y)) {
                    snapped_y -= 1.0f;
                }
                particle.y = snapped_y;
                if (hit_x) {
                    particle.vx = 0.0f;
                }
            } else {
                if (!hit_x) {
                    particle.x = next_x;
                } else {
                    particle.vx = 0.0f;
                }
                particle.y = next_y;
            }
        } else {
            if (!has_surface_support(collision_map, particle.x, particle.y)) {
                particle.settled = false;
                particle.vy = 12.0f;
                continue;
            }

            const float next_x = particle.x + particle.vx * dt;
            if (!point_hits_solid(collision_map, next_x, particle.y) &&
                has_surface_support(collision_map, next_x, particle.y)) {
                particle.x = next_x;
            } else {
                particle.vx = 0.0f;
            }

            particle.vx *= std::pow(0.04f, dt);
            if (std::fabs(particle.vx) < 2.0f) {
                particle.vx = 0.0f;
            }
        }

    }

    for (SmokeParticle& particle : smoke_particles_) {
        if (!particle.active) {
            continue;
        }

        particle.age += dt;
        particle.x += particle.vx * dt;
        particle.y += particle.vy * dt;
        particle.vx *= std::pow(0.35f, dt);
        particle.vy *= std::pow(0.35f, dt);

        if (particle.age >= particle.lifetime) {
            particle.active = false;
        }
    }
}

void EffectsSystem::spawn_blood(float x, float y, ImpactDirection direction)
{
    const int count = kBloodSpawnMin + (std::rand() % (kBloodSpawnMax - kBloodSpawnMin + 1));
    for (int i = 0; i < count; ++i) {
        BloodParticle& particle = blood_particles_[next_blood_particle_];
        next_blood_particle_ = (next_blood_particle_ + 1) % kMaxBloodParticles;

        particle.x = x;
        particle.y = y;
        particle.settled = false;
        particle.active = true;
        particle.lifetime = random_range(kBloodLifetimeMin, kBloodLifetimeMax);

        const float speed = random_range(42.0f, 118.0f);
        const float spread = random_range(-0.8f, 0.8f);
        switch (direction) {
        case ImpactDirection::Left:
            particle.vx = -speed;
            particle.vy = spread * speed * 0.7f;
            break;
        case ImpactDirection::Right:
            particle.vx = speed;
            particle.vy = spread * speed * 0.7f;
            break;
        case ImpactDirection::Top:
            particle.vx = spread * speed * 0.7f;
            particle.vy = -speed;
            break;
        case ImpactDirection::Bottom:
        default:
            particle.vx = spread * speed * 0.7f;
            particle.vy = speed * 0.25f;
            break;
        }
    }
}

void EffectsSystem::spawn_smoke(float x, float y, float angle_radians, float speed, int count)
{
    for (int i = 0; i < count; ++i) {
        SmokeParticle& particle = smoke_particles_[next_smoke_particle_];
        next_smoke_particle_ = (next_smoke_particle_ + 1) % kMaxSmokeParticles;

        const float spread = random_range(-0.26f, 0.26f);
        const float velocity = speed * random_range(0.65f, 1.15f);
        particle.x = x;
        particle.y = y;
        particle.vx = std::cos(angle_radians + spread) * velocity;
        particle.vy = -std::sin(angle_radians + spread) * velocity;
        particle.age = 0.0f;
        particle.lifetime = random_range(0.75f, 1.45f);
        particle.active = true;
    }
}

const BloodParticle* EffectsSystem::blood_particles() const
{
    return blood_particles_;
}

int EffectsSystem::blood_particle_count() const
{
    return kMaxBloodParticles;
}

const SmokeParticle* EffectsSystem::smoke_particles() const
{
    return smoke_particles_;
}

int EffectsSystem::smoke_particle_count() const
{
    return kMaxSmokeParticles;
}

} // namespace zg
