#pragma once

#include "CollisionMap.h"
#include "Constants.h"
#include "Zombie.h"

namespace zg {

struct Player;

class ZombieDirector {
public:
    struct StairInfo {
        TriggerType trigger = TriggerType::None;
        float x = 0.0f;
        bool found = false;
    };

    void init(const CollisionMap& collision_map);
    void update(const CollisionMap& collision_map, const Player& player, Zombie* zombies, int zombie_count, float dt);
    float move_axis_for(int zombie_index) const;
    int wave() const;
    int alive_count() const;

private:
    void find_stair_centers(const CollisionMap& collision_map);
    void spawn_if_needed(Zombie* zombies, int zombie_count);
    void spawn_one(Zombie* zombies, int zombie_count, bool from_left);
    void update_ai(const CollisionMap& collision_map, const Player& player, Zombie* zombies, int zombie_count);
    float compute_move_axis(const CollisionMap& collision_map, const Player& player, Zombie* zombie) const;
    bool try_use_stairs(const CollisionMap& collision_map, Zombie* zombie, int target_floor) const;

    StairInfo red_stair_;
    StairInfo green_stair_;
    StairInfo blue_stair_;
    float spawn_timer_ = 0.0f;
    int wave_ = 1;
    int alive_count_ = 0;
    float move_axes_[kZombiePoolSize] = {};
    bool initialized_ = false;
};

} // namespace zg
