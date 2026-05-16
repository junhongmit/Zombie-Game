#pragma once

#include <SDL3/SDL_stdinc.h>

namespace zg {

class CollisionMap;

enum class ZombieHitRegion {
    None,
    Body,
    Head
};

struct Zombie {
    float x = 0.0f;
    float y = 400.0f;
    float speed = 24.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    bool walking_right = true;
    int walk_frame = 0;
    float walk_frame_distance = 0.0f;
    int hp = 100;
    bool active = true;
    bool alive = true;
    bool airborne = false;
    float hit_flash = 0.0f;
    float corpse_angle_degrees = 0.0f;
    float corpse_alpha = 255.0f;
    float corpse_fade_timer = 0.0f;

    Zombie(float x_in, float y_in, float speed_in, bool walking_right_in, int walk_frame_in);
    void update(const CollisionMap& collision_map, float dt);
    void damage(int amount);
    void kill();
    ZombieHitRegion hit_test(float world_x, float world_y, bool use_sprite_mask) const;
};

} // namespace zg
